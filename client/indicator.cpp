
#include <QtCore/qmath.h>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QMouseEvent>

#include "indicator.h"

//------------------------------------------------------------------------------

static const QString c_StyleSheets[3] =
{
  "",
  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 153, 153, 255), stop:0.49 rgba(255, 153, 153, 255), stop:0.50 rgba(0, 0, 0, 0), stop:1 rgba(0, 0, 0, 0))",
  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 0), stop:0.50 rgba(0, 0, 0, 0), stop:0.51 rgba(153, 204, 255, 255), stop:1 rgba(153, 204, 255, 255))"
};

//------------------------------------------------------------------------------

class CustomDigit: public QLabel
{
public:
  CustomDigit(QWidget *parent = 0): QLabel(parent), m_Active(true), m_State(0), m_Delta(0) {}

  void resizeEvent(QResizeEvent *event)
  {
    QFont f = font();
    f.setPixelSize(height());
    setFont(f);
  }

  void mouseMoveEvent(QMouseEvent *event)
  {
    if(!m_Active) return;
    QPalette pal;
    QColor color = palette().color(QPalette::WindowText);
    if(event->y() < height() / 2 && m_State != 1)
    {
      m_State = 1;
      setStyleSheet(c_StyleSheets[1]);
      pal = palette();
      pal.setColor(QPalette::WindowText, color);
      setPalette(pal);
    }
    if(event->y() > height() / 2 && m_State != 2)
    {
      m_State = 2;
      setStyleSheet(c_StyleSheets[2]);
      pal = palette();
      pal.setColor(QPalette::WindowText, color);
      setPalette(pal);
    }
  }

  void leaveEvent(QEvent *event)
  {
    if(!m_Active) return;
    QPalette pal;
    QColor color = palette().color(QPalette::WindowText);
    m_State = 0;
    setStyleSheet(c_StyleSheets[0]);
    pal = palette();
    pal.setColor(QPalette::WindowText, color);
    setPalette(pal);
  }

  void mousePressEvent(QMouseEvent *event)
  {
    if(!m_Active) return;
    if(event->y() < height() / 2)
    {
      m_Indicator->applyDelta(m_Delta);
    }
    if(event->y() > height() / 2)
    {
      m_Indicator->applyDelta(-m_Delta);
    }
  }

  void wheelEvent(QWheelEvent *event)
  {
    if(!m_Active) return;
    m_Indicator->applyDelta(event->delta() / 90 * m_Delta);
  }

  bool m_Active;
  int m_State;
  int m_Delta;
  Indicator *m_Indicator;
};

//------------------------------------------------------------------------------

Indicator::Indicator(QWidget *parent):
  QFrame(parent), m_Layout(0), m_Font(),
  m_Value(0), m_ValueMin(0), m_ValueMax(0)
{
  m_Layout = new QHBoxLayout(this);
  m_Layout->setContentsMargins(1, 1, 1, 1);
  m_Layout->setSpacing(1);
  setLayout(m_Layout);
}

//------------------------------------------------------------------------------

Indicator::~Indicator()
{
}

//------------------------------------------------------------------------------

void Indicator::setFont(QFont font)
{
  m_Font = font;
  foreach(CustomDigit *digit, findChildren<CustomDigit *>())
  {
    digit->setFont(font);
  }
}

//------------------------------------------------------------------------------

void Indicator::setSize(int size)
{
  int i;
  CustomDigit *digit;
  QLayoutItem *item;

  while((digit = findChild<CustomDigit *>()))
  {
    delete digit;
  }
  while((item = m_Layout->takeAt(0)))
  {
    delete item;
  }

  m_Layout->addStretch();
  for(i = size; i > 0; --i)
  {
    digit = new CustomDigit(this);
    digit->setText(QString::number(0));
    digit->setAlignment(Qt::AlignCenter);
    digit->setFont(m_Font);
    m_Layout->addWidget(digit);
    digit->setMouseTracking(true);
    digit->m_Delta = (int)(qPow(10.0, i - 1) + 0.5);
    digit->m_Indicator = this;
    if(i > 1 && i % 3 == 1) m_Layout->addStretch();
  }
  m_Layout->addStretch();
  setValue(m_Value);
}

//------------------------------------------------------------------------------

void Indicator::setValue(int value)
{
  int quotient;
  QPalette palette;
  if(value < m_ValueMin || value > m_ValueMax) return;
  foreach(CustomDigit *digit, findChildren<CustomDigit *>())
  {
    quotient = value / digit->m_Delta;
    palette = digit->palette();
    if(quotient == 0)
    {
      palette.setColor(QPalette::WindowText, QColor(Qt::lightGray));
    }
    else
    {
      palette.setColor(QPalette::WindowText, QColor(Qt::black));
    }
    digit->setPalette(palette);
    digit->setText(QString::number(quotient % 10));
  }
  m_Value = value;
  emit valueChanged(value);
}

//------------------------------------------------------------------------------

void Indicator::setValueMin(int value)
{
  if(value > m_ValueMax) return;
  m_ValueMin = value;
  if(m_Value < value) setValue(value);
}

//------------------------------------------------------------------------------

void Indicator::setValueMax(int value)
{
  if(value < m_ValueMin) return;
  m_ValueMax = value;
  if(m_Value > value) setValue(value);
}

//------------------------------------------------------------------------------

void Indicator::setDeltaMin(int delta)
{
  foreach(CustomDigit *digit, findChildren<CustomDigit *>())
  {
    if(digit->m_Delta < delta)
    {
      digit->setText(QString::number(0));
      digit->setMouseTracking(false);
      digit->m_Active = false;
    }
    else
    {
      digit->setMouseTracking(true);
      digit->m_Active = true;
    }
  }
}

//------------------------------------------------------------------------------

void Indicator::applyDelta(int delta)
{
  int value = m_Value + delta;
  if(delta < 0 && value < m_ValueMin) return;
  if(delta > 0 && value > m_ValueMax) return;
  setValue(value);
}
