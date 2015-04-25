#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>

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
  m_BufferRX(0), m_BufferTX(0), m_BufferFFT(0),
  m_LimitRX(256), m_InputOffsetRX(0),
  m_LimitTX(0), m_InputOffsetTX(0),
  m_InputBufferRX(0), m_OutputBufferRX(0),
  m_CounterRX(0), m_PointerRX(0), m_StateRX(0), m_DataRX(0),
  m_TimerRX(0), m_TimerTX(0),
  m_WebSocketServer(0), m_WebSocket(0)
{
  int memFile;
  FILE *wisdomFile;
  int32_t i, *bufferFloatt, error;
  float *pointerFloat;

  if((memFile = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    qApp->quit();
  }

  m_Cfg = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40000000);
  m_Sts = (uint32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40001000);
  m_BufferRX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40002000);
  m_BufferTX = (int32_t *)mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40003000);
  m_BufferFFT = (int32_t *)mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, memFile, 0x40010000);

  /* enter reset mode */
  *(m_Cfg + 0) &= ~255;
  /* set default phase increment */
  *(m_Cfg + 1) = (uint32_t)floor(621000/125.0e6*(1<<30)+0.5);

  bufferFloatt = m_BufferTX;
  for(i = 0; i < 512; ++i) *(bufferFloatt++) = 0;

  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "r")))
  {
    fftwf_import_wisdom_from_file(wisdomFile);
    fclose(wisdomFile);
  }
  OpenChannel(0, 256, 4096, 20000, 20000, 20000, 0, 0, 0.010, 0.025, 0.000, 0.010, 0);
  OpenChannel(1, 256, 4096, 20000, 20000, 20000, 1, 0, 0.010, 0.025, 0.000, 0.010, 0);
  if((wisdomFile = fopen("wdsp-fftw-wisdom.txt", "w")))
  {
    fftwf_export_wisdom_to_file(wisdomFile);
    fclose(wisdomFile);
  }

  SetRXAAGCFixed(0, 40.0);
  SetRXAAGCTop(0, 40.0);

  m_InputBufferRX = new QByteArray();
  m_InputBufferRX->resize(1590 * sizeof(float));

  m_OutputBufferRX = new QByteArray();
  m_OutputBufferRX->resize(2048 * sizeof(int16_t) + 4);

  m_PointerRX = (int16_t *)(m_OutputBufferRX->constData() + 4);

  m_StateRX = src_new(SRC_SINC_FASTEST, 2, &error);
  m_DataRX = new SRC_DATA;

  pointerFloat = (float *)(m_InputBufferRX->constData());

  m_DataRX->data_in = pointerFloat + 512;
  m_DataRX->input_frames = 256;

  m_DataRX->data_out = pointerFloat + 1024;
  m_DataRX->output_frames = 283 ;
  m_DataRX->src_ratio = 22050.0/20000.0;
  m_DataRX->end_of_input = 0;

  m_TimerRX = new QTimer(this);
  connect(m_TimerRX, SIGNAL(timeout()), this, SLOT(on_TimerRX_timeout()));

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

void Server::on_WebSocket_textMessageReceived(QString message)
{
  int32_t i, *bufferFloatt;

  if(message.compare(QString("start RX")) == 0)
  {
    *(m_Cfg + 0) |= 3;
    SetChannelState(0, 1, 0);
    m_TimerRX->start(6);
  }
  else if(message.compare(QString("stop RX")) == 0)
  {
    m_TimerRX->stop();
  }
  if(message.compare(QString("start FFT")) == 0)
  {
  }
  else if(message.compare(QString("stop FFT")) == 0)
  {
  }
  if(message.compare(QString("start TX")) == 0)
  {
    m_TimerTX->start(6);
  }
  else if(message.compare(QString("stop TX")) == 0)
  {
    m_TimerTX->stop();
    bufferFloatt = m_BufferTX;
    for(i = 0; i < 512; ++i) *(bufferFloatt++) = 0;
  }
}

//------------------------------------------------------------------------------

void Server::on_TimerRX_timeout()
{
  int32_t i, offset, position, error;
  int32_t *pointerFloatt;
  float *bufferFloat, *pointerFloat;

  position = *(m_Sts + 0);
  if((m_LimitRX > 0 && position > m_LimitRX) || (m_LimitRX == 0 && position < 256))
  {
    offset = m_LimitRX > 0 ? 0 : 512;
    m_LimitRX += 256;
    if(m_LimitRX == 512) m_LimitRX = 0;
    pointerFloatt = m_BufferRX + offset;
    bufferFloat = (float *)(m_InputBufferRX->constData());
    pointerFloat = bufferFloat;
    for(i = 0; i < 512; ++i)
    {
      *(pointerFloat++) = ((float) *(pointerFloatt++)) / 2147483647.0;
    }
    fexchange0(0, bufferFloat, bufferFloat + 512, &error);
    src_process(m_StateRX, m_DataRX) ;
    pointerFloat = bufferFloat + 1024;
    for(i = 0; i < m_DataRX->output_frames_gen; ++i)
    {
      *(m_PointerRX++) = (int16_t)(*(pointerFloat++) * 32767.0);
      *(m_PointerRX++) = (int16_t)(*(pointerFloat++) * 32767.0);
      ++m_CounterRX;
      if(m_CounterRX == 1024)
      {
        m_CounterRX = 0;
        m_PointerRX = (int16_t *)(m_OutputBufferRX->constData() + 4);
        if(m_WebSocket) m_WebSocket->sendBinaryMessage(*m_OutputBufferRX);
      }
    }
  }
}

//------------------------------------------------------------------------------

void Server::on_WebSocket_binaryMessageReceived(QByteArray message)
{
  int32_t i, size;
  float *bufferReal, *bufferComplex, value;
/*
  size = txa[0].size;
  bufferReal = (float *)(message.constData());
  bufferComplex = txa[1].inbuff;

  for(i = 0; i < size; ++i)
  {
    value = *(bufferReal++);
    *(bufferComplex++) = value;
    *(bufferComplex++) = value;
  }
*/
}

//------------------------------------------------------------------------------

void Server::on_TimerTX_timeout()
{
  int32_t i, offset, position;
  int32_t *bufferFloatt;
  float *bufferFloat;

  position = *(m_Sts + 2);
  if((m_LimitTX > 0 && position > m_LimitTX) || (m_LimitTX == 0 && position < 256))
  {
    offset = m_LimitTX > 0 ? 0 : 512;
    m_LimitTX += 256;
    if(m_LimitTX == 512) m_LimitTX = 0;
    bufferFloatt = m_BufferTX + offset;
    bufferFloat = txa[1].outbuff + m_InputOffsetTX;
    for(i = 0; i < 256; ++i)
    {
      *(bufferFloatt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
      *(bufferFloatt++) = (int32_t)(*(bufferFloat++) * 2147483647.0);
    }
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

  connect(webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(on_WebSocket_textMessageReceived(QString)));
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
