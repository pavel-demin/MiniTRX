/*
 *  MiniTRX: minimalist user interface for the Red Pitaya SDR transceiver
 *  Copyright (C) 2014-2015  Pavel Demin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>

#include <QtGui/QPixmap>
#include <QtGui/QPainter>

#include "waterfall.h"

//------------------------------------------------------------------------------

Waterfall::Waterfall(QQuickItem *parent):
  QQuickPaintedItem(parent), m_Pixmap(0), m_Colors(0)
{
  int i;
  m_Pixmap = new QPixmap(0, 0);
  m_Colors = new QColor[256];
  for(i = 0; i < 256; ++i)
  {
    if(i < 43) m_Colors[i].setRgb(0, 0, 255*(i)/43);
    if(i >=  43 && i <  87) m_Colors[i].setRgb(0, 255*(i-43)/43, 255);
    if(i >=  87 && i < 120) m_Colors[i].setRgb(0, 255, 255 - 255*(i-87)/32);
    if(i >= 120 && i < 154) m_Colors[i].setRgb(255*(i-120)/33, 255, 0);
    if(i >= 154 && i < 217) m_Colors[i].setRgb(255, 255 - 255*(i-154)/62, 0);
    if(i >= 217) m_Colors[i].setRgb(255, 0, 128*(i-217)/38);
  }
  setFlag(ItemHasContents, true);
}

//------------------------------------------------------------------------------

Waterfall::~Waterfall()
{
  if(m_Pixmap) delete m_Pixmap;
  if(m_Colors) delete[] m_Colors;
}

//------------------------------------------------------------------------------

void Waterfall::setData(unsigned char *data)
{
  QPainter painter;
  int i, width, height;
  int x, xnew;
  uint8_t y, ynew;
 
  width = m_Pixmap->width();
  height = m_Pixmap->height();
  m_Pixmap->scroll(0, 1, 0, 0, width, height);
  painter.begin(m_Pixmap);
  
  x = 0;
  y = 255; 
  for(i = 0; i < 4096; ++i)
  {
    xnew = (i * width) >> 12;
    ynew = data[i];
    if(xnew > x)
    {
      painter.setPen(m_Colors[y]);
      painter.drawPoint(x + 0.5, 0.5);
      x = xnew;
      y = 255;
    }
    else if(ynew < y)
    {
      y = ynew;
    }
  }

  update();
}

//------------------------------------------------------------------------------

void Waterfall::paint(QPainter *painter)
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
