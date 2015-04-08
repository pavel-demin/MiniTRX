#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <QtCore/QTimer>
#include <QtCore/QFutureWatcher>
#include <QtCore/QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <fftw3.h>

#include "server.h"

#include "comm.h"

using namespace std;

//------------------------------------------------------------------------------

Server::Server(int16_t port, QObject *parent):
  QObject(parent), fCfg(0), fSts(0),
  fBufRX(0), fBufTX(0), fBufFFT(0),
  fLimitRX(256), fInputOffsetRX(0),
  fLimitTX(0), fInputOffsetTX(0),
  fOutputBufferRX(0),
  fTimerRX(0), fTimerTX(0), fFutureWatcherRX(0), fFutureWatcherTX(0),
  fWebSocketServer(0), fWebSocket(0)
{
  int memFile;
  FILE *wisdomFile;
  int32_t i, *bufferInt;

  if((memFile = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    qApp->quit();
  }

  fCfg = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40000000);
  fSts = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40001000);
  fBufRX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40002000);
  fBufTX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40003000);
  fBufFFT = (int32_t *)mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40010000);

  bufferInt = fBufTX;
  for(i = 0; i < 512; ++i) *(bufferInt++) = 0;

  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "r")))
  {
    fftwf_import_wisdom_from_file(wisdomFile);
    fclose(wisdomFile);
  }
  create_rxa(0, 2048, 20000);
  create_txa(0, 2048, 20000);
  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "w")))
  {
    fftwf_export_wisdom_to_file(wisdomFile);
    fclose(wisdomFile);
  }

  fOutputBufferRX = new QByteArray(0, rxa[0].size * sizeof(complex) + 1);

  fTimerRX = new QTimer(this);
  connect(fTimerRX, SIGNAL(timeout()), this, SLOT(on_TimerRX_timeout()));

  fFutureWatcherRX = new QFutureWatcher<void>(this);
  connect(fFutureWatcherRX, SIGNAL(finished), this, SLOT(on_FutureWatcherRX_finished));

  fFutureWatcherTX = new QFutureWatcher<void>(this);
  connect(fFutureWatcherTX, SIGNAL(finished), this, SLOT(on_FutureWatcherTX_finished));

  fWebSocketServer = new QWebSocketServer(QString("SDR"), QWebSocketServer::NonSecureMode, this);
  if(fWebSocketServer->listen(QHostAddress::Any, port))
  {
    connect(fWebSocketServer, SIGNAL(newConnection), this, SLOT(on_WebSocketServer_newConnection));
    connect(fWebSocketServer, SIGNAL(closed), this, SLOT(on_WebSocketServer_closed));
  }
}

//------------------------------------------------------------------------------

Server::~Server()
{
  fWebSocketServer->close();
  if(fWebSocket) delete fWebSocket;
  if(fOutputBufferRX) delete fOutputBufferRX;
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_textMessageReceived(QString message)
{
  int32_t i, *bufferInt;

  if(message.compare(QString("startRX")) == 0)
  {
    fTimerRX->start(6);
  }
  else if(message.compare(QString("stopRX")) == 0)
  {
    fTimerRX->stop();
  }
  if(message.compare(QString("startTX")) == 0)
  {
    fTimerTX->start(6);
  }
  else if(message.compare(QString("stopTX")) == 0)
  {
    fTimerTX->stop();
    bufferInt = fBufTX;
    for(i = 0; i < 512; ++i) *(bufferInt++) = 0;
  }
}

//------------------------------------------------------------------------------

void Server::on_TimerRX_timeout()
{
  int32_t i, offset, position;
  int32_t *bufferInt;
  float *bufferFloat;

  position = *(fSts + 0);
  if((fLimitRX > 0 && position > fLimitRX) || (fLimitRX == 0 && position < 256))
  {
    offset = fLimitRX > 0 ? 0 : 512;
    fLimitRX += 256;
    if(fLimitRX == 512) fLimitRX = 0;
    bufferInt = fBufRX + offset;
    bufferFloat = rxa[0].inbuff + fInputOffsetRX;
    for(i = 0; i < 256; ++i)
    {
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
    }
    fInputOffsetRX += 512;
    if(fInputOffsetRX == 4096)
    {
      fInputOffsetRX = 0;
      QFuture<void> future = QtConcurrent::run(this, &Server::processRX);
      fFutureWatcherRX->setFuture(future);
    }
  }
}

//------------------------------------------------------------------------------

void Server::processRX()
{
  int32_t i, size;
  float *bufferReal, *bufferComplex;

  xrxa(0);

  size = rxa[0].size;
  bufferReal = (float *)(fOutputBufferRX->constData() + 1);
  bufferComplex = rxa[0].outbuff;

  for(i = 0; i < size; ++i)
  {
    *(bufferReal++) = *(bufferComplex++);
    ++bufferComplex;
  }
}

//------------------------------------------------------------------------------

void Server::on_FutureWatcherRX_finished()
{
  if(fWebSocket) fWebSocket->sendBinaryMessage(*fOutputBufferRX);
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_binaryMessageReceived(QByteArray message)
{
  int32_t i, size;
  float *bufferReal, *bufferComplex, value;

  size = txa[0].size;
  bufferReal = (float *)(message.constData());
  bufferComplex = txa[1].inbuff;

  for(i = 0; i < size; ++i)
  {
    value = *(bufferReal++);
    *(bufferComplex++) = value;
    *(bufferComplex++) = value;
  }

  QFuture<void> future = QtConcurrent::run(this, &Server::processTX);
  fFutureWatcherTX->setFuture(future);
}

//------------------------------------------------------------------------------

void Server::processTX()
{
  xtxa(1);
}

//------------------------------------------------------------------------------

void Server::on_FutureWatcherTX_finished()
{
}

//------------------------------------------------------------------------------

void Server::on_TimerTX_timeout()
{
  int32_t i, offset, position;
  int32_t *bufferInt;
  float *bufferFloat;

  position = *(fSts + 2);
  if((fLimitTX > 0 && position > fLimitTX) || (fLimitTX == 0 && position < 256))
  {
    offset = fLimitTX > 0 ? 0 : 512;
    fLimitTX += 256;
    if(fLimitTX == 512) fLimitTX = 0;
    bufferInt = fBufTX + offset;
    bufferFloat = txa[1].outbuff + fInputOffsetTX;
    for(i = 0; i < 256; ++i)
    {
      *(bufferInt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
      *(bufferInt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
    }
    fInputOffsetTX += 256;
    if(fInputOffsetTX == 4096)
    {
      fInputOffsetTX = 0;
    }
  }
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_closed()
{
  qApp->quit();
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_newConnection()
{
  QWebSocket *webSocket = fWebSocketServer->nextPendingConnection();

  if(fWebSocket && webSocket)
  {
    webSocket->close();
  }

  connect(webSocket, SIGNAL(textMessageReceived), this, SLOT(on_WebSocketServer_textMessageReceived));
  connect(webSocket, SIGNAL(binaryMessageReceived), this, SLOT(on_WebSocketServer_binaryMessageReceived));
  connect(webSocket, SIGNAL(disconnected), this, SLOT(on_WebSocketServer_disconnected));

  fWebSocket = webSocket;
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_disconnected()
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  if(fWebSocket == webSocket) fWebSocket = 0;

  webSocket->deleteLater();
}
