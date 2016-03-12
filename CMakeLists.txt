cmake_minimum_required(VERSION 2.6)
project(pyemerald)

add_library(pyemerald MODULE pyemerald.c _pyemerald.c)

include(GNUInstallDirs)

set(ENGINE_DIR /${CMAKE_INSTALL_LIBDIR}/emerald/engines CACHE STRING "System-wide directory where Emerald looks for engines")
set(SCRIPT_SUBDIR pyEmerald CACHE STRING "Subdirectory inside ENGINE_DIR where our python parts will be located")
set(SCRIPT_DIR "" CACHE STRING "System-wide directory for our python parts (override)")
mark_as_advanced(SCRIPT_DIR)

find_package(PythonLibs)
find_package(PkgConfig)
pkg_search_module(EMERALD REQUIRED emeraldengine)
pkg_search_module(PYCAIRO REQUIRED py3cairo)
pkg_search_module(PYGOBJECT REQUIRED pygobject-3.0)
target_link_libraries(pyemerald ${PYTHON_LIBRARIES} ${EMERALD_LIBRARIES} ${PYCAIRO_LIBRARIES} ${PYGOBJECT_LIBRARIES})
include_directories(pyemerald ${PYTHON_INCLUDE_DIRS} ${EMERALD_INCLUDE_DIRS} ${PYCAIRO_INCLUDE_DIRS} ${PYGOBJECT_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})

option(LIBPY_LINK_HACK "Enable workaround for python modules that don't declare dependency on libpython" ON)
option(PYTHON_DICT_GC_HACK "Enable workaround for python circular GC clearing our dict while we're building it." ON)

if (LIBPY_LINK_HACK)
    set(LIBPY_SONAME ${PYTHON_LIBRARIES})
endif(LIBPY_LINK_HACK)

configure_file(config.h.in config.h)

option(LOCAL_INSTALL "Install in ~/.emerald instead of installing system-wide")

if (LOCAL_INSTALL)
    set(LOCAL_ENGINE_DIR $ENV{HOME}/.emerald/engines)
    install(TARGETS pyemerald DESTINATION ${LOCAL_ENGINE_DIR})
    install(FILES emerald.py DESTINATION ${LOCAL_ENGINE_DIR}/pyEmerald)
    add_custom_target(syminstall mkdir -p ${LOCAL_ENGINE_DIR}/pyEmerald
                         COMMAND ln -sf $<TARGET_FILE:pyemerald> ${LOCAL_ENGINE_DIR}
                         COMMAND ln -sf ${CMAKE_SOURCE_DIR}/emerald.py ${LOCAL_ENGINE_DIR}/pyEmerald VERBATIM)
else(LOCAL_INSTALL)
    install(TARGETS pyemerald DESTINATION ${ENGINE_DIR})
    install(FILES emerald.py DESTINATION ${ENGINE_DIR}/pyEmerald)
    add_custom_target(syminstall mkdir -p $ENV{HOME}/.emerald/engines/pyEmerald
                         COMMAND ln -sf $<TARGET_FILE:pyemerald> ${ENGINE_DIR}
                         COMMAND ln -sf ${CMAKE_SOURCE_DIR}/emerald.py ${ENGINE_DIR}/pyEmerald VERBATIM)
endif(LOCAL_INSTALL)

