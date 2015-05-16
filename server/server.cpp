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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <fftw3.h>

#include <samplerate.h>

extern "C"
{
  #include "comm.h"
}

#include "server.h"

using namespace std;

//------------------------------------------------------------------------------

Server::Server(int16_t port, QObject *parent):
  QObject(parent), m_Cfg(0), m_Sts(0),
  m_BufferRX(0), m_BufferTX(0), m_BufferFFT(0), m_BufferWIN(0),
  m_LimitRX(256), m_InputOffsetRX(0),
  m_LimitTX(0), m_InputOffsetTX(0),
  m_InputBufferRX(0), m_OutputBufferRX(0),
  m_OutputBufferFFT(0),
  m_CounterRX(0), m_PointerRX(0), m_FreqMin(25000),
  m_StateRX(0), m_DataRX(0),
  m_TimerRX(0), m_TimerFFT(0), m_TimerTX(0),
  m_WebSocketServer(0), m_WebSocket(0)
{
  int memFile;
  FILE *wisdomFile;
  int32_t i, error, *pointerInt;
  float *pointerFloat;
  int rc;

  if((memFile = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    qApp->quit();
  }

  m_Cfg = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40000000);
  m_Sts = (uint16_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40001000);
  m_BufferRX = (float *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40002000);
  m_BufferTX = (float *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40003000);
  m_BufferFFT = (float *)mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40020000);
  m_BufferWIN = (int32_t *)mmap(NULL, 4*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40010000);

  /* enter reset mode */
  *(m_Cfg + 0) &= ~255;
  /* set default rate */
  *(m_Cfg + 2) = 1250;
  /* set default phase increment */
  *(m_Cfg + 1) = uint32_t(floor(621000/125.0e6*(1<<30)+0.5));
  *(m_Cfg + 3) = uint32_t(floor(610000/125.0e6*(1<<30)+0.5));

  pointerFloat = m_BufferTX;
  for(i = 0; i < 512; ++i) *(pointerFloat++) = 0.0;

  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "r")))
  {
    fftwf_import_wisdom_from_file(wisdomFile);
    fclose(wisdomFile);
  }
  OpenChannel(0, 256, 4096, 20000, 20000, 20000, 0, 0, 0.010, 0.025, 0.000, 0.010, 0);
  OpenChannel(1, 256, 4096, 20000, 20000, 20000, 1, 0, 0.010, 0.025, 0.000, 0.010, 0);
  XCreateAnalyzer(0, &rc, 4096, 1, 1, 0);
  new_window(0, 6, 4096, 14.0);
  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "w")))
  {
    fftwf_export_wisdom_to_file(wisdomFile);
    fclose(wisdomFile);
  }

  SetRXAShiftRun(0, 0);
  SetRXAAMDRun(0, 1);
  SetRXAMode(0, RXA_AM);
  SetRXABandpassFreqs(0, -5000.0, 5000.0);
  SetRXAAGCFixed(0, 30.0);
  SetRXAAGCTop(0, 30.0);
  SetRXAEMNRRun(0, 0);

  DP a = pdisp[0];
  pointerInt = m_BufferWIN;
  for(i = 0; i < 4096; ++i) *(pointerInt++) = int32_t(floor(a->window[i]*(1<<22)+0.5));

  m_InputBufferRX = new QByteArray();
  m_InputBufferRX->resize(1078 * sizeof(float));

  m_OutputBufferRX = new QByteArray();
  m_OutputBufferRX->resize(2048 * sizeof(int16_t) + 4);
  *(uint32_t *)(m_OutputBufferRX->constData() + 0) = 0;

  m_OutputBufferFFT = new QByteArray();
  m_OutputBufferFFT->resize(4096 * sizeof(uint8_t) + 4);
  *(uint32_t *)(m_OutputBufferFFT->constData() + 0) = 1;

  m_PointerRX = (int16_t *)(m_OutputBufferRX->constData() + 4);

  m_StateRX = src_new(SRC_SINC_FASTEST, 2, &error);
  m_DataRX = new SRC_DATA;

  pointerFloat = (float *)(m_InputBufferRX->constData());

  m_DataRX->data_in = pointerFloat;
  m_DataRX->input_frames = 256;

  m_DataRX->data_out = pointerFloat + 512;
  m_DataRX->output_frames = 283 ;
  m_DataRX->src_ratio = 22050.0/20000.0;
  m_DataRX->end_of_input = 0;

  m_TimerRX = new QTimer(this);
  connect(m_TimerRX, SIGNAL(timeout()), this, SLOT(on_TimerRX_timeout()));

  m_TimerFFT = new QTimer(this);
  connect(m_TimerFFT, SIGNAL(timeout()), this, SLOT(on_TimerFFT_timeout()));

  m_WebSocketServer = new QWebSocketServer(QString("SDR"), QWebSocketServer::NonSecureMode, this);
  if(m_WebSocketServer->listen(QHostAddress::Any, port))
  {
    connect(m_WebSocketServer, SIGNAL(newConnection()), this, SLOT(on_WebSocketServer_newConnection()));
    connect(m_WebSocketServer, SIGNAL(closed()), this, SLOT(on_WebSocketServer_closed()));
  }
}

//------------------------------------------------------------------------------

Server::~Server()
{
  m_WebSocketServer->close();
  if(m_WebSocket) delete m_WebSocket;
  if(m_OutputBufferRX) delete m_OutputBufferRX;
}

//------------------------------------------------------------------------------

void Server::on_TimerRX_timeout()
{
  int32_t i, offset, position, error;
  float *bufferFloat, *pointerFloat;

  position = *(m_Sts + 0);
  if((m_LimitRX > 0 && position > m_LimitRX) || (m_LimitRX == 0 && position < 256))
  {
    offset = m_LimitRX > 0 ? 0 : 512;
    m_LimitRX = m_LimitRX > 0 ? 0 : 256;
    bufferFloat = (float *)(m_InputBufferRX->constData());
    fexchange0(0, m_BufferRX + offset, bufferFloat, &error);
    src_process(m_StateRX, m_DataRX) ;
    pointerFloat = m_DataRX->data_out;
    for(i = 0; i < m_DataRX->output_frames_gen * 2; ++i)
    {
      *(m_PointerRX++) = int16_t(floor(*(pointerFloat++) * 32767.0 + 0.5));
      ++m_CounterRX;
      if(m_CounterRX == 2048)
      {
        m_CounterRX = 0;
        m_PointerRX = (int16_t *)(m_OutputBufferRX->constData() + 4);
        if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_OutputBufferRX);
      }
    }
  }
}

//------------------------------------------------------------------------------

void Server::on_TimerFFT_timeout()
{
  int32_t i;
  float re, im;
  uint8_t *pointerInt;

  *(m_Cfg + 0) &= ~32;

  pointerInt = (uint8_t *)(m_OutputBufferFFT->constData() + 4);
  for(i = 2048; i < 4096; ++i)
  {
    re = m_BufferFFT[2 * i + 0];
    im = m_BufferFFT[2 * i + 1];
    *(pointerInt++) = uint8_t(floor(-20.0*log10(hypot(re, im)/2048.0) + 0.5));
  }
  for(i = 0; i < 2048; ++i)
  {
    re = m_BufferFFT[2 * i + 0];
    im = m_BufferFFT[2 * i + 1];
    *(pointerInt++) = uint8_t(floor(-20.0*log10(hypot(re, im)/2048.0) + 0.5));
  }

  if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_OutputBufferFFT);

  *(m_Cfg + 0) |= 32;
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_binaryMessageReceived(QByteArray message)
{
  int32_t i, size;
  int32_t command;
  int32_t *dataInt;
  float *dataFloat;
  float *pointerFloat;
  float *bufferReal, *bufferComplex;
  int flp = 1;
/*
  size = txa[0].size;
  bufferReal = (float *)(message.constData());
  bufferComplex = txa[1].inbuff;

  for(i = 0; i < size; ++i)
  {
    *(bufferComplex++) = *(bufferReal);
    *(bufferComplex++) = *(bufferReal++);
  }
*/
  command = *(int32_t *)(message.constData() + 0);
  dataInt = (int32_t *)(message.constData() + 4);
  dataFloat = (float *)(message.constData() + 4);

  switch(command)
  {
    case 0:
      // TX data
      break;
    case 1:
      // start RX
      *(m_Cfg + 0) |= 3;
      SetChannelState(0, 1, 0);
      m_TimerRX->start(6);
      break;
    case 2:
      // stop RX
      m_TimerRX->stop();
      break;
    case 3:
      // start FFT
      *(m_Cfg + 0) |= 61;
      m_TimerFFT->start(100);
      break;
    case 4:
      // stop FFT
      m_TimerFFT->stop();
      break;
    case 5:
      // start TX
      m_TimerTX->start(6);
      break;
    case 6:
      // stop TX
      m_TimerTX->stop();
      pointerFloat = m_BufferTX;
      for(i = 0; i < 512; ++i) *(pointerFloat++) = 0.0;
      break;
    case 7:
      switch(dataInt[0])
      {
        case 0:
          m_FreqMin = 25000;
          *(m_Cfg + 0) &= ~8;
          *(m_Cfg + 2) = 1250;
          *(m_Cfg + 0) |= 8;
          break;
        case 1:
          m_FreqMin = 50000;
          *(m_Cfg + 0) &= ~8;
          *(m_Cfg + 2) = 625;
          *(m_Cfg + 0) |= 8;
          break;
        case 2:
          m_FreqMin = 125000;
          *(m_Cfg + 0) &= ~8;
          *(m_Cfg + 2) = 250;
          *(m_Cfg + 0) |= 8;
          break;
        case 3:
          m_FreqMin = 250000;
          *(m_Cfg + 0) &= ~8;
          *(m_Cfg + 2) = 125;
          *(m_Cfg + 0) |= 8;
          break;
      }
      break;
    case 8:
      // set RX frequency
      if(dataInt[0] < 10000 || dataInt[0] > 50000000) break;
      *(m_Cfg + 1) = uint32_t(floor(dataInt[0]/125.0e6*(1<<30)+0.5));
      break;
    case 9:
      // set FFT frequency
      if(dataInt[0] < m_FreqMin || dataInt[0] > 50000000) break;
      *(m_Cfg + 3) = uint32_t(floor(dataInt[0]/125.0e6*(1<<30)+0.5));
      break;
    case 10:
      // set TX frequency
      if(dataInt[0] < 10000 || dataInt[0] > 50000000) break;
      *(m_Cfg + 4) = uint32_t(floor(dataInt[0]/125.0e6*(1<<30)+0.5));
      break;
    case 11:
      // set RX mode
      if(dataInt[0] < 0 || dataInt[0] > 11) break;
      SetRXAMode(0, dataInt[0]);
      break;
    case 12:
      // set TX mode
      if(dataInt[0] < 0 || dataInt[0] > 11) break;
      SetTXAMode(1, dataInt[0]);
      break;
    case 13:
      // set RX filter
      if(dataFloat[0] < -9.0e3 || dataInt[0] > 9.0e3) break;
      if(dataFloat[1] < -9.0e3 || dataInt[1] > 9.0e3) break;
      SetRXABandpassFreqs(0, dataFloat[0], dataFloat[1]);
      break;
    case 14:
      // set TX filter
      if(dataFloat[0] < -9.0e3 || dataInt[0] > 9.0e3) break;
      if(dataFloat[1] < -9.0e3 || dataInt[1] > 9.0e3) break;
      SetTXABandpassFreqs(1, dataFloat[0], dataFloat[1]);
      break;
    case 15:
      // set RX AGC mode
      if(dataInt[0] < 0 || dataInt[0] > 5) break;
      SetRXAAGCMode(0, dataInt[0]);
      break;
    case 16:
      // set RX AGC fixed gain
      if(dataFloat[0] < 0.0 || dataFloat[0] > 120.0) break;
      SetRXAAGCFixed(0, dataFloat[0]);
      break;
    case 17:
      // set RX AGC top gain
      if(dataFloat[0] < 0.0 || dataFloat[0] > 120.0) break;
      SetRXAAGCTop(0, dataFloat[0]);
      break;
    case 18:
      // set RX AGC slope
      if(dataInt[0] < 0 || dataInt[0] > 20) break;
      SetRXAAGCSlope(0, dataInt[0]);
      break;
    case 19:
      // set RX AGC decay
      if(dataInt[0] < 0 || dataInt[0] > 10000) break;
      SetRXAAGCDecay(0, dataInt[0]);
    case 20:
      // set RX AGC hang
      if(dataInt[0] < 0 || dataInt[0] > 10000) break;
      SetRXAAGCHang(0, dataInt[0]);
      break;
    case 21:
      // set RX AGC hang threshold
      if(dataInt[0] < 0 || dataInt[0] > 100) break;
      SetRXAAGCHangThreshold(0, dataInt[0]);
      break;
  }
}

//------------------------------------------------------------------------------

void Server::on_TimerTX_timeout()
{
  int32_t i, offset, position, error;
  float *bufferFloat;

  position = *(m_Sts + 2);
  if((m_LimitTX > 0 && position > m_LimitTX) || (m_LimitTX == 0 && position < 256))
  {
    offset = m_LimitTX > 0 ? 0 : 512;
    m_LimitTX = m_LimitTX > 0 ? 0 : 256;
    bufferFloat = 0;
    fexchange0(1, bufferFloat, m_BufferTX + offset, &error);
    m_InputOffsetTX += 256;
    if(m_InputOffsetTX == 4096)
    {
      m_InputOffsetTX = 0;
    }
  }
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_closed()
{
  qApp->quit();
}

//------------------------------------------------------------------------------

void Server::on_WebSocketServer_newConnection()
{
  QWebSocket *webSocket = m_WebSocketServer->nextPendingConnection();

  printf("new connection\n");
  if(m_WebSocket && webSocket)
  {
    printf("closing\n");
    webSocket->close();
    return;
  }

  connect(webSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(on_WebSocket_binaryMessageReceived(QByteArray)));
  connect(webSocket, SIGNAL(disconnected()), this, SLOT(on_WebSocket_disconnected()));

  m_WebSocket = webSocket;
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_disconnected()
{
  QWebSocket *webSocket = qobject_cast<QWebSocket *>(sender());

  if(!webSocket) return;

  printf("disconnected\n");

  if(m_WebSocket == webSocket) m_WebSocket = 0;

  webSocket->deleteLater();
}
