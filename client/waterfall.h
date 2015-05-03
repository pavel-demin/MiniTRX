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
