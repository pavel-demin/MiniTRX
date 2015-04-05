
#include <QtWidgets/QLabel>
#include <QtGui/QMouseEvent>

#include "indicator.h"

//------------------------------------------------------------------------------

static const QString gStyleSheets[3] =
{
  "",
  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 155, 155, 255), stop:0.49 rgba(255, 155, 155, 255), stop:0.50 rgba(0, 0, 0, 0), stop:1 rgba(0, 0, 0, 0));",
  "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0,0,0,0), stop:0.50 rgba(0, 0, 0, 0), stop:0.51 rgba(155, 155, 255, 255), stop:1 rgba(155, 155, 255, 255));"
};

//------------------------------------------------------------------------------

class CustomDigit: public QLabel
{
public:
  CustomDigit(QWidget *parent = 0): QLabel(parent), fActive(true), fState(0), fDelta(0) {}

  void mouseMoveEvent(QMouseEvent *event)
  {
    if(!fActive) return;
    QPalette pal;
    QColor color = palette().color(QPalette::WindowText);
    if(event->y() < height()/2 && fState != 1)
    {
      fState = 1;
      setStyleSheet(gStyleSheets[1]);
      pal = palette();
      pal.setColor(QPalette::WindowText, color);
      setPalette(pal);
    }
    if(event->y() > height()/2 && fState != 2)
    {
      fState = 2;
      setStyleSheet(gStyleSheets[2]);
      pal = palette();
      pal.setColor(QPalette::WindowText, color);
      setPalette(pal);
    }
  }

  void leaveEvent(QEvent *event)
  {
    if(!fActive) return;
    QPalette pal;
    QColor color = palette().color(QPalette::WindowText);
    fState = 0;
    setStyleSheet(gStyleSheets[0]);
    pal = palette();
    pal.setColor(QPalette::WindowText, color);
    setPalette(pal);
  }

  void mousePressEvent(QMouseEvent *event)
  {
    if(!fActive) return;
    if(event->y() < height()/2)
    {
      fIndicator->applyDelta(fDelta);
    }
    if(event->y() > height()/2)
    {
      fIndicator->applyDelta(-fDelta);
    }
  }

  void wheelEvent(QWheelEvent *event)
  {
    if(!fActive) return;
    fIndicator->applyDelta(event->delta()/90*fDelta);
  }

  bool fActive;
  int fState;
  int fDelta;
  Indicator *fIndicator;
};

//------------------------------------------------------------------------------

Indicator::Indicator(QWidget *parent):
  QFrame(parent)
{
  CustomDigit *digit;
  int i, x, delta;
  QFont font("Arial", 27, QFont::Normal);
  x = 155;
  delta = 1;
  for(i = 0; i < 8; ++i)
  {
    digit = new CustomDigit(this);
    digit->setText(QString::number(0));
    digit->setAlignment(Qt::AlignCenter);
    digit->setFont(font);
    digit->setGeometry(QRect(x, 5, 20, 30));
    digit->setMouseTracking(true);
    digit->fDelta = delta;
    digit->fIndicator = this;
    x -= (i % 3 == 2) ? 25 : 20;
    delta *= 10;
  }
}

//------------------------------------------------------------------------------

Indicator::~Indicator()
{
}

//------------------------------------------------------------------------------

void Indicator::setValue(int value)
{
  int quotient;
  QPalette palette;
  if(value < 0 && value > 50000000) return;
  foreach(CustomDigit *digit, findChildren<CustomDigit *>())
  {
    quotient = value/digit->fDelta;
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
    digit->setText(QString::number(quotient%10));
  }
  fValue = value;
  emit valueChanged(value);
}

//------------------------------------------------------------------------------

void Indicator::setDeltaMin(int delta)
{
  foreach(CustomDigit *digit, findChildren<CustomDigit *>())
  {
    if(digit->fDelta < delta)
    {
      digit->setText(QString::number(0));
      digit->setMouseTracking(false);
      digit->fActive = false;
    }
    else
    {
      digit->setMouseTracking(true);
      digit->fActive = true;
    }
  }
}

//------------------------------------------------------------------------------

void Indicator::applyDelta(int delta)
{
  int value = fValue + delta;
  if(delta < 0 && value < 0) value = 0;
  if(delta > 0 && value > 50000000) value = 50000000;
  setValue(value);
}
