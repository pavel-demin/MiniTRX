#ifndef Client_h
#define Client_h

#include <QtWidgets/QWidget>

class QPushButton;
class QProgressBar;
class QSlider;
class QComboBox;
class QAudioOutput;
class QAudioInput;
class Indicator;
class CPlotter;

class Client: public QWidget
{
  Q_OBJECT

public:
  Client(QWidget *parent = 0);
  virtual ~Client();

private slots:
  virtual void on_IndicatorRX_changed(int freq);
  virtual void on_FrequencyRX_changed(int freq);
  virtual void on_IndicatorFFT_changed(int freq);
  virtual void on_Range_changed(int range);
  virtual void on_Offset_changed(int offset);

private:

  Indicator *m_IndicatorRX;
  Indicator *m_IndicatorTX;
  Indicator *m_IndicatorFFT;
  QProgressBar *m_LevelRX;
  QProgressBar *m_LevelTX;
  QSlider *m_Range;
  QSlider *m_Offset;
  CPlotter *m_Plotter;

  QComboBox *m_DeviceRX;
  QComboBox *m_DeviceTX;
  QAudioOutput *m_AudioOutput;
  QAudioInput *m_AudioInput;

  bool m_pullMode;
  QByteArray m_buffer;

};

#endif
