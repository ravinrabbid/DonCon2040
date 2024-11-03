#ifndef _USB_DEVICE_MIDI_DRIVER_H_
#define _USB_DEVICE_MIDI_DRIVER_H_

#include "usb/device_driver.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute((packed, aligned(1))) {
    struct {
        bool acoustic_bass_drum;
        bool electric_bass_drum;
        bool drumsticks;
        bool side_stick;
    } status;

    struct {
        uint8_t acoustic_bass_drum;
        uint8_t electric_bass_drum;
        uint8_t drumsticks;
        uint8_t side_stick;
    } velocity;
} midi_report_t;

extern const usbd_driver_t midi_device_driver;

#ifdef __cplusplus
}
#endif

#endif // _USB_DEVICE_MIDI_DRIVER_H_