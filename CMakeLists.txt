cmake_minimum_required(VERSION 3.31.6)

project(libibackup C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PLIST REQUIRED IMPORTED_TARGET libplist-2.0)
pkg_check_modules(LIMD_GLUE REQUIRED IMPORTED_TARGET libimobiledevice-glue-1.0)

add_library(libibackup src/libibackup.c)

target_link_libraries(libibackup PRIVATE PkgConfig::PLIST PkgConfig::LIMD_GLUE)
target_include_directories(libibackup PRIVATE ${PLIST_INCLUDE_DIR} ${LIMD_GLUE_INCLUDE_DIR})
target_sources(libibackup PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include # The base directory to preserve structure from
        FILES include/libibackup/libibackup.h
)

install(TARGETS libibackup
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})