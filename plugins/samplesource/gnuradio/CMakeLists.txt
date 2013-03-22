project(gnuradio)

set(gnuradio_SOURCES
	gnuradiogui.cpp
	gnuradioinput.cpp
	gnuradioplugin.cpp
	gnuradiothread.cpp
)

set(gnuradio_HEADERS
	gnuradiogui.h
	gnuradioinput.h
	gnuradioplugin.h
	gnuradiothread.h
)

set(gnuradio_FORMS
	gnuradiogui.ui
)

include_directories(
	.
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/include-gpl
	${Boost_INCLUDE_DIRS}
	${GRUEL_INCLUDE_DIRS}
	${GNURADIO_CORE_INCLUDE_DIRS}
	${GNURADIO_OSMOSDR_INCLUDE_DIRS}
)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)

qt4_wrap_cpp(gnuradio_HEADERS_MOC ${gnuradio_HEADERS})
qt4_wrap_ui(gnuradio_FORMS_HEADERS ${gnuradio_FORMS})

add_library(inputgnuradio SHARED
	${gnuradio_SOURCES}
	${gnuradio_HEADERS_MOC}
	${gnuradio_FORMS_HEADERS}
)

target_link_libraries(inputgnuradio
	${QT_LIBRARIES}
	${LIBUSB_LIBRARIES}
	${Boost_LIBRARIES}
	${GRUEL_LIBRARIES}
	${GNURADIO_CORE_LIBRARIES}
	${GNURADIO_OSMOSDR_LIBRARIES}
)
