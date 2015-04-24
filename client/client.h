#ifndef Client_h
#define Client_h

#include <QtWidgets/QWidget>

#include <soxr.h>

class QPushButton;
class QProgressBar;
class QSlider;
class QComboBox;
class QLineEdit;
class QAudioFormat;
class QAudioOutput;
class QAudioInput;
class QByteArray;
class QIODevice;
class QWebSocket;
class Indicator;
class CPlotter;

class Client: public QWidget
{
  Q_OBJECT

public:
  Client(QWidget *parent = 0);
  virtual ~Client();

private slots:
  void on_IndicatorRX_changed(int freq);
  void on_FrequencyRX_changed(int freq);
  void on_IndicatorFFT_changed(int freq);
  void on_Range_changed(int range);
  void on_Offset_changed(int offset);
  void on_InputDevice_activated(int index);
  void on_OutputDevice_activated(int index);
  void on_AudioInput_notify();
  void on_AudioOutput_notify();
  void on_WebSocket_connected();
  void on_WebSocket_disconnected();
  void on_WebSocket_binaryMessageReceived(QByteArray message);

  void on_EnableRX_clicked();
  void on_EnableTX_clicked();
  void on_EnableFFT_clicked();
  void on_Connect_clicked();

private:

  Indicator *m_IndicatorRX;
  Indicator *m_IndicatorTX;
  Indicator *m_IndicatorFFT;
  QProgressBar *m_LevelRX;
  QProgressBar *m_LevelTX;
  QSlider *m_Range;
  QSlider *m_Offset;
  CPlotter *m_Plotter;

  QLineEdit *m_Address;

  QPushButton *m_Connect;
  QPushButton *m_EnableRX;
  QPushButton *m_EnableTX;
  QPushButton *m_EnableFFT;

  QComboBox *m_InputDevice;
  QComboBox *m_OutputDevice;

  QByteArray *m_InputBuffer;
  QByteArray *m_OutputBuffer;

  soxr_t m_InputResampler;
  soxr_t m_OutputResampler;

  QAudioFormat *m_AudioFormat;
  QAudioInput *m_AudioInput;
  QAudioOutput *m_AudioOutput;
  QIODevice *m_AudioInputDevice;
  QIODevice *m_AudioOutputDevice;

  QWebSocket *m_WebSocket;
};

#endif
