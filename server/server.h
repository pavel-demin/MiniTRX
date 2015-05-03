#ifndef Server_h
#define Server_h

#include <stdint.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <samplerate.h>

class QTimer;
class QWebSocketServer;
class QWebSocket;

class Server: public QObject
{
  Q_OBJECT

public:
  Server(int16_t port, QObject *parent = 0);
  virtual ~Server();

private slots:
  void on_TimerRX_timeout();
  void on_TimerFFT_timeout();
  void on_TimerTX_timeout();
  void on_WebSocketServer_closed();
  void on_WebSocketServer_newConnection();
  void on_WebSocket_binaryMessageReceived(QByteArray message);
  void on_WebSocket_disconnected();

private:
  uint32_t *m_Cfg;
  uint16_t *m_Sts;
  int32_t *m_BufferRX, *m_BufferTX, *m_BufferFFT;
  int m_LimitRX, m_InputOffsetRX;
  int m_LimitTX, m_InputOffsetTX;
  QByteArray *m_InputBufferRX;
  QByteArray *m_OutputBufferRX;
  QByteArray *m_OutputBufferFFT;
  int32_t m_CounterRX;
  int16_t *m_PointerRX;
  int32_t m_FreqMin;
  SRC_STATE *m_StateRX;
  SRC_DATA *m_DataRX;
  QTimer *m_TimerRX;
  QTimer *m_TimerFFT;
  QTimer *m_TimerTX;
  QWebSocketServer *m_WebSocketServer;
  QWebSocket *m_WebSocket;
};

#endif
