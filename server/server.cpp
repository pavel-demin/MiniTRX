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

#include "server.h"

using namespace std;

//------------------------------------------------------------------------------

Server::Server(quint16 port, QObject *parent):
  QObject(parent), fCfg(0), fSts(0),
  fBufRX(0), fLimitRX(128),
  fInputBufferRX(0), fInputOffsetRX(0),
  fTimerRX(0), fFutureWatcherRX(0),
  fWebSocketServer(0), fWebSocket(0)
{
  int file;
  if((file = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    qApp->quit();
  }

  fCfg = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, file, 0x40000000);
  fSts = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, file, 0x40001000);
  fBufRX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, file, 0x40002000);

  fTimerRX = new QTimer(this);
  connect(fTimerRX, SIGNAL(timeout()), this, SLOT(on_TimerRX_timeout()));

  fFutureWatcherRX = new QFutureWatcher<void>(this);
  connect(fFutureWatcherRX, SIGNAL(finished), this, SLOT(on_FutureWatcherRX_finished));

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
}

//------------------------------------------------------------------------------

void Server::on_TimerRX_timeout()
{
  int32_t i, offset, position;
  int32_t *bufferInt;
  float *bufferFloat;

  position = *(fSts + 0);
  if((fLimitRX > 0 && position > fLimitRX) || (fLimitRX == 0 && position < 384))
  {
    offset = fLimitRX > 0 ? fLimitRX*8 - 1024 : 3072;
    fLimitRX += 128;
    if(fLimitRX == 512) fLimitRX = 0;
    bufferInt = fBufRX + offset;
    bufferFloat = fInputBufferRX + fInputOffsetRX;
    for(i = 0; i < 128; ++i)
    { 
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
      *(bufferFloat++) = ((float) *(bufferInt++)) / 2147483647.0;
      
      fInputOffsetRX += 256;
      if(fInputOffsetRX == 4096)
      {
        fInputOffsetRX = 0;
        QFuture<void> future = QtConcurrent::run(this, &Server::processRX);
        fFutureWatcherRX->setFuture(future);
      }
    }
  }
}

//------------------------------------------------------------------------------

void Server::processRX()
{
}

//------------------------------------------------------------------------------

void Server::on_FutureWatcherRX_finished()
{
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

void Server::on_WebSocketServer_textMessageReceived(QString message)
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  if(message.compare(QString("startRX")) == 0)
  {
    fTimerRX->start(5);
  }
  else if(message.compare(QString("stopRX")) == 0)
  {
    fTimerRX->stop();
  }
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_binaryMessageReceived(QByteArray message)
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  webSocket->sendBinaryMessage(message);
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_disconnected()
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  if(fWebSocket == webSocket) fWebSocket = 0;

  webSocket->deleteLater();
}
