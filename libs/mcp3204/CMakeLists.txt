file(GLOB mcp3204_SOURCES src/*.cpp)

add_library(mcp3204 STATIC ${mcp3204_SOURCES})

target_include_directories(
  mcp3204
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/mcp3204)

target_link_libraries(mcp3204 PUBLIC pico_stdlib hardware_spi hardware_dma)
