#ifndef Spectrum_h
#define Spectrum_h

#include <QQuickPaintedItem>

class QPixmap;

class Spectrum: public QQuickPaintedItem
{
  Q_OBJECT

public:
  Spectrum(QQuickItem *parent = 0);
  ~Spectrum();

  void setData(unsigned char *data);

  void paint(QPainter *painter);

private:
  QPixmap *m_Pixmap;
};

#endif
