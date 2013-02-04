cmake_minimum_required(VERSION 2.6)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project(sdrangelove)

set(CMAKE_BUILD_TYPE "Release")

set(QT_USE_QTOPENGL TRUE)

find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig)
find_package(LibOsmoSDR REQUIRED)
find_package(Portaudio REQUIRED)
find_package(LibUSB REQUIRED)

set(sdrangelove_SOURCES
	main.cpp
	mainwindow.cpp
	miniz.cpp
	osdrupgrade.cpp
	preset.cpp
	settings.cpp

	dsp/channelizer.cpp
	dsp/dspcommands.cpp
	dsp/dspengine.cpp
	dsp/fftwindow.cpp
	dsp/interpolator.cpp
	dsp/lowpass.cpp
	dsp/nco.cpp
	dsp/nfmdemod.cpp
	dsp/pidcontroller.cpp
	dsp/samplesink.cpp
	dsp/spectrumvis.cpp

	gui/glspectrum.cpp
	gui/indicator.cpp
	gui/presetitem.cpp
	gui/scale.cpp
	gui/scaleengine.cpp
	gui/scope.cpp
	gui/scopewindow.cpp
	gui/valuedial.cpp

	hardware/audiofifo.cpp
	hardware/audiooutput.cpp
	hardware/osmosdrgui.cpp
	hardware/osmosdrinput.cpp
	hardware/osmosdrthread.cpp
	hardware/samplefifo.cpp
	hardware/samplesource.cpp
	hardware/samplesourcegui.cpp
	hardware/soundcardinfo.cpp

	util/message.cpp
	util/messagequeue.cpp
	util/spinlock.cpp
)

set(sdrangelove_HEADERS
	mainwindow.h
	osdrupgrade.h
	preset.h
	settings.h

	dsp/channelizer.h
	dsp/dspcommands.h
	dsp/dspengine.h
	dsp/dsptypes.h
	dsp/fftwindow.h
	dsp/interpolator.h
	dsp/kissfft.h
	dsp/lowpass.h
	dsp/nco.h
	dsp/nfmdemod.h
	dsp/pidcontroller.h
	dsp/samplesink.h
	dsp/spectrumvis.h

	gui/glspectrum.h
	gui/indicator.h
	gui/physicalunit.h
	gui/presetitem.h
	gui/scale.h
	gui/scaleengine.h
	gui/scope.h
	gui/scopewindow.h
	gui/valuedial.h

	hardware/audiofifo.h
	hardware/audiooutput.h
	hardware/osmosdrgui.h
	hardware/osmosdrinput.h
	hardware/osmosdrthread.h
	hardware/samplefifo.h
	hardware/samplesource.h
	hardware/samplesourcegui.h
	hardware/soundcardinfo.h

	util/message.h
	util/messagequeue.h
	util/spinlock.h
)

set(sdrangelove_FORMS
	mainwindow.ui
	osdrupgrade.ui

	gui/scopewindow.ui

	hardware/osmosdrgui.ui
)

set(sdrangelove_RESOURCES
	resources/res.qrc
)

include_directories(
	.
	portaudio/include
	${CMAKE_CURRENT_BINARY_DIR}
	${OPENGL_INCLUDE_DIR}
	${LIBOSMOSDR_INCLUDE_DIR}
	${PORTAUDIO_INCLUDE_DIRS}
	${LIBUSB_INCLUDE_DIR}
)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

qt4_wrap_cpp(sdrangelove_HEADERS_MOC ${sdrangelove_HEADERS})
qt4_wrap_ui(sdrangelove_FORMS_HEADERS ${sdrangelove_FORMS})
qt4_add_resources(sdrangelove_RESOURCES_RCC ${sdrangelove_RESOURCES})

if(WIN32)
	SET(sdrangelove_SOURCES ${sdrangelove_SOURCES} resources/sdrangelove.rc)
endif(WIN32)

add_executable(sdrangelove
	${sdrangelove_SOURCES}
	${sdrangelove_HEADERS_MOC}
	${sdrangelove_FORMS_HEADERS}
	${sdrangelove_RESOURCES_RCC}
)

target_link_libraries(sdrangelove
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
	${LIBOSMOSDR_LIBRARIES}
	${PORTAUDIO_LIBRARIES}
	${LIBUSB_LIBRARIES}
)

if(WIN32)
	set_target_properties(sdrangelove PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(sdrangelove PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
	set_target_properties(sdrangelove PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
	set_target_properties(sdrangelove PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
	set_target_properties(sdrangelove PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	set_target_properties(sdrangelove PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
endif(WIN32)
