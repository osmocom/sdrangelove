#-------------------------------------------------
#
# Project created by QtCreator 2012-05-19T10:01:31
#
#-------------------------------------------------


!exists(local.pri) {
	error(Please create a local.pri)
}

include(local.pri)

QT       += core gui opengl

CONFIG += silent

UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sdrangelove 
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	hardware/samplefifo.cpp \
	dsp/dspengine.cpp \
	gui/indicator.cpp \
	dsp/fftwindow.cpp \
	settings.cpp \
	hardware/osmosdrinput.cpp \
	hardware/osmosdrthread.cpp \
	hardware/samplesource.cpp \
	gui/scale.cpp \
    dsp/nco.cpp \
    gui/glspectrum.cpp \
    gui/scaleengine.cpp

HEADERS  += mainwindow.h \
	hardware/samplefifo.h \
	dsp/dspengine.h \
	gui/indicator.h \
	dsp/kissfft.h \
	dsp/dsptypes.h \
	dsp/fftwindow.h \
	settings.h \
	hardware/osmosdrinput.h \
	hardware/osmosdrthread.h \
	hardware/samplesource.h \
	gui/scale.h \
	gui/physicalunit.h \
    dsp/nco.h \
    gui/glspectrum.h \
    gui/scaleengine.h

FORMS    += mainwindow.ui

unix {
	LIBS += -lrt -lGLU
}

# Portaudio - currently not in use
unix:portaudio {
	LIBS += -lasound
	LIBS += portaudio/libportaudio.a
	INCLUDEPATH += portaudio/portaudio/include
}

# libosmosdr
unix {
	LIBS += -lusb-1.0
	LIBS += $${LIBOSMOSDR_LIB_PATH}/libosmosdr.a
	INCLUDEPATH += $${LIBOSMOSDR_INCLUDE_PATH}
}
