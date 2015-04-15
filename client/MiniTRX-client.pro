TARGET = MiniTRX-client
TEMPLATE = app
QT += uitools multimedia websockets
CONFIG += static
QMAKE_LFLAGS += -static
RESOURCES = client.qrc
HEADERS = client.h indicator.h plotter.h
SOURCES = client.cpp indicator.cpp plotter.cpp main.cpp
