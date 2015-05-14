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

#include "grid.h"

//------------------------------------------------------------------------------

Grid::Grid(QQuickItem *parent):
  QQuickPaintedItem(parent)
{
  setFlag(ItemHasContents, true);
}

//------------------------------------------------------------------------------

Grid::~Grid()
{

}

//------------------------------------------------------------------------------

void Grid::paint(QPainter *painter)
{
  int i, width, height, x, y;
  QRectF bounds = boundingRect();

  width = bounds.width();
  height = bounds.height();

  painter->setPen(QPen(Qt::lightGray, 1.0, Qt::DashLine));

  for(i = 0; i < 9; ++i)
  {
    x = (i + 1) * width / 10;
    y = (i + 1) * height / 10;
    painter->drawLine(x, 0, x, height);
    painter->drawLine(0, y, width, y);
  }
}
