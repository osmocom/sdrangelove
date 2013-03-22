project(samplesource)

find_package(LibOsmoSDR REQUIRED)
find_package(LibRTLSDR REQUIRED)

add_subdirectory(osmosdr)
add_subdirectory(rtlsdr)
