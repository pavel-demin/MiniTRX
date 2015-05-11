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

#include <stdint.h>

#include <iostream>

#include <QQuickItem>
#include <QStringList>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QtWebSockets/QWebSocket>

#include "client.h"
#include "spectrum.h"
#include "waterfall.h"

//------------------------------------------------------------------------------

Client::Client(QObject *parent):
  QObject(parent), m_Spectrum(0), m_Waterfall(0),
/*
  m_IndicatorRX(0), m_IndicatorTX(0), m_IndicatorFFT(0),
  m_LevelRX(0), m_LevelTX(0), m_Range(0), m_Offset(0), m_Plotter(0),
  m_Address(0), m_Connect(0), m_EnableRX(0), m_EnableTX(0), m_EnableFFT(0),
  m_InputDevice(0), m_OutputDevice(0),
*/
  m_BufferCmd(0), m_Command(0), m_DataInt(0), m_DataFloat(0),
  m_AudioFormat(0), m_AudioInput(0), m_AudioOutput(0),
  m_AudioInputDevice(0), m_AudioOutputDevice(0),
  m_WebSocket(0)
{
  m_BufferCmd = new QByteArray();
  m_BufferCmd->resize(48);
  m_Command = (int32_t *)(m_BufferCmd->constData() + 0);
  m_DataInt = (int32_t *)(m_BufferCmd->constData() + 4);
  m_DataFloat = (float *)(m_BufferCmd->constData() + 4);
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
  m_OutputDeviceList << defaultOutputDevice.deviceName();
  m_OutputDeviceInfoList << defaultOutputDevice;
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
  {
    if(device != defaultOutputDevice)
    {
      m_OutputDeviceList << device.deviceName();
      m_OutputDeviceInfoList << device;
    }
  }
  m_AudioOutput = new QAudioOutput(defaultOutputDevice, *m_AudioFormat, this);
  m_AudioOutput->setBufferSize(16384);

  QStringList list;
  const QAudioDeviceInfo &defaultInputDevice = QAudioDeviceInfo::defaultInputDevice();
  m_InputDeviceList << defaultInputDevice.deviceName();
  m_InputDeviceInfoList << defaultInputDevice;
  foreach(const QAudioDeviceInfo &device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
  {
    if(device != defaultInputDevice)
    {
      m_InputDeviceList << device.deviceName();
      m_InputDeviceInfoList << device;
    }
  }
  m_AudioInput = new QAudioInput(defaultInputDevice, *m_AudioFormat, this);
  m_AudioInput->setBufferSize(16384);

  m_WebSocket = new QWebSocket();
  connect(m_WebSocket, SIGNAL(connected()), this, SLOT(on_WebSocket_connected()));
  connect(m_WebSocket, SIGNAL(disconnected()), this, SLOT(on_WebSocket_disconnected()));
}

//------------------------------------------------------------------------------

Client::~Client()
{
}

//------------------------------------------------------------------------------

QStringList Client::outputDeviceList()
{
  return m_OutputDeviceList;
}

//------------------------------------------------------------------------------

QStringList Client::inputDeviceList()
{
  return m_InputDeviceList;
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
  *m_Command = 1;
  sendCommand();
}

//------------------------------------------------------------------------------

void Client::on_StopRX_clicked()
{
  *m_Command = 2;
  sendCommand();
  m_AudioOutput->stop();
  m_AudioOutputDevice = 0;
}

//------------------------------------------------------------------------------

void Client::on_StartFFT_clicked()
{
  *m_Command = 3;
  sendCommand();
}

//------------------------------------------------------------------------------

void Client::on_StopFFT_clicked()
{
  *m_Command = 4;
  sendCommand();
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
  int32_t command;
  command = *(int32_t *)(message.constData() + 0);
  switch(command)
  {
    case 0:
      // RX data
      if(m_AudioOutputDevice) m_AudioOutputDevice->write(message.constData() + 4, 2048 * sizeof(int16_t));
      break;
    case 1:
      // FFT data
      if(m_Spectrum) m_Spectrum->setData((uint8_t *)(message.constData() + 4));
      if(m_Waterfall) m_Waterfall->setData((uint8_t *)(message.constData() + 4));
      break;
  }
}

//------------------------------------------------------------------------------

void Client::on_OutputDevice_changed(int index)
{
  bool active = m_AudioOutputDevice;
  const QAudioDeviceInfo &device = m_OutputDeviceInfoList[index];

  if(active) on_StopRX_clicked();

  delete m_AudioOutput;
  m_AudioOutput = new QAudioOutput(device, *m_AudioFormat, this);
  m_AudioOutput->setBufferSize(16384);

  if(active) on_StartRX_clicked();
}

//------------------------------------------------------------------------------

void Client::on_InputDevice_changed(int index)
{
  bool active = m_AudioInputDevice;
  const QAudioDeviceInfo &device = m_InputDeviceInfoList[index];

  if(active) on_StopTX_clicked();

  delete m_AudioInput;
  m_AudioInput = new QAudioInput(device, *m_AudioFormat, this);
  m_AudioInput->setBufferSize(16384);

  if(active) on_StartTX_clicked();
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
  *m_Command = 8;
  m_DataInt[0] = freq;
  sendCommand();
}

//------------------------------------------------------------------------------

void Client::on_IndicatorFFT_changed(int freq)
{
  *m_Command = 9;
  m_DataInt[0] = freq;
  sendCommand();
}

//------------------------------------------------------------------------------

void Client::on_IndicatorTX_changed(int freq)
{
  *m_Command = 10;
  m_DataInt[0] = freq;
  sendCommand();
}

//------------------------------------------------------------------------------
/*
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