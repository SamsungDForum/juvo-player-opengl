# Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(gles_sample C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

IF(NOT DEFINED MANIFESTDIR)
	SET(MANIFESTDIR "/opt/share/packages")
ENDIF(NOT DEFINED MANIFESTDIR)
IF(NOT DEFINED LIBDIR)
	SET(LIBDIR "${PREFIX}/lib")
ENDIF(NOT DEFINED LIBDIR)

SET(SRCS
  src/main.cpp
  src/Menu.cpp
  src/Animation.cpp
  src/Background.cpp
  src/Loader.cpp
  src/Playback.cpp
  src/Text.cpp
  src/Tile.cpp
  src/TileAnimation.cpp
  src/Subtitles.cpp
)

INCLUDE(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED
  gles20
  egl
  dlog)

FOREACH(flag ${PKGS_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} ")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fpermissive")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++17 -Iinclude/freetype-2.9/include -Iinclude")

ADD_LIBRARY (${PROJECT_NAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PKGS_LDFLAGS})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR})
