#ifndef Client_h
#define Client_h

#include <QObject>
#include <QStringList>

class QByteArray;
class QAudioFormat;
class QAudioInput;
class QAudioOutput;
class QIODevice;
class QWebSocket;

class Spectrum;
class Waterfall;

class Client: public QObject
{
  Q_OBJECT

public:
  Client(QObject *parent = 0);
  virtual ~Client();

  void setSpectrum(Spectrum *spectrum) { m_Spectrum = spectrum; }
  void setWaterfall(Waterfall *waterfall) { m_Waterfall = waterfall; }

  Q_INVOKABLE QStringList availableOutputDevices();
  Q_INVOKABLE QStringList availableInputDevices();

public slots:
  void on_Connect_clicked(QString address);
  void on_Disconnect_clicked();
  void on_StartRX_clicked();
  void on_StopRX_clicked();
  void on_StartTX_clicked();
  void on_StopTX_clicked();
  void on_StartFFT_clicked();
  void on_StopFFT_clicked();

private slots:
/*
  void on_IndicatorRX_changed(int freq);
  void on_FrequencyRX_changed(int freq);
  void on_IndicatorFFT_changed(int freq);
  void on_IndicatorTX_changed(int freq);
  void on_Range_changed(int range);
  void on_Offset_changed(int offset);
*/
  void on_InputDevice_activated(int index);
  void on_OutputDevice_activated(int index);
  void on_AudioInput_notify();
  void on_AudioOutput_notify();

  void on_WebSocket_connected();
  void on_WebSocket_disconnected();
  void on_WebSocket_binaryMessageReceived(QByteArray message);

private:
  void sendCommand();

  Spectrum *m_Spectrum;
  Waterfall *m_Waterfall;

  QByteArray *m_BufferCmd;
  const char *m_PointerCmd;

  QAudioFormat *m_AudioFormat;
  QAudioInput *m_AudioInput;
  QAudioOutput *m_AudioOutput;
  QIODevice *m_AudioInputDevice;
  QIODevice *m_AudioOutputDevice;

  QWebSocket *m_WebSocket;
};

#endif
