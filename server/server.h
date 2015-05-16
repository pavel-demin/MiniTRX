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

#ifndef Server_h
#define Server_h

#include <stdint.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <samplerate.h>

class QTimer;
class QWebSocketServer;
class QWebSocket;

class Server: public QObject
{
  Q_OBJECT

public:
  Server(int16_t port, QObject *parent = 0);
  virtual ~Server();

private slots:
  void on_TimerRX_timeout();
  void on_TimerFFT_timeout();
  void on_TimerTX_timeout();
  void on_WebSocketServer_closed();
  void on_WebSocketServer_newConnection();
  void on_WebSocket_binaryMessageReceived(QByteArray message);
  void on_WebSocket_disconnected();

private:
  uint32_t *m_Cfg;
  uint16_t *m_Sts;
  float *m_BufferRX, *m_BufferTX, *m_BufferFFT;
  int32_t *m_BufferWIN;
  int m_LimitRX, m_InputOffsetRX;
  int m_LimitTX, m_InputOffsetTX;
  QByteArray *m_InputBufferRX;
  QByteArray *m_OutputBufferRX;
  QByteArray *m_OutputBufferFFT;
  int32_t m_CounterRX;
  int16_t *m_PointerRX;
  int32_t m_FreqMin;
  SRC_STATE *m_StateRX;
  SRC_DATA *m_DataRX;
  QTimer *m_TimerRX;
  QTimer *m_TimerFFT;
  QTimer *m_TimerTX;
  QWebSocketServer *m_WebSocketServer;
  QWebSocket *m_WebSocket;
};

#endif
