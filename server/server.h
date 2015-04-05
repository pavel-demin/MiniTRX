#ifndef Server_h
#define Server_h

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

class QWebSocketServer;
class QWebSocket;

class Server: public QObject
{
  Q_OBJECT

public:
  Server(quint16 port, QObject *parent = 0);
  virtual ~Server();

signals:
  void closed();

private slots:
  void onNewConnection();
  void processTextMessage(QString message);
  void processBinaryMessage(QByteArray message);
  void socketDisconnected();

private:
  QWebSocketServer *fWebSocketServer;
  QList<QWebSocket *> fWebSockets;
};

#endif
