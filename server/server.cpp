#include <stdint.h>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include "server.h"

using namespace std;

//------------------------------------------------------------------------------

Server::Server(quint16 port, QObject *parent):
  QObject(parent), fWebSocketServer(0)
{
  fWebSocketServer = new QWebSocketServer(QString("SDR"), QWebSocketServer::NonSecureMode, this);
  if(fWebSocketServer->listen(QHostAddress::Any, port))
  {
    connect(fWebSocketServer, &QWebSocketServer::newConnection, this, &Server::onNewConnection);
    connect(fWebSocketServer, &QWebSocketServer::closed, this, &Server::closed);
  }
}

//------------------------------------------------------------------------------

Server::~Server()
{
  fWebSocketServer->close();
  qDeleteAll(fWebSockets.begin(), fWebSockets.end());
}

//------------------------------------------------------------------------------

void Server::onNewConnection()
{
  QWebSocket *sock = fWebSocketServer->nextPendingConnection();

  connect(sock, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
  connect(sock, &QWebSocket::binaryMessageReceived, this, &Server::processBinaryMessage);
  connect(sock, &QWebSocket::disconnected, this, &Server::socketDisconnected);

  fWebSockets << sock;
}

//------------------------------------------------------------------------------

void Server::processTextMessage(QString message)
{
  QWebSocket *sock = qobject_cast<QWebSocket *>(sender());
  if(sock)
  {
    sock->sendTextMessage(message);
  }
}

//------------------------------------------------------------------------------

void Server::processBinaryMessage(QByteArray message)
{
  QWebSocket *sock = qobject_cast<QWebSocket *>(sender());
  if(sock)
  {
    sock->sendBinaryMessage(message);
  }
}

//------------------------------------------------------------------------------

void Server::socketDisconnected()
{
  QWebSocket *sock = qobject_cast<QWebSocket *>(sender());
  if(sock)
  {
    fWebSockets.removeAll(sock);
    sock->deleteLater();
  }
}
