cmake_minimum_required(VERSION 3.2)
PROJECT (tipcalc)

FILE(GLOB SOURCE
  "src/*.h"
  "src/*.c"
  "src/js/*.js"
)

FILE(GLOB HEADERS
  "/usr/local/Cellar/pebble-sdk/3.7/Pebble/chalk/include/*.h"
)

INCLUDE_DIRECTORIES(/usr/local/Cellar/pebble-sdk/3.7/Pebble/chalk/include)
ADD_EXECUTABLE(tipcalc ${SOURCE} ${HEADERS})
