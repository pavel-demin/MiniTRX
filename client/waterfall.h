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

#ifndef Waterfall_h
#define Waterfall_h

#include <QQuickPaintedItem>

class QColor;
class QPixmap;

class Waterfall: public QQuickPaintedItem
{
  Q_OBJECT

public:
  Waterfall(QQuickItem *parent = 0);
  ~Waterfall();

  void setData(unsigned char *data);

  void paint(QPainter *painter);

private:
  QPixmap *m_Pixmap;
  QColor *m_Colors;
};

#endif
