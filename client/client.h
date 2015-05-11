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

#ifndef Client_h
#define Client_h

#include <stdint.h>

#include <QObject>
#include <QList>
#include <QStringList>
#include <QtMultimedia/QAudioDeviceInfo>

class QByteArray;
class QAudioFormat;
class QAudioInput;
class QAudioOutput;
class QIODevice;
class QWebSocket;

class Spectrum;
class Waterfall;

class Client: public QObject
{
  Q_OBJECT

public:
  Client(QObject *parent = 0);
  virtual ~Client();

  void setSpectrum(Spectrum *spectrum) { m_Spectrum = spectrum; }
  void setWaterfall(Waterfall *waterfall) { m_Waterfall = waterfall; }

  Q_INVOKABLE QStringList outputDeviceList();
  Q_INVOKABLE QStringList inputDeviceList();

public slots:
  void on_Connect_clicked(QString address);
  void on_Disconnect_clicked();
  void on_StartRX_clicked();
  void on_StopRX_clicked();
  void on_StartFFT_clicked();
  void on_StopFFT_clicked();
  void on_StartTX_clicked();
  void on_StopTX_clicked();
  void on_IndicatorRX_changed(int freq);
  void on_IndicatorFFT_changed(int freq);
  void on_IndicatorTX_changed(int freq);
  void on_InputDevice_changed(int index);
  void on_OutputDevice_changed(int index);

private slots:
/*
  void on_FrequencyRX_changed(int freq);
  void on_Range_changed(int range);
  void on_Offset_changed(int offset);
*/
  void on_AudioInput_notify();
  void on_AudioOutput_notify();

  void on_WebSocket_connected();
  void on_WebSocket_disconnected();
  void on_WebSocket_binaryMessageReceived(QByteArray message);

private:
  void sendCommand();

  Spectrum *m_Spectrum;
  Waterfall *m_Waterfall;

  QByteArray *m_BufferCmd;
  int32_t *m_Command;
  int32_t *m_DataInt;
  float *m_DataFloat;

  QStringList m_InputDeviceList;
  QList<QAudioDeviceInfo> m_InputDeviceInfoList;
  QStringList m_OutputDeviceList;
  QList<QAudioDeviceInfo> m_OutputDeviceInfoList;

  QAudioFormat *m_AudioFormat;
  QAudioInput *m_AudioInput;
  QAudioOutput *m_AudioOutput;
  QIODevice *m_AudioInputDevice;
  QIODevice *m_AudioOutputDevice;

  QWebSocket *m_WebSocket;
};

#endif
