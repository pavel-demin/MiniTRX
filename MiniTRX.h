#ifndef MiniTRX_h
#define MiniTRX_h

#include <QtWidgets/QWidget>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QRadioButton;

class MiniTRX: public QWidget
{
  Q_OBJECT

public:
  MiniTRX(QWidget *parent = 0);
  virtual ~MiniTRX();

private slots:
/*
  virtual void on_Hello_clicked();
  virtual void on_Quit_clicked();
  virtual void on_Scale_stateChanged();
*/
private:

  QWidget *fSpectrum;

};

#endif
