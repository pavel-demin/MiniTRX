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
  QObject(parent), m_Cfg(0), m_Sts(0),
  m_BufferRX(0), m_BufferTX(0), m_BufferFFT(0),
  m_LimitRX(256), m_InputOffsetRX(0),
  m_LimitTX(0), m_InputOffsetTX(0),
  m_OutputBufferRX(0),
  m_TimerRX(0), m_TimerTX(0), m_FutureWatcherRX(0), m_FutureWatcherTX(0),
  m_WebSocketServer(0), m_WebSocket(0)
{
  int memFile;
  FILE *wisdomFile;
  int32_t i, *bufferInt;

  if((memFile = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    qApp->quit();
  }

  m_Cfg = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40000000);
  m_Sts = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40001000);
  m_BufferRX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40002000);
  m_BufferTX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40003000);
  m_BufferFFT = (int32_t *)mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40010000);

  /* enter reset mode */
  *(m_Cfg + 0) &= ~255;
  /* set default phase increment */
  *(m_Cfg + 1) = (uint32_t)floor(621000/125.0e6*(1<<30)+0.5);

  bufferInt = m_BufferTX;
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

  m_OutputBufferRX = new QByteArray();
  m_OutputBufferRX->resize(rxa[0].size * sizeof(float) + 1);

  m_TimerRX = new QTimer(this);
  connect(m_TimerRX, SIGNAL(timeout()), this, SLOT(on_TimerRX_timeout()));

  m_FutureWatcherRX = new QFutureWatcher<void>(this);
  connect(m_FutureWatcherRX, SIGNAL(finished()), this, SLOT(on_FutureWatcherRX_finished()));

  m_FutureWatcherTX = new QFutureWatcher<void>(this);
  connect(m_FutureWatcherTX, SIGNAL(finished()), this, SLOT(on_FutureWatcherTX_finished()));

  m_WebSocketServer = new QWebSocketServer(QString("SDR"), QWebSocketServer::NonSecureMode, this);
  if(m_WebSocketServer->listen(QHostAddress::Any, port))
  {
    connect(m_WebSocketServer, SIGNAL(newConnection()), this, SLOT(on_WebSocketServer_newConnection()));
    connect(m_WebSocketServer, SIGNAL(closed()), this, SLOT(on_WebSocketServer_closed()));
  }
}

//------------------------------------------------------------------------------

Server::~Server()
{
  m_WebSocketServer->close();
  if(m_WebSocket) delete m_WebSocket;
  if(m_OutputBufferRX) delete m_OutputBufferRX;
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_textMessageReceived(QString message)
{
  int32_t i, *bufferInt;

  if(message.compare(QString("start RX")) == 0)
  {
    *(m_Cfg + 0) |= 3;
    m_TimerRX->start(6);
  }
  else if(message.compare(QString("stop RX")) == 0)
  {
    m_TimerRX->stop();
  }
  if(message.compare(QString("start FFT")) == 0)
  {
  }
  else if(message.compare(QString("stop FFT")) == 0)
  {
  }
  if(message.compare(QString("start TX")) == 0)
  {
    m_TimerTX->start(6);
  }
  else if(message.compare(QString("stop TX")) == 0)
  {
    m_TimerTX->stop();
    bufferInt = m_BufferTX;
    for(i = 0; i < 512; ++i) *(bufferInt++) = 0;
  }
}

//------------------------------------------------------------------------------

void Server::on_TimerRX_timeout()
{
  int32_t i, offset, position;
  int32_t *bufferInt;
  float *bufferFloat;

  position = *(m_Sts + 0);
  if((m_LimitRX > 0 && position > m_LimitRX) || (m_LimitRX == 0 && position < 256))
  {
    offset = m_LimitRX > 0 ? 0 : 512;
    m_LimitRX += 256;
    if(m_LimitRX == 512) m_LimitRX = 0;
    bufferInt = m_BufferRX + offset;
    bufferFloat = rxa[0].inbuff + m_InputOffsetRX;
    for(i = 0; i < 256; ++i)
    {
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
    }
    m_InputOffsetRX += 512;
    if(m_InputOffsetRX == 4096)
    {
      m_InputOffsetRX = 0;
      QFuture<void> future = QtConcurrent::run(this, &Server::processRX);
      m_FutureWatcherRX->setFuture(future);
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
  bufferReal = (float *)(m_OutputBufferRX->constData() + 1);
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
  printf("send RX data\n");
  if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_OutputBufferRX);
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_binaryMessageReceived(QByteArray message)
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
  m_FutureWatcherTX->setFuture(future);
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

  position = *(m_Sts + 2);
  if((m_LimitTX > 0 && position > m_LimitTX) || (m_LimitTX == 0 && position < 256))
  {
    offset = m_LimitTX > 0 ? 0 : 512;
    m_LimitTX += 256;
    if(m_LimitTX == 512) m_LimitTX = 0;
    bufferInt = m_BufferTX + offset;
    bufferFloat = txa[1].outbuff + m_InputOffsetTX;
    for(i = 0; i < 256; ++i)
    {
      *(bufferInt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
      *(bufferInt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
    }
    m_InputOffsetTX += 256;
    if(m_InputOffsetTX == 4096)
    {
      m_InputOffsetTX = 0;
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
  QWebSocket *webSocket = m_WebSocketServer->nextPendingConnection();

  printf("new connection\n");
  if(m_WebSocket && webSocket)
  {
    printf("closing\n");
    webSocket->close();
    return;
  }

  connect(webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(on_WebSocket_textMessageReceived(QString)));
  connect(webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(on_WebSocket_binaryMessageReceived(QByteArray)));
  connect(webSocket, SIGNAL(disconnected()), this, SLOT(on_WebSocket_disconnected()));

  m_WebSocket = webSocket;
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_disconnected()
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  printf("disconnected\n");

  if(m_WebSocket == webSocket) m_WebSocket = 0;

  webSocket->deleteLater();
}
