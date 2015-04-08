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
  Server(int16_t port, QObject *parent = 0);
  virtual ~Server();

private slots:
  void on_TimerRX_timeout();
  void on_TimerTX_timeout();
  void on_FutureWatcherRX_finished();
  void on_FutureWatcherTX_finished();
  void on_WebSocketServer_closed();
  void on_WebSocketServer_newConnection();
  void on_WebSocketServer_textMessageReceived(QString message);
  void on_WebSocketServer_binaryMessageReceived(QByteArray message);
  void on_WebSocketServer_disconnected();

private:
  void processRX();
  void processTX();

  uint32_t *fCfg, *fSts;
  int32_t *fBufRX, *fBufTX, *fBufFFT;
  int fLimitRX, fInputOffsetRX;
  int fLimitTX, fInputOffsetTX;
  QByteArray *fOutputBufferRX;
  QTimer *fTimerRX;
  QTimer *fTimerTX;
  QFutureWatcher<void> *fFutureWatcherRX;
  QFutureWatcher<void> *fFutureWatcherTX;
  QWebSocketServer *fWebSocketServer;
  QWebSocket *fWebSocket;
};

#endif
