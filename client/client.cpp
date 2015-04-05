#include <stdint.h>

#include <iostream>

#include <QtGui>
#include <QtUiTools/QUiLoader>
#include <QtCore/QFile>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>

#include "client.h"
#include "indicator.h"
#include "plotter.h"

using namespace std;

//------------------------------------------------------------------------------

class CustomUiLoader: public QUiLoader
{
public:
  CustomUiLoader(QObject *parent = 0): QUiLoader(parent) {}
  virtual QWidget *createWidget(const QString &className, QWidget *parent = 0, const QString &name = QString())
  {
    QWidget *widget;
    if(className == QString("Indicator"))
    {
      widget = new Indicator(parent);
      widget->setObjectName(name);
      return widget;
    }
    if(className == QString("CPlotter"))
    {
      widget = new CPlotter(parent);
      widget->setObjectName(name);
      return widget;
    }
    return QUiLoader::createWidget(className, parent, name);
  }
};

//------------------------------------------------------------------------------

Client::Client(QWidget *parent):
  QWidget(parent), fIndicatorRX(0), fIndicatorTX(0), fIndicatorFFT(0),
  fLevelRX(0), fLevelTX(0), fRange(0), fOffset(0), fPlotter(0)
{
  CustomUiLoader loader;
  QString buffer;

  QFile file("MiniTRX-client.ui");
  file.open(QFile::ReadOnly);
  QWidget *formWidget = loader.load(&file, this);
  file.close();

  fIndicatorRX = findChild<Indicator *>("IndicatorRX");
  fIndicatorTX = findChild<Indicator *>("IndicatorTX");
  fIndicatorFFT = findChild<Indicator *>("IndicatorFFT");

  fLevelRX = findChild<QProgressBar *>("LevelRX");
  fLevelTX = findChild<QProgressBar *>("LevelTX");

  fRange = findChild<QSlider *>("Range");
  fOffset = findChild<QSlider *>("Offset");

  fPlotter = findChild<CPlotter *>("Plotter");

	connect(fIndicatorRX, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorRX_changed(int)));
	connect(fPlotter, SIGNAL(NewDemodFreq(int)), this, SLOT(on_FrequencyRX_changed(int)));
	connect(fIndicatorFFT, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorFFT_changed(int)));
	connect(fRange, SIGNAL(valueChanged(int)), this, SLOT(on_Range_changed(int)));
	connect(fOffset, SIGNAL(valueChanged(int)), this, SLOT(on_Offset_changed(int)));

  fIndicatorRX->setValue(600000);
  fIndicatorRX->setFrameShape(QFrame::Panel);
  fIndicatorRX->setFrameShadow(QFrame::Sunken);
  fIndicatorTX->setValue(600000);
  fIndicatorTX->setFrameShape(QFrame::Panel);
  fIndicatorTX->setFrameShadow(QFrame::Sunken);
  fIndicatorFFT->setValue(600000);
  fIndicatorFFT->setFrameShape(QFrame::Panel);
  fIndicatorFFT->setFrameShadow(QFrame::Sunken);
  fIndicatorFFT->setDeltaMin(1000);

  QMetaObject::connectSlotsByName(this);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(formWidget);
  setLayout(layout);

  setFixedSize(layout->maximumSize());

}

//------------------------------------------------------------------------------

Client::~Client()
{
}

//------------------------------------------------------------------------------

/*
void Client::on_Quit_clicked()
{
  qApp->quit();
}

*/

//------------------------------------------------------------------------------

void Client::on_IndicatorRX_changed(int freq)
{
	fPlotter->SetDemodCenterFreq(freq);
	fPlotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_FrequencyRX_changed(int freq)
{
	fIndicatorRX->setValue(freq);
}

//------------------------------------------------------------------------------

void Client::on_IndicatorFFT_changed(int freq)
{
	fPlotter->SetCenterFreq(freq);
	fPlotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_Range_changed(int range)
{
  int step = 10;
  int offset = fOffset->value();
  switch(range)
  {
    case 0: step = 1; break;
    case 1: step = 2; break;
    case 2: step = 5; break;
    case 3: step = 10; break;
    case 4: step = 15; break;
  }
  fPlotter->SetMaxdB(-200 + offset*5 + step*12);
  fPlotter->SetdBStepSize(step);
	fPlotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_Offset_changed(int offset)
{
  int step = 10;
  switch(fRange->value())
  {
    case 0: step = 1; break;
    case 1: step = 2; break;
    case 2: step = 5; break;
    case 3: step = 10; break;
    case 4: step = 15; break;
  }
  fPlotter->SetMaxdB(-200 + offset*5 + step*12);
  fPlotter->SetdBStepSize(step);
	fPlotter->UpdateOverlay();
}
