#ifndef Server_h
#define Server_h

#include <stdint.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

class QTimer;
class QWebSocketServer;
class QWebSocket;

template <class T> class QFutureWatcher;

class Server: public QObject
{
  Q_OBJECT

public:
  Server(quint16 port, QObject *parent = 0);
  virtual ~Server();

private slots:
  void on_TimerRX_timeout();
  void on_FutureWatcherRX_finished();
  void on_WebSocketServer_closed();
  void on_WebSocketServer_newConnection();
  void on_WebSocketServer_textMessageReceived(QString message);
  void on_WebSocketServer_binaryMessageReceived(QByteArray message);
  void on_WebSocketServer_disconnected();

private:
  void processRX();

  uint32_t *fCfg, *fSts;
  int32_t *fBufRX;
  int fLimitRX;
  float *fInputBufferRX;
  int fInputOffsetRX;
  QTimer *fTimerRX;
  QFutureWatcher<void> *fFutureWatcherRX;
  QWebSocketServer *fWebSocketServer;
  QWebSocket *fWebSocket;
};

#endif
