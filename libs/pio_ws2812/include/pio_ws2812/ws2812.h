#ifndef PIO_WS2812_WS2812_H_
#define PIO_WS2812_WS2812_H_

#include "hardware/pio.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

void ws2812_init(PIO pio, uint8_t pin, bool is_rgbw);

uint32_t ws2812_rgb_to_u32pixel(uint8_t r, uint8_t g, uint8_t b);
uint32_t ws2812_rgb_to_gamma_corrected_u32pixel(uint8_t r, uint8_t g, uint8_t b);

void ws2812_put_pixel(PIO pio, uint32_t pixel_grb);
void ws2812_put_frame(PIO pio, uint32_t *frame, size_t length);

#ifdef __cplusplus
}
#endif

#endif // PIO_WS2812_WS2812_H_