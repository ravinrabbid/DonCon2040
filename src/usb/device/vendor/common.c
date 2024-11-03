#include "usb/device/vendor/common.h"

#include "usb/device/vendor/debug_driver.h"
#include "usb/device/vendor/xinput_driver.h"
#include "usb/device_driver.h"

#include "tusb.h"

// Implement TinyUSB internal callback since vendor control requests are not forwarded to custom drivers.
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    switch (usbd_driver_get_mode()) {
    case USB_MODE_XBOX360:
    case USB_MODE_XBOX360_ANALOG_P1:
    case USB_MODE_XBOX360_ANALOG_P2:
        return xinput_control_xfer_cb(rhport, stage, request);
    case USB_MODE_DEBUG:
        return debug_control_xfer_cb(rhport, stage, request);
    default:
    }

    return false;
}