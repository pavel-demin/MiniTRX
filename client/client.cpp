#include <stdint.h>

#include <iostream>

#include <QtGui/QtGui>
#include <QtUiTools/QUiLoader>
#include <QtCore/QFile>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioOutput>

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
  QWidget(parent), m_IndicatorRX(0), m_IndicatorTX(0), m_IndicatorFFT(0),
  m_LevelRX(0), m_LevelTX(0), m_Range(0), m_Offset(0), m_Plotter(0)
{
  CustomUiLoader loader;
  QString buffer;

  QFile file("MiniTRX-client.ui");
  file.open(QFile::ReadOnly);
  QWidget *formWidget = loader.load(&file, this);
  file.close();

  m_IndicatorRX = findChild<Indicator *>("IndicatorRX");
  m_IndicatorTX = findChild<Indicator *>("IndicatorTX");
  m_IndicatorFFT = findChild<Indicator *>("IndicatorFFT");

  m_LevelRX = findChild<QProgressBar *>("LevelRX");
  m_LevelTX = findChild<QProgressBar *>("LevelTX");

  m_Range = findChild<QSlider *>("Range");
  m_Offset = findChild<QSlider *>("Offset");

  m_Plotter = findChild<CPlotter *>("Plotter");

	connect(m_IndicatorRX, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorRX_changed(int)));
	connect(m_Plotter, SIGNAL(NewDemodFreq(int)), this, SLOT(on_FrequencyRX_changed(int)));
	connect(m_IndicatorFFT, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorFFT_changed(int)));
	connect(m_Range, SIGNAL(valueChanged(int)), this, SLOT(on_Range_changed(int)));
	connect(m_Offset, SIGNAL(valueChanged(int)), this, SLOT(on_Offset_changed(int)));

  m_IndicatorRX->setValue(600000);
  m_IndicatorRX->setFrameShape(QFrame::Panel);
  m_IndicatorRX->setFrameShadow(QFrame::Sunken);
  m_IndicatorTX->setValue(600000);
  m_IndicatorTX->setFrameShape(QFrame::Panel);
  m_IndicatorTX->setFrameShadow(QFrame::Sunken);
  m_IndicatorFFT->setValue(600000);
  m_IndicatorFFT->setFrameShape(QFrame::Panel);
  m_IndicatorFFT->setFrameShadow(QFrame::Sunken);
  m_IndicatorFFT->setDeltaMin(1000);

  m_DeviceRX = findChild<QComboBox *>("DeviceRX");
  const QAudioDeviceInfo &defaultOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
  m_DeviceRX->addItem(defaultOutputDevice.deviceName(), qVariantFromValue(defaultOutputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
  {
    if(device != defaultOutputDevice) m_DeviceRX->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_DeviceRX, SIGNAL(activated(int)), this, SLOT(on_DeviceTX_activated(int)));

  m_DeviceTX = findChild<QComboBox *>("DeviceTX");
  const QAudioDeviceInfo &defaultInputDevice = QAudioDeviceInfo::defaultInputDevice();
  m_DeviceTX->addItem(defaultInputDevice.deviceName(), qVariantFromValue(defaultInputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
  {
    if(device != defaultInputDevice) m_DeviceTX->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_DeviceTX, SIGNAL(activated(int)), this, SLOT(on_DeviceTX_activated(int)));

  QMetaObject::connectSlotsByName(this);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
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
	m_Plotter->SetDemodCenterFreq(freq);
	m_Plotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_FrequencyRX_changed(int freq)
{
	m_IndicatorRX->setValue(freq);
}

//------------------------------------------------------------------------------

void Client::on_IndicatorFFT_changed(int freq)
{
	m_Plotter->SetCenterFreq(freq);
	m_Plotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_Range_changed(int range)
{
  int step = 10;
  int offset = m_Offset->value();
  switch(range)
  {
    case 0: step = 1; break;
    case 1: step = 2; break;
    case 2: step = 3; break;
    case 3: step = 5; break;
    case 4: step = 10; break;
    case 5: step = 15; break;
  }
  m_Plotter->SetMaxdB(-200 + offset*5 + step*12);
  m_Plotter->SetdBStepSize(step);
	m_Plotter->UpdateOverlay();
}

//------------------------------------------------------------------------------

void Client::on_Offset_changed(int offset)
{
  int step = 10;
  switch(m_Range->value())
  {
    case 0: step = 1; break;
    case 1: step = 2; break;
    case 2: step = 3; break;
    case 3: step = 5; break;
    case 4: step = 10; break;
    case 5: step = 15; break;
  }
  m_Plotter->SetMaxdB(-200 + offset*5 + step*12);
  m_Plotter->SetdBStepSize(step);
	m_Plotter->UpdateOverlay();
}
