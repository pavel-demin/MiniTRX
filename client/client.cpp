#include <stdint.h>

#include <iostream>

#include <QtGui/QtGui>
#include <QtUiTools/QUiLoader>
#include <QtCore/QFile>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGraphicsColorizeEffect>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QtWebSockets/QWebSocket>

#include "client.h"
#include "indicator.h"
#include "plotter.h"

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
  m_LevelRX(0), m_LevelTX(0), m_Range(0), m_Offset(0), m_Plotter(0),
  m_Address(0), m_Connect(0), m_EnableRX(0), m_EnableTX(0), m_EnableFFT(0),
  m_InputDevice(0), m_OutputDevice(0),
  m_BufferCmd(0), m_PointerCmd(0),
  m_AudioFormat(0), m_AudioInput(0), m_AudioOutput(0),
  m_AudioInputDevice(0), m_AudioOutputDevice(0),
  m_WebSocket(0)
{
  CustomUiLoader loader;
  QString buffer;

  m_BufferCmd = new QByteArray();
  m_BufferCmd->resize(48);
  m_PointerCmd = m_BufferCmd->constData();

  QFile file(":/forms/client.ui");
  file.open(QFile::ReadOnly);
  QWidget *widget = loader.load(&file, this);
  file.close();

  m_IndicatorRX = findChild<Indicator *>("IndicatorRX");
  m_IndicatorTX = findChild<Indicator *>("IndicatorTX");
  m_IndicatorFFT = findChild<Indicator *>("IndicatorFFT");

  m_LevelRX = findChild<QProgressBar *>("LevelRX");
  m_LevelTX = findChild<QProgressBar *>("LevelTX");

  m_Range = findChild<QSlider *>("Range");
  m_Offset = findChild<QSlider *>("Offset");

  m_Plotter = findChild<CPlotter *>("Plotter");

  m_Address = findChild<QLineEdit *>("Address");

  m_Connect = findChild<QPushButton *>("Connect");
  m_EnableRX = findChild<QPushButton *>("EnableRX");
  m_EnableTX = findChild<QPushButton *>("EnableTX");
  m_EnableFFT = findChild<QPushButton *>("EnableFFT");

  connect(m_IndicatorRX, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorRX_changed(int)));
  connect(m_Plotter, SIGNAL(NewDemodFreq(int)), this, SLOT(on_FrequencyRX_changed(int)));
  connect(m_IndicatorFFT, SIGNAL(valueChanged(int)), this, SLOT(on_IndicatorFFT_changed(int)));
  connect(m_Range, SIGNAL(valueChanged(int)), this, SLOT(on_Range_changed(int)));
  connect(m_Offset, SIGNAL(valueChanged(int)), this, SLOT(on_Offset_changed(int)));

  m_IndicatorRX->setSize(8);
  m_IndicatorRX->setValueMin(0);
  m_IndicatorRX->setValueMax(50000000);
  m_IndicatorRX->setValue(600000);
  m_IndicatorRX->setFrameShape(QFrame::Panel);
  m_IndicatorRX->setFrameShadow(QFrame::Sunken);
  m_IndicatorTX->setSize(8);
  m_IndicatorTX->setValueMin(0);
  m_IndicatorTX->setValueMax(50000000);
  m_IndicatorTX->setValue(600000);
  m_IndicatorTX->setFrameShape(QFrame::Panel);
  m_IndicatorTX->setFrameShadow(QFrame::Sunken);
  m_IndicatorFFT->setSize(8);
  m_IndicatorFFT->setValueMin(0);
  m_IndicatorFFT->setValueMax(50000000);
  m_IndicatorFFT->setValue(600000);
  m_IndicatorFFT->setFrameShape(QFrame::Panel);
  m_IndicatorFFT->setFrameShadow(QFrame::Sunken);
  m_IndicatorFFT->setDeltaMin(1000);

  m_AudioFormat = new QAudioFormat;

  m_AudioFormat->setSampleRate(22050);
  m_AudioFormat->setChannelCount(2);
  m_AudioFormat->setSampleSize(16);
  m_AudioFormat->setCodec("audio/pcm");
  m_AudioFormat->setByteOrder(QAudioFormat::LittleEndian);
  m_AudioFormat->setSampleType(QAudioFormat::SignedInt);

  m_OutputDevice = findChild<QComboBox *>("OutputDevice");
  const QAudioDeviceInfo &defaultOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
  m_OutputDevice->addItem(defaultOutputDevice.deviceName(), qVariantFromValue(defaultOutputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
  {
    if(device != defaultOutputDevice) m_OutputDevice->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_OutputDevice, SIGNAL(activated(int)), this, SLOT(on_OutputDevice_activated(int)));

  m_AudioOutput = new QAudioOutput(defaultOutputDevice, *m_AudioFormat, this);
  m_AudioOutput->setBufferSize(16384);
  //m_AudioOutputDevice = m_AudioOutput->start();
  //connect(m_AudioOutput, SIGNAL(notify()), this, SLOT(on_AudioOutput_notify()));

  m_InputDevice = findChild<QComboBox *>("InputDevice");
  const QAudioDeviceInfo &defaultInputDevice = QAudioDeviceInfo::defaultInputDevice();
  m_InputDevice->addItem(defaultInputDevice.deviceName(), qVariantFromValue(defaultInputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
  {
    if(device != defaultInputDevice) m_InputDevice->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_InputDevice, SIGNAL(activated(int)), this, SLOT(on_InputDevice_activated(int)));

  m_AudioInput = new QAudioInput(defaultInputDevice, *m_AudioFormat, this);
  //m_AudioInputDevice = m_AudioInput->start();
  //connect(m_AudioInput, SIGNAL(notify()), this, SLOT(on_AudioInput_notify()));

  QMetaObject::connectSlotsByName(this);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(widget);
  setLayout(layout);

  setFixedSize(layout->maximumSize());

  qRegisterMetaType<QAbstractSocket::SocketState>();

  m_WebSocket = new QWebSocket();
  connect(m_WebSocket, SIGNAL(connected()), this, SLOT(on_WebSocket_connected()));
  connect(m_WebSocket, SIGNAL(disconnected()), this, SLOT(on_WebSocket_disconnected()));
}

//------------------------------------------------------------------------------

Client::~Client()
{
}

//------------------------------------------------------------------------------

void Client::SendCommand()
{
  if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_BufferCmd);
}

//------------------------------------------------------------------------------

void Client::on_EnableRX_clicked()
{
  if(m_EnableRX->isChecked())
  {
    *(uint32_t *)(m_PointerCmd + 0) = 1;
    m_EnableRX->setText(QString("OFF"));
    m_AudioOutputDevice = m_AudioOutput->start();
  }
  else
  {
    *(uint32_t *)(m_PointerCmd + 0) = 2;
    m_EnableRX->setText(QString("ON"));
    m_AudioOutput->stop();
    m_AudioOutputDevice = 0;
  }
  SendCommand();
}

//------------------------------------------------------------------------------

void Client::on_EnableFFT_clicked()
{
  if(m_EnableFFT->isChecked())
  {
    *(uint32_t *)(m_PointerCmd + 0) = 3;
    m_EnableFFT->setText(QString("OFF"));
  }
  else
  {
    *(uint32_t *)(m_PointerCmd + 0) = 4;
    m_EnableFFT->setText(QString("ON"));
  }
  SendCommand();
}

//------------------------------------------------------------------------------

void Client::on_EnableTX_clicked()
{
  if(m_EnableTX->isChecked())
  {
    *(uint32_t *)(m_PointerCmd + 0) = 5;
    m_EnableTX->setText(QString("OFF"));
  }
  else
  {
    *(uint32_t *)(m_PointerCmd + 0) = 6;
    m_EnableTX->setText(QString("ON"));
  }
  SendCommand();
}

//------------------------------------------------------------------------------

void Client::on_Connect_clicked()
{
  if(m_Connect->isChecked())
  {
    m_WebSocket->open(QString("ws://") + m_Address->text() + QString(":1001"));
    m_Connect->setText(QString("Disconnect"));
  }
  else
  {
    m_WebSocket->close();
    m_Connect->setText(QString("Connect"));
  }
}

//------------------------------------------------------------------------------

void Client::on_WebSocket_connected()
{
  connect(m_WebSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(on_WebSocket_binaryMessageReceived(QByteArray)));
}

//------------------------------------------------------------------------------

void Client::on_WebSocket_disconnected()
{
  disconnect(m_WebSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(on_WebSocket_binaryMessageReceived(QByteArray)));
}

//------------------------------------------------------------------------------

void Client::on_WebSocket_binaryMessageReceived(QByteArray message)
{
  if(m_AudioOutputDevice) m_AudioOutputDevice->write(message.constData() + 4, 2048 * sizeof(int16_t));
}

//------------------------------------------------------------------------------

/*
void Client::on_Quit_clicked()
{
  qApp->quit();
}

*/

//------------------------------------------------------------------------------

void Client::on_OutputDevice_activated(int index)
{
/*
  const QAudioDeviceInfo &device = m_OutputDevice->itemData(index).value<QAudioDeviceInfo>();
  m_AudioOutput->stop();
  m_AudioOutput->disconnect(this);
  delete m_AudioOutput;
  m_AudioOutput = new QAudioOutput(device, *m_AudioFormat, this);
  m_AudioOutputDevice = m_AudioOutput->start();
  connect(m_AudioOutput, SIGNAL(notify()), this, SLOT(on_AudioOutput_notify()));
*/
}

//------------------------------------------------------------------------------

void Client::on_InputDevice_activated(int index)
{
/*
  const QAudioDeviceInfo &device = m_InputDevice->itemData(index).value<QAudioDeviceInfo>();
  m_AudioInput->stop();
  m_AudioInput->disconnect(this);
  delete m_AudioInput;
  m_AudioInput = new QAudioInput(device, *m_AudioFormat, this);
  m_AudioInputDevice = m_AudioInput->start();
  connect(m_AudioInput, SIGNAL(notify()), this, SLOT(on_AudioInput_notify()));
*/
}

//------------------------------------------------------------------------------

void Client::on_AudioInput_notify()
{
}

//------------------------------------------------------------------------------

void Client::on_AudioOutput_notify()
{
}

//------------------------------------------------------------------------------

void Client::on_IndicatorRX_changed(int freq)
{
  m_Plotter->SetDemodCenterFreq(freq);
  m_Plotter->UpdateOverlay();
  *(uint32_t *)(m_PointerCmd + 0) = 8;
  *(uint32_t *)(m_PointerCmd + 4) = freq;
  SendCommand();
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
  *(uint32_t *)(m_PointerCmd + 0) = 8;
  *(uint32_t *)(m_PointerCmd + 4) = freq;
}

//------------------------------------------------------------------------------

void Client::on_IndicatorTX_changed(int freq)
{
  *(uint32_t *)(m_PointerCmd + 0) = 9;
  *(uint32_t *)(m_PointerCmd + 4) = freq;
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
