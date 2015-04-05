#ifndef Client_h
#define Client_h

#include <QtWidgets/QWidget>

class QPushButton;
class QProgressBar;
class QSlider;
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

  Indicator *fIndicatorRX;
  Indicator *fIndicatorTX;
  Indicator *fIndicatorFFT;
  QProgressBar *fLevelRX;
  QProgressBar *fLevelTX;
  QSlider *fRange;
  QSlider *fOffset;
  CPlotter *fPlotter;

};

#endif
