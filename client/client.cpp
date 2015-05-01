#include <stdint.h>

#include <iostream>

#include <QQuickItem>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QtWebSockets/QWebSocket>

#include "client.h"

//------------------------------------------------------------------------------

Client::Client(QObject *parent):
  QObject(parent), m_Root(0),
/*
  m_IndicatorRX(0), m_IndicatorTX(0), m_IndicatorFFT(0),
  m_LevelRX(0), m_LevelTX(0), m_Range(0), m_Offset(0), m_Plotter(0),
  m_Address(0), m_Connect(0), m_EnableRX(0), m_EnableTX(0), m_EnableFFT(0),
  m_InputDevice(0), m_OutputDevice(0),
*/
  m_BufferCmd(0), m_PointerCmd(0),
  m_AudioFormat(0), m_AudioInput(0), m_AudioOutput(0),
  m_AudioInputDevice(0), m_AudioOutputDevice(0),
  m_WebSocket(0)
{
  m_BufferCmd = new QByteArray();
  m_BufferCmd->resize(48);
  m_PointerCmd = m_BufferCmd->constData();
/*
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
*/
  m_AudioFormat = new QAudioFormat;

  m_AudioFormat->setSampleRate(22050);
  m_AudioFormat->setChannelCount(2);
  m_AudioFormat->setSampleSize(16);
  m_AudioFormat->setCodec("audio/pcm");
  m_AudioFormat->setByteOrder(QAudioFormat::LittleEndian);
  m_AudioFormat->setSampleType(QAudioFormat::SignedInt);

  const QAudioDeviceInfo &defaultOutputDevice = QAudioDeviceInfo::defaultOutputDevice();
/*
  m_OutputDevice = findChild<QComboBox *>("OutputDevice");
  m_OutputDevice->addItem(defaultOutputDevice.deviceName(), qVariantFromValue(defaultOutputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
  {
    if(device != defaultOutputDevice) m_OutputDevice->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_OutputDevice, SIGNAL(activated(int)), this, SLOT(on_OutputDevice_activated(int)));
*/
  m_AudioOutput = new QAudioOutput(defaultOutputDevice, *m_AudioFormat, this);
  m_AudioOutput->setBufferSize(16384);

  const QAudioDeviceInfo &defaultInputDevice = QAudioDeviceInfo::defaultInputDevice();
/*
  m_InputDevice = findChild<QComboBox *>("InputDevice");
  m_InputDevice->addItem(defaultInputDevice.deviceName(), qVariantFromValue(defaultInputDevice));
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
  {
    if(device != defaultInputDevice) m_InputDevice->addItem(device.deviceName(), qVariantFromValue(device));
  }
  connect(m_InputDevice, SIGNAL(activated(int)), this, SLOT(on_InputDevice_activated(int)));
*/
  m_AudioInput = new QAudioInput(defaultInputDevice, *m_AudioFormat, this);

  m_WebSocket = new QWebSocket();
  connect(m_WebSocket, SIGNAL(connected()), this, SLOT(on_WebSocket_connected()));
  connect(m_WebSocket, SIGNAL(disconnected()), this, SLOT(on_WebSocket_disconnected()));
}

//------------------------------------------------------------------------------

Client::~Client()
{
}

//------------------------------------------------------------------------------

void Client::sendCommand()
{
  if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_BufferCmd);
}

//------------------------------------------------------------------------------

void Client::on_StartRX_clicked()
{
  m_AudioOutputDevice = m_AudioOutput->start();
  *(uint32_t *)(m_PointerCmd + 0) = 1;
  sendCommand();
}

//------------------------------------------------------------------------------

void Client::on_StopRX_clicked()
{
  *(uint32_t *)(m_PointerCmd + 0) = 2;
  sendCommand();
  m_AudioOutput->stop();
  m_AudioOutputDevice = 0;
}

//------------------------------------------------------------------------------

void Client::on_StartTX_clicked()
{
}

//------------------------------------------------------------------------------

void Client::on_StopTX_clicked()
{
}

//------------------------------------------------------------------------------

void Client::on_StartFFT_clicked()
{
}

//------------------------------------------------------------------------------

void Client::on_StopFFT_clicked()
{
}

//------------------------------------------------------------------------------
/*
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
*/
//------------------------------------------------------------------------------
/*
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
*/
//------------------------------------------------------------------------------

void Client::on_Connect_clicked(QString address)
{
  m_WebSocket->open(QString("ws://") + address + QString(":1001"));
}

//------------------------------------------------------------------------------

void Client::on_Disconnect_clicked()
{
  m_WebSocket->close();
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
/*
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
*/