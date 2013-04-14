project(gnuradio)

find_package(Gruel)
find_package(GnuradioCore)
find_package(GnuradioOsmosdr)

SET(BOOST_REQUIRED_COMPONENTS
    system
)

if(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64")
    list(APPEND BOOST_LIBRARYDIR "/usr/lib64") #fedora 64-bit fix
endif(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64")

set(Boost_ADDITIONAL_VERSIONS
    "1.35.0" "1.35" "1.36.0" "1.36" "1.37.0" "1.37" "1.38.0" "1.38" "1.39.0" "1.39"
    "1.40.0" "1.40" "1.41.0" "1.41" "1.42.0" "1.42" "1.43.0" "1.43" "1.44.0" "1.44"
    "1.45.0" "1.45" "1.46.0" "1.46" "1.47.0" "1.47" "1.48.0" "1.48" "1.49.0" "1.49"
    "1.50.0" "1.50" "1.51.0" "1.51" "1.52.0" "1.52" "1.53.0" "1.53" "1.54.0" "1.54"
    "1.55.0" "1.55" "1.56.0" "1.56" "1.57.0" "1.57" "1.58.0" "1.58" "1.59.0" "1.59"
    "1.60.0" "1.60" "1.61.0" "1.61" "1.62.0" "1.62" "1.63.0" "1.63" "1.64.0" "1.64"
    "1.65.0" "1.65" "1.66.0" "1.66" "1.67.0" "1.67" "1.68.0" "1.68" "1.69.0" "1.69"
)
find_package(Boost 1.35 COMPONENTS ${BOOST_REQUIRED_COMPONENTS})

if(Boost_FOUND AND GRUEL_FOUND AND GNURADIO_CORE_FOUND AND GNURADIO_OSMOSDR_FOUND)

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
add_definitions(-DBOOST_ALL_NO_LIB)

qt4_wrap_cpp(gnuradio_HEADERS_MOC ${gnuradio_HEADERS})
qt4_wrap_ui(gnuradio_FORMS_HEADERS ${gnuradio_FORMS})

add_library(inputgnuradio SHARED
	${gnuradio_SOURCES}
	${gnuradio_HEADERS_MOC}
	${gnuradio_FORMS_HEADERS}
)

target_link_libraries(inputgnuradio
	${QT_LIBRARIES}
	${Boost_LIBRARIES}
	${GRUEL_LIBRARIES}
	${GNURADIO_CORE_LIBRARIES}
	${GNURADIO_OSMOSDR_LIBRARIES}
	sdrbase
)

endif(Boost_FOUND AND GRUEL_FOUND AND GNURADIO_CORE_FOUND AND GNURADIO_OSMOSDR_FOUND)
