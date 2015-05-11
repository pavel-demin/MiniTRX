#ifndef Spectrum_h
#define Spectrum_h

#include <vector>

#include <QQuickItem>

class Spectrum: public QQuickItem
{
  Q_OBJECT

public:
  Spectrum(QQuickItem *parent = 0);
  ~Spectrum();

  void setData(unsigned char *data);

  QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data);

private:
  std::vector<float> m_Data;
  int m_Width, m_Height;
};

#endif
