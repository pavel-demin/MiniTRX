//////////////////////////////////////////////////////////////////////
// plotter.cpp: implementation of the CPlotter class.
//
//  This class creates and draws a combination spectral view using
// a 2D and waterfall display and manages mouse events within the plot area
//
// History:
//  2010-09-15  Initial creation MSW
//  2011-03-27  Initial release
//  2012-02-11  Fixed compiler warning
//////////////////////////////////////////////////////////////////////

//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
//Copyright 2010 Moe Wheatley. All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Moe Wheatley ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Moe Wheatley OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Moe Wheatley.
//==========================================================================================
#include "plotter.h"
#include <stdlib.h>
#include <QDebug>
#include <QtWidgets/QToolTip>

//////////////////////////////////////////////////////////////////////
// Local defines
//////////////////////////////////////////////////////////////////////
#define MAX_SCREENSIZE 4096
#define CUR_CUT_DELTA 10    //cursor capture delta in pixels
#define OVERLOAD_DISPLAY_LIMIT 3

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPlotter::CPlotter(QWidget *parent) :
  QFrame(parent)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_PaintOnScreen,false);
  setAutoFillBackground(false);
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setMouseTracking(true);

  //create a default waterfall color scheme
  // *** Need to read from file ***
  for(int i=0; i<256; i++)
  {
    if((i<43))
      m_ColorTbl[i].setRgb(0,0, 255*(i)/43);
    if((i>=43) && (i<87))
      m_ColorTbl[i].setRgb(0, 255*(i-43)/43, 255);
    if((i>=87) && (i<120))
      m_ColorTbl[i].setRgb(0,255, 255-(255*(i-87)/32));
    if((i>=120) && (i<154))
      m_ColorTbl[i].setRgb((255*(i-120)/33), 255, 0);
    if((i>=154) && (i<217))
      m_ColorTbl[i].setRgb(255, 255 - (255*(i-154)/62), 0);
    if((i>=217) )
      m_ColorTbl[i].setRgb(255, 0, 128*(i-217)/38);
  }

  m_UseCursorText = true;

  m_CenterFreq = 600000;
  m_DemodCenterFreq = 600000;
  m_DemodHiCutFreq = 4600;
  m_DemodLowCutFreq = -4600;

  m_FLowCmin = -4600;
  m_FLowCmax = -1000;
  m_FHiCmin = 1000;
  m_FHiCmax = 4600;
  m_symetric = true;

  m_ClickResolution = 100;
  m_FilterClickResolution = 200;
  m_CursorCaptureDelta = CUR_CUT_DELTA;

  m_Span = 100000;
  m_MaxdB = 0;
  m_dBStepSize = 10;
  m_FreqUnits = 1000;
  m_CursorCaptured = NONE;
  m_Running = false;
  m_ADOverloadOneShotCounter = 0;
  m_ADOverLoad = false;
  m_2DPixmap = QPixmap(0,0);
  m_OverlayPixmap = QPixmap(0,0);
  m_WaterfallPixmap = QPixmap(0,0);
  m_Size = QSize(0,0);
  m_GrabPosition = 0;
  m_Percent2DScreen = 70; //percent of screen used for 2D display
  m_RdsCall[0] = 0;
  m_RdsText[0] = 0;

}

CPlotter::~CPlotter()
{
}

//////////////////////////////////////////////////////////////////////
// Sizing interface
//////////////////////////////////////////////////////////////////////
QSize CPlotter::minimumSizeHint() const
{
  return QSize(50, 50);
}

QSize CPlotter::sizeHint() const
{
  return QSize(180, 180);
}

//////////////////////////////////////////////////////////////////////
// Called when mouse moves and does different things depending
//on the state of the mouse buttons for dragging the demod bar or
// filter edges.
//////////////////////////////////////////////////////////////////////
void CPlotter::mouseMoveEvent(QMouseEvent *event)
{
  QPoint pt = event->pos();
  QPoint gpt = event->globalPos();
  if(m_OverlayPixmap.rect().contains(pt))
  { //is in Overlay bitmap region
    if(event->buttons()==Qt::NoButton)
    { //if no mouse button monitor grab regions and change cursor icon
      if(IsPointCloseTo(pt.x(),(m_DemodHiCutFreqX+m_DemodLowCutFreqX)/2,
        (m_DemodHiCutFreqX-m_DemodLowCutFreqX/*-m_CursorCaptureDelta*/)/2))
      { //in move demod box center frequency region
        if(CENTER!=m_CursorCaptured)
          setCursor(QCursor(Qt::CrossCursor));
        m_CursorCaptured = CENTER;
        DisplayCursorFreq(gpt, RoundFreq(FreqfromX(pt.x()), m_ClickResolution));
      }
      else if(IsPointCloseTo(pt.x(),m_DemodHiCutFreqX, m_CursorCaptureDelta))
      { //in move demod hicut region
        if(RIGHT!=m_CursorCaptured)
          setCursor(QCursor(Qt::SizeFDiagCursor));
        m_CursorCaptured = RIGHT;
        DisplayCursorFreq(gpt, m_DemodHiCutFreq);
      }
      else if(IsPointCloseTo(pt.x(),m_DemodLowCutFreqX, m_CursorCaptureDelta))
      { //in move demod lowcut region
        if(LEFT!=m_CursorCaptured)
          setCursor(QCursor(Qt::SizeBDiagCursor));
        m_CursorCaptured = LEFT;
        DisplayCursorFreq(gpt, m_DemodLowCutFreq);
      }
      else
      { //if not near any grab boundaries
        if(NONE!=m_CursorCaptured)
        {
          setCursor(QCursor(Qt::ArrowCursor));
          m_CursorCaptured = NONE;
        }
        DisplayCursorFreq(gpt, RoundFreq(FreqfromX(pt.x()), m_ClickResolution));
      }
      m_GrabPosition = 0;
    }
  }
  else
  { //not in Overlay region
    if(event->buttons()==Qt::NoButton)
    {
      if(NONE!=m_CursorCaptured)
        setCursor(QCursor(Qt::ArrowCursor));
      m_CursorCaptured = NONE;
      m_GrabPosition = 0;
      DisplayCursorFreq(gpt, RoundFreq(FreqfromX(pt.x()), m_ClickResolution));
    }
  }
  //process mouse moves while in cursor capture modes
  if(LEFT==m_CursorCaptured)
  { //moving in demod lowcut region
    if(event->buttons()&Qt::RightButton)
    { //moving in demod lowcut region with right button held
      if(m_GrabPosition!=0)
      {
        m_DemodLowCutFreq = FreqfromX(pt.x()-m_GrabPosition) - m_DemodCenterFreq;
        m_DemodLowCutFreq = RoundFreq(m_DemodLowCutFreq, m_FilterClickResolution);
        DrawOverlay();
        if(m_symetric)
        {
          m_DemodHiCutFreq = -m_DemodLowCutFreq;
          emit NewHighCutFreq(m_DemodHiCutFreq);
        }
        emit NewLowCutFreq(m_DemodLowCutFreq);
      }
      else
      { //save initial grab postion from m_DemodFreqX
        m_GrabPosition = pt.x()-m_DemodLowCutFreqX;
      }
      DisplayCursorFreq(gpt, m_DemodLowCutFreq);
    }
    else if(event->buttons() & ~Qt::NoButton)
    {
      setCursor(QCursor(Qt::ArrowCursor));
      m_CursorCaptured = NONE;
    }
  }
  else if(RIGHT==m_CursorCaptured)
  { //moving in demod highcut region
    if(event->buttons()&Qt::RightButton)
    { //moving in demod highcut region with right button held
      if(m_GrabPosition!=0)
      {
        m_DemodHiCutFreq = FreqfromX(pt.x()-m_GrabPosition) - m_DemodCenterFreq;
        m_DemodHiCutFreq = RoundFreq(m_DemodHiCutFreq, m_FilterClickResolution);
        DrawOverlay();
        if(m_symetric)
        {
          m_DemodLowCutFreq = -m_DemodHiCutFreq;
          emit NewLowCutFreq(m_DemodLowCutFreq);
        }
        emit NewHighCutFreq(m_DemodHiCutFreq);
      }
      else
      { //save initial grab postion from m_DemodFreqX
        m_GrabPosition = pt.x()-m_DemodHiCutFreqX;
      }
      DisplayCursorFreq(gpt, m_DemodHiCutFreq);
    }
    else if(event->buttons() & ~Qt::NoButton)
    {
      setCursor(QCursor(Qt::ArrowCursor));
      m_CursorCaptured = NONE;
    }
  }
  else if(CENTER==m_CursorCaptured)
  { //moving inbetween demod lowcut and highcut region
    if(event->buttons()&Qt::LeftButton)
    {//moving inbetween demod lowcut and highcut region with left button held
      if(m_GrabPosition!=0)
      {
        m_DemodCenterFreq = RoundFreq(FreqfromX(pt.x()-m_GrabPosition),m_ClickResolution);
        emit NewDemodFreq(m_DemodCenterFreq);
      }
      else
      { //save initial grab postion from m_DemodFreqX
        m_GrabPosition = pt.x()-m_DemodFreqX;
      }
      DisplayCursorFreq(gpt, m_DemodCenterFreq);
    }
    else if(event->buttons() & ~Qt::NoButton)
    {
      setCursor(QCursor(Qt::ArrowCursor));
      m_CursorCaptured = NONE;
    }
  }
  else  //if cursor not captured
  {
    m_GrabPosition = 0;
  }
  if(!this->rect().contains(pt))
  {
    if(NONE != m_CursorCaptured)
      setCursor(QCursor(Qt::ArrowCursor));
    m_CursorCaptured = NONE;
    QToolTip::hideText();
  }
}

//////////////////////////////////////////////////////////////////////
// Called when a mouse button is pressed
//////////////////////////////////////////////////////////////////////
void CPlotter::mousePressEvent(QMouseEvent *event)
{
  QPoint pt = event->pos();
  QPoint gpt = event->globalPos();
  if(event->buttons()==Qt::LeftButton)
  {
    if(IsPointCloseTo(pt.x(),(m_DemodHiCutFreqX+m_DemodLowCutFreqX)/2,
      (m_DemodHiCutFreqX-m_DemodLowCutFreqX/*-m_CursorCaptureDelta*/)/2))
    { //in move demod box center frequency region
      if(CENTER!=m_CursorCaptured)
        setCursor(QCursor(Qt::CrossCursor));
      m_CursorCaptured = CENTER;
      m_GrabPosition = pt.x()-m_DemodFreqX;
    }
    if(CENTER!=m_CursorCaptured)
    { //if cursor not captured set demod frequency and start demod box capture
      m_DemodCenterFreq = RoundFreq(FreqfromX(pt.x()),m_ClickResolution);
      emit NewDemodFreq(m_DemodCenterFreq);
      //save initial grab postion from m_DemodFreqX
      setCursor(QCursor(Qt::CrossCursor));
      m_CursorCaptured = CENTER;
      m_GrabPosition = 1;
    }
    DisplayCursorFreq(gpt, m_DemodCenterFreq);
  }
  else if(event->buttons()==Qt::MiddleButton)
  {
    if(NONE==m_CursorCaptured)
    { //if cursor not captured set center freq
      m_CenterFreq = RoundFreq(FreqfromX(pt.x()),m_ClickResolution);
      m_DemodCenterFreq = m_CenterFreq;
      emit NewCenterFreq(m_CenterFreq);
      DisplayCursorFreq(gpt, m_CenterFreq);
    }
  }
  else if(event->buttons()==Qt::RightButton)
  {
    if(NONE==m_CursorCaptured)
    {
      DisplayCursorFreq(gpt, FreqfromX(pt.x()));
    }
    else if(IsPointCloseTo(pt.x(),m_DemodHiCutFreqX, m_CursorCaptureDelta))
    { //in move demod hicut region
      DisplayCursorFreq(gpt, m_DemodHiCutFreq);
    }
    else if(IsPointCloseTo(pt.x(),m_DemodLowCutFreqX, m_CursorCaptureDelta))
    { //in move demod lowcut region
      DisplayCursorFreq(gpt, m_DemodLowCutFreq);
    }
  }
}

//////////////////////////////////////////////////////////////////////
// Called when a mouse button is released
//////////////////////////////////////////////////////////////////////
void CPlotter::mouseReleaseEvent(QMouseEvent *event)
{
  QPoint pt = event->pos();
  QPoint gpt = event->globalPos();
  if(!m_OverlayPixmap.rect().contains(pt))
  { //not in Overlay region
    if(NONE!=m_CursorCaptured)
      setCursor(QCursor(Qt::ArrowCursor));
    m_CursorCaptured = NONE;
    m_GrabPosition = 0;
    DisplayCursorFreq(gpt, m_DemodCenterFreq);
  }
  else
  {
    if(IsPointCloseTo(pt.x(),m_DemodHiCutFreqX, m_CursorCaptureDelta))
    { //in move demod hicut region
      DisplayCursorFreq(gpt, m_DemodHiCutFreq);
    }
    else if(IsPointCloseTo(pt.x(),m_DemodLowCutFreqX, m_CursorCaptureDelta))
    { //in move demod lowcut region
      DisplayCursorFreq(gpt, m_DemodLowCutFreq);
    }
  }

}

//////////////////////////////////////////////////////////////////////
// Called when a mouse wheel is turned
//////////////////////////////////////////////////////////////////////
void CPlotter::wheelEvent(QWheelEvent *event)
{
  QPoint gpt = event->globalPos();
  int numDegrees = event->delta() / 8;
  int numSteps = numDegrees / 15;
  if(event->buttons()==Qt::RightButton)
  { //right button held while wheel is spun
    if(RIGHT==m_CursorCaptured)
    { //change demod high cut
      m_DemodHiCutFreq  += (numSteps*m_FilterClickResolution);
      m_DemodHiCutFreq = RoundFreq(m_DemodHiCutFreq, m_FilterClickResolution);
      DrawOverlay();
      if(m_symetric)
      {
        m_DemodLowCutFreq = -m_DemodHiCutFreq;
        emit NewLowCutFreq(m_DemodLowCutFreq);
      }
      emit NewHighCutFreq(m_DemodHiCutFreq);
      DisplayCursorFreq(gpt, m_DemodHiCutFreq);
    }
    else if(LEFT==m_CursorCaptured)
    { //change demod low cut
      m_DemodLowCutFreq  += (numSteps*m_FilterClickResolution);
      m_DemodLowCutFreq = RoundFreq(m_DemodLowCutFreq, m_FilterClickResolution);
      DrawOverlay();
      if(m_symetric)
      {
        m_DemodHiCutFreq = -m_DemodLowCutFreq;
        emit NewHighCutFreq(m_DemodHiCutFreq);
      }
      emit NewLowCutFreq(m_DemodLowCutFreq);
      DisplayCursorFreq(gpt, m_DemodLowCutFreq);
    }
  }
  else
  { //inc/dec demod frequency if right button NOT pressed
    m_DemodCenterFreq += (numSteps*m_ClickResolution);
    m_DemodCenterFreq = RoundFreq(m_DemodCenterFreq, m_ClickResolution);
    emit NewDemodFreq(m_DemodCenterFreq);
    DisplayCursorFreq(gpt, m_DemodCenterFreq);
  }
}

//////////////////////////////////////////////////////////////////////
// Called when screen size changes so must recalculate bitmaps
//////////////////////////////////////////////////////////////////////
void CPlotter::resizeEvent(QResizeEvent*)
{
  if(!size().isValid())
    return;
  if(m_Size != size())
  { //if changed, resize pixmaps to new screensize
    m_Size = size();
    m_OverlayPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
    m_OverlayPixmap.fill(Qt::black);
    m_2DPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
    m_2DPixmap.fill(Qt::black);
    m_WaterfallPixmap = QPixmap(m_Size.width(), (100-m_Percent2DScreen)*m_Size.height()/100);
  }
  m_WaterfallPixmap.fill(Qt::black);
  DrawOverlay();
}

//////////////////////////////////////////////////////////////////////
// Called by QT when screen needs to be redrawn
//////////////////////////////////////////////////////////////////////
void CPlotter::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.drawPixmap(0,0,m_2DPixmap);
  painter.drawPixmap(0, m_Percent2DScreen*m_Size.height()/100,m_WaterfallPixmap);
  return;
}


//////////////////////////////////////////////////////////////////////
// Called to update spectrum data for displaying on the screen
//////////////////////////////////////////////////////////////////////
void CPlotter::draw()
{
  int i;
  int w;
  int h;
  int fftbuf[MAX_SCREENSIZE];
  QPoint LineBuf[MAX_SCREENSIZE];

  if(!m_Running)
    return;

  //get/draw the waterfall
  w = m_WaterfallPixmap.width();
  h = m_WaterfallPixmap.height();

  //move current data down one line(must do before attaching a QPainter object)
  m_WaterfallPixmap.scroll(0,1,0,0, w, h);

  QPainter painter1(&m_WaterfallPixmap);
  //get scaled FFT data
  bool fftoverload = false;

  //draw new line of fft data at top of waterfall bitmap
  for(i=0; i<w; i++)
  {
    painter1.setPen(m_ColorTbl[ 255-fftbuf[i] ]);
    painter1.drawPoint(i,0);
  }

  //get/draw the 2D spectrum
  w = m_2DPixmap.width();
  h = m_2DPixmap.height();
  //first copy into 2Dbitmap the overlay bitmap.
  m_2DPixmap = m_OverlayPixmap.copy(0,0,w,h);

  QPainter painter2(&m_2DPixmap);
  // workaround for "fixed" line drawing since Qt 5
  // see http://stackoverflow.com/questions/16990326
#if QT_VERSION >= 0x050000
  painter2.translate(0.5, 0.5);
#endif

  //draw the 2D spectrum
  if(m_ADOverLoad || fftoverload)
  {
    painter2.setPen(Qt::red);
    if(m_ADOverloadOneShotCounter++ > OVERLOAD_DISPLAY_LIMIT)
    {
      m_ADOverloadOneShotCounter = 0;
      m_ADOverLoad = false;
    }
  }
  else
    painter2.setPen(Qt::green);
  for(i=0; i<w; i++)
  {
    LineBuf[i].setX(i);
    LineBuf[i].setY(fftbuf[i]);
  }
  painter2.drawPolyline(LineBuf,w);

  //trigger a new paintEvent
  update();
}

//////////////////////////////////////////////////////////////////////
// Called to draw an overlay bitmap containing grid and text that
// does not need to be recreated every fft data update.
//////////////////////////////////////////////////////////////////////
void CPlotter::DrawOverlay()
{
  if(m_OverlayPixmap.isNull())
    return;
  int w = m_OverlayPixmap.width();
  int h = m_OverlayPixmap.height();
  int x,y;
  float pixperdiv;
  QRect rect;
  QPainter painter(&m_OverlayPixmap);
  painter.initFrom(this);

  //m_OverlayPixmap.fill(Qt::black);
  //fill background with gradient
  QLinearGradient gradient(0, 0, 0 ,h);
  gradient.setColorAt(1, Qt::black);
//  gradient.setColorAt(0, Qt::gray);
  gradient.setColorAt(0, Qt::darkBlue);
  painter.setBrush(gradient);
  painter.drawRect(0, 0, w, h);


  //Draw demod filter box
  ClampDemodParameters();
  m_DemodFreqX = XfromFreq(m_DemodCenterFreq);
  m_DemodLowCutFreqX = XfromFreq(m_DemodCenterFreq + m_DemodLowCutFreq);
  m_DemodHiCutFreqX = XfromFreq(m_DemodCenterFreq + m_DemodHiCutFreq);

  int dw = m_DemodHiCutFreqX - m_DemodLowCutFreqX;

  painter.setBrush(Qt::SolidPattern);
  painter.setOpacity(0.5);

  int LockState = false;
  if(LockState)
    painter.fillRect(m_DemodLowCutFreqX, 0,dw, h, Qt::green);
  else
    painter.fillRect(m_DemodLowCutFreqX, 0,dw, h, Qt::gray);

  painter.setPen(QPen(Qt::magenta, 2,Qt::DotLine));
  painter.drawLine(m_DemodLowCutFreqX, 0, m_DemodLowCutFreqX, h);
  painter.drawLine(m_DemodHiCutFreqX, 0, m_DemodHiCutFreqX, h);

  painter.setPen(QPen(Qt::blue, 1,Qt::DashLine));
  painter.drawLine(m_DemodFreqX, 0, m_DemodFreqX, h);
  painter.setOpacity(1);

  //create Font to use for scales
  QFont Font("Arial");
  QFontMetrics metrics(Font);
  y = h/VERT_DIVS;
  int vskip;
  if(y>15)
  {
    vskip = 1;
    Font.setPointSize(10);
  }
  else if(y>8)
  {
    vskip = 2;
    Font.setPixelSize(y);
  }
  else
  {
    vskip = 20;
    Font.setPointSize(6);
  }
  painter.setFont(Font);
  //draw vertical grids
  pixperdiv = (float)w / (float)HORZ_DIVS;
  y = h - h/VERT_DIVS;
  for(int i=1; i<HORZ_DIVS; i++)
  {
    x = (int)((float)i*pixperdiv);
    if(i==HORZ_DIVS/2)
      painter.setPen(QPen(Qt::red, 1,Qt::DotLine));
    else
      painter.setPen(QPen(Qt::white, 1,Qt::DotLine));
    painter.drawLine(x, 0, x , y);
    painter.drawLine(x, h-5, x , h);
  }

  //draw frequency values
  MakeFrequencyStrs();
  painter.setPen(Qt::white);
  y = h - (h/VERT_DIVS);
  int hskip;
  if(pixperdiv>90.)
    hskip=1;
  else if(pixperdiv>65.)
    hskip = 2;
  else
    hskip = 3;
  for(int i=0; i<=HORZ_DIVS; i++)
  {
    if(0==i)
    {
      if(hskip==1)
      { //left justify the leftmost text
        x = (int)((float)i*pixperdiv);
        rect.setRect(x ,y, (int)pixperdiv, h/VERT_DIVS);
        painter.drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, m_HDivText[i]);
      }
    }
    else if(HORZ_DIVS == i)
    {
      if(hskip==1)
      { //right justify the rightmost text
        x = (int)((float)i*pixperdiv - pixperdiv);
        rect.setRect(x ,y, (int)pixperdiv, h/VERT_DIVS);
        painter.drawText(rect, Qt::AlignRight|Qt::AlignVCenter, m_HDivText[i]);
      }
    }
    else
    {
      if((hskip<=2) || (i&1))
      {
        //center justify the rest of the text
        x = (int)((float)i*pixperdiv - pixperdiv/2);
        rect.setRect(x ,y, (int)pixperdiv, h/VERT_DIVS);
        painter.drawText(rect, Qt::AlignHCenter|Qt::AlignVCenter, m_HDivText[i]);
      }
    }
  }


  //draw horizontal grids
  pixperdiv = (float)h / (float)VERT_DIVS;
  painter.setPen(QPen(Qt::white, 1,Qt::DotLine));
  for(int i=1; i<VERT_DIVS; i++)
  {
    y = (int)((float)i*pixperdiv);
    painter.drawLine(0, y, w, y);
  }

  //draw amplitude values
  painter.setPen(Qt::yellow);
  Font.setWeight(QFont::Light);
  painter.setFont(Font);
  int dB = m_MaxdB;
  for(int i=0; i<VERT_DIVS-1; i+=vskip)
  {
    y = (int)((float)i*pixperdiv);
    painter.drawStaticText(5, y-1, QString::number(dB)+" dB");
    dB -= (m_dBStepSize*vskip);
  }

  if(0 != m_RdsCall[0])
  {
    painter.setPen(Qt::yellow);
    Font.setPointSize(20);
    painter.setFont(Font);
    rect.setTop(0);
    rect.setLeft(0);
    rect.setRight(w);
    rect.setHeight(m_OverlayPixmap.height());
    painter.drawText(rect,Qt::AlignHCenter|Qt::AlignTop, m_RdsCall);
    if(0 != m_RdsText[0])
    {
      QFontMetrics metrics2(Font);
      //
      painter.setPen(Qt::white);
      Font.setPointSize(12);
      painter.setFont(Font);
      rect.setLeft(0);
      rect.setRight(w);
      rect.setTop(metrics2.height());
      painter.drawText(rect,Qt::AlignHCenter|Qt::AlignTop, m_RdsText);
    }
  }

  if(!m_Running)
  { //if not running so is no data updates to draw to screen
    //copy into 2Dbitmap the overlay bitmap.
    m_2DPixmap = m_OverlayPixmap.copy(0,0,w,h);
    //trigger a new paintEvent
    update();
  }
}

//////////////////////////////////////////////////////////////////////
// Helper function Called to create all the frequency division text
//strings based on start frequency, span frequency, frequency units.
//Places in QString array m_HDivText
//Keeps all strings the same fractional length
//////////////////////////////////////////////////////////////////////
void CPlotter::MakeFrequencyStrs()
{
  int FreqPerDiv = m_Span/HORZ_DIVS;
  int StartFreq = m_CenterFreq - m_Span/2;
  float freq;
  int i,j;
  int numfractdigits = (int)log10((float)m_FreqUnits);
  if(1 == m_FreqUnits)
  { //if units is Hz then just output integer freq
    for(int i=0; i<=HORZ_DIVS; i++)
    {
      freq = (float)StartFreq/(float)m_FreqUnits;
      m_HDivText[i].setNum((int)freq);
      StartFreq += FreqPerDiv;
    }
    return;
  }
  //here if is fractional frequency values
  //so create max sized text based on frequency units
  for(int i=0; i<=HORZ_DIVS; i++)
  {
    freq = (float)StartFreq/(float)m_FreqUnits;
    m_HDivText[i].setNum(freq,'f', numfractdigits);
    StartFreq += FreqPerDiv;
  }
  //now find the division text with the longest non-zero digit
  //to the right of the decimal point.
  int max = 0;
  for(i=0; i<=HORZ_DIVS; i++)
  {
    int dp = m_HDivText[i].indexOf('.');
    int l = m_HDivText[i].length()-1;
    for(j=l; j>dp; j--)
    {
      if(m_HDivText[i][j] != '0')
        break;
    }
    if((j-dp) > max)
      max = j-dp;
  }
  //truncate all strings to maximum fractional length
  StartFreq = m_CenterFreq - m_Span/2;
  for(i=0; i<=HORZ_DIVS; i++)
  {
    freq = (float)StartFreq/(float)m_FreqUnits;
    m_HDivText[i].setNum(freq,'f', max);
    StartFreq += FreqPerDiv;
  }
}

//////////////////////////////////////////////////////////////////////
// Helper functions to convert to/from screen coordinates to frequency
//////////////////////////////////////////////////////////////////////
int CPlotter::XfromFreq(int freq)
{
float w = m_OverlayPixmap.width();
float StartFreq = (float)m_CenterFreq - (float)m_Span/2.;
int x = (int) w * ((float)freq - StartFreq)/(float)m_Span;
  if(x<0)
    return 0;
  if(x>(int)w)
    return m_OverlayPixmap.width();
  return x;
}

int CPlotter::FreqfromX(int x)
{
  float w = m_OverlayPixmap.width();
  float StartFreq = (float)m_CenterFreq - (float)m_Span/2.;
  int f = (int)(StartFreq + (float)m_Span * (float)x/(float)w);
  return f;
}

//////////////////////////////////////////////////////////////////////
// Helper function to round frequency to click resolution value
//////////////////////////////////////////////////////////////////////
int CPlotter::RoundFreq(int freq, int resolution)
{
int delta = resolution;
int delta_2 = delta/2;
  if(freq>=0)
    return (freq - (freq+delta_2)%delta + delta_2);
  else
    return (freq - (freq+delta_2)%delta - delta_2);
}

//////////////////////////////////////////////////////////////////////
// Helper function called to display frequency text with cursor tooltip
//////////////////////////////////////////////////////////////////////
void CPlotter::DisplayCursorFreq(QPoint pt, int freq)
{
  if(!m_UseCursorText)
    return;
  if(freq<100000)
    m_Str = QString::number(freq) + " Hz";
  else if(freq<1000000)
    m_Str = QString::number((float)freq/1000.0) + " kHz";
  else
    m_Str = QString::number((float)freq/1000000.0,'g', 8) + " MHz";
  QToolTip::showText(pt, m_Str, this, rect());
}

//////////////////////////////////////////////////////////////////////
// Helper function clamps demod freqeuency limits of
// m_DemodCenterFreq
//////////////////////////////////////////////////////////////////////
void CPlotter::ClampDemodParameters()
{

  if(m_DemodLowCutFreq < m_FLowCmin)
    m_DemodLowCutFreq = m_FLowCmin;
  if(m_DemodLowCutFreq > m_FLowCmax)
    m_DemodLowCutFreq = m_FLowCmax;

  if(m_DemodHiCutFreq < m_FHiCmin)
    m_DemodHiCutFreq = m_FHiCmin;
  if(m_DemodHiCutFreq > m_FHiCmax)
    m_DemodHiCutFreq = m_FHiCmax;

}

void CPlotter::SetDemodRanges(int FLowCmin, int FLowCmax, int FHiCmin, int FHiCmax, bool symetric)
{
  m_FLowCmin=FLowCmin;
  m_FLowCmax=FLowCmax;
  m_FHiCmin=FHiCmin;
  m_FHiCmax=FHiCmax;
  m_symetric=symetric;
  ClampDemodParameters();
}

