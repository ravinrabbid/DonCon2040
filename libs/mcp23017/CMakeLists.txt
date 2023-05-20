file(GLOB mcp23017_SOURCES src/*.cpp)

add_library(mcp23017 STATIC ${mcp23017_SOURCES})

target_include_directories(
  mcp23017
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/mcp23017)

target_link_libraries(mcp23017 PUBLIC pico_stdlib hardware_i2c)
