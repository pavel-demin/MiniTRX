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

#include <string.h>

#include <QtQuick/QSGNode>
#include <QtQuick/QSGFlatColorMaterial>

#include "spectrum.h"

using namespace std;

//------------------------------------------------------------------------------

Spectrum::Spectrum(QQuickItem *parent):
  QQuickItem(parent), m_Width(0), m_Height(0)
{
  setFlag(ItemHasContents, true);
}

//------------------------------------------------------------------------------

Spectrum::~Spectrum()
{
}

//------------------------------------------------------------------------------

void Spectrum::setData(unsigned char *data)
{
  int i;
  int x, xnew;
  int y, ynew;

  x = 0;
  y = m_Height;
  m_Data.clear();
  for(i = 0; i < 4096; ++i)
  {
    xnew = (i * m_Width) >> 12;
    ynew = (data[i] * m_Height) >> 8;
    if(xnew > x)
    {
      m_Data.push_back(y);
      x = xnew;
      y = m_Height;
    }
    else if(ynew < y)
    {
      y = ynew;
    }
  }

  update();
}

//------------------------------------------------------------------------------

QSGNode *Spectrum::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
  int i;
  float x, y;
  QSGGeometryNode *node = 0;
  QSGGeometry *geometry = 0;
  QSGFlatColorMaterial *material = 0;
  QSGGeometry::Point2D *vertices = 0;

  QRectF bounds = boundingRect();

  m_Width = bounds.width();
  m_Height = bounds.height();
  x = bounds.x();
  y = bounds.y();

  if(!oldNode)
  {
    node = new QSGGeometryNode;
    geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), m_Width - 1);
    geometry->setLineWidth(1);
    geometry->setDrawingMode(GL_LINE_STRIP);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    material = new QSGFlatColorMaterial;
    material->setColor(Qt::yellow);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
  }
  else
  {
    node = static_cast<QSGGeometryNode *>(oldNode);
    geometry = node->geometry();
    geometry->allocate(m_Width - 1);
  }

  if(m_Data.size() < m_Width - 1)
  {
    m_Data.resize(m_Width - 1, 0.0);
  }

  vertices = geometry->vertexDataAsPoint2D();

  for(i = 0; i < m_Width - 1; ++i)
  {
    vertices[i].set(x + float(i), y + m_Data[i]);
  }

  node->markDirty(QSGNode::DirtyGeometry);

  return node;
}
