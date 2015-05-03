#include <string.h>

#include <QtGui/QPixmap>
#include <QtGui/QPainter>

#include "spectrum.h"

//------------------------------------------------------------------------------

Spectrum::Spectrum(QQuickItem *parent):
  QQuickPaintedItem(parent), m_Pixmap(0)
{
  m_Pixmap = new QPixmap(0, 0);
  setFlag(ItemHasContents, true);
}

//------------------------------------------------------------------------------

Spectrum::~Spectrum()
{
  if(m_Pixmap) delete m_Pixmap;
}

//------------------------------------------------------------------------------

void Spectrum::setData(unsigned char *data)
{
  QPainter painter;
  int i, width, height;
  int x, xnew;
  int y, ynew;
  QPoint curve[4096];

  width = m_Pixmap->width();
  height = m_Pixmap->height();

  m_Pixmap->fill(Qt::black);

  painter.begin(m_Pixmap);
  painter.setPen(Qt::yellow);

  x = 0;
  y = height;
  for(i = 0; i < 4096; ++i)
  {
    xnew = (i * width) >> 12;
    ynew = (data[i] * height) >> 8;
    if(xnew > x)
    {
      curve[x].setX(x);
      curve[x].setY(y);
      x = xnew;
      y = height;
    }
    else if(ynew < y)
    {
      y = ynew;
    }
  }

  painter.drawPolyline(curve, width - 1);

  update();
}

//------------------------------------------------------------------------------

void Spectrum::paint(QPainter *painter)
{
  QRectF bounds = boundingRect();
  QSize size = m_Pixmap->size();

  if(size.width() != bounds.width() || size.height() != bounds.height())
  {
    delete m_Pixmap;
    m_Pixmap = new QPixmap(bounds.width(), bounds.height());
    m_Pixmap->fill(Qt::black);
  }

  painter->drawPixmap(bounds, *m_Pixmap, bounds);
}
