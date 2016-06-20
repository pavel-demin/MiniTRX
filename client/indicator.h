#ifndef Indicator_h
#define Indicator_h

#include <QtWidgets/QFrame>

class QHBoxLayout;

class Indicator: public QFrame
{
  Q_OBJECT

public:
  Indicator(QWidget *parent = 0);
  virtual ~Indicator();
  int value() {return m_Value;};
  void setFont(QFont font);
  void setSize(int size);
  void setValue(int value);
  void setValueMin(int value);
  void setValueMax(int value);
  void setDeltaMin(int delta);
  void applyDelta(int delta);

signals:
  void valueChanged(int value);

private:
  QHBoxLayout *m_Layout;
  QFont m_Font;
  int m_Value;
  int m_ValueMin;
  int m_ValueMax;
};

#endif
