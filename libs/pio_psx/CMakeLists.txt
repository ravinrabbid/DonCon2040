file(GLOB pio_psx_SOURCES src/*.c)

add_library(pio_psx STATIC ${pio_psx_SOURCES})

pico_generate_pio_header(pio_psx ${CMAKE_CURRENT_LIST_DIR}/src/psx_spi.pio
                         OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_include_directories(
  pio_psx
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/pio_psx
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(pio_psx PUBLIC pico_stdlib hardware_pio)

enable_clang_tidy(pio_psx)
