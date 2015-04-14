#ifndef Indicator_h
#define Indicator_h

#include <QtWidgets/QFrame>

class Indicator: public QFrame
{
  Q_OBJECT

public:
  Indicator(QWidget *parent = 0);
  virtual ~Indicator();
  int value() {return m_Value;};
  void setValue(int value);
  void setDeltaMin(int delta);
  void applyDelta(int delta);

signals:
  void valueChanged(int value);

private:
  int m_Value;
};

#endif
