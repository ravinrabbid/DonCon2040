#include "usb/device/hid/ps4_driver.h"

#include "usb/device/hid/common.h"
#include "usb/device/hid/ps4_auth.h"

#include "pico/unique_id.h"

#include "tusb.h"

const tusb_desc_device_t ps4_tatacon_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x0F0D,  // HORI
    .idProduct = 0x00C9, // PS4-095 aka Taiko Drum
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

const tusb_desc_device_t ds4_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x054C,  // Sny
    .idProduct = 0x05C4, // Dualshock 4
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_HID,
    USBD_ITF_MAX,
};

const uint8_t ps4_desc_hid_report[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,       // Usage (Game Pad)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x32,       //   Usage (Z)
    0x09, 0x35,       //   Usage (Rz)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x04,       //   Report Count (4)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x07,       //   Logical Maximum (7)
    0x35, 0x00,       //   Physical Minimum (0)
    0x46, 0x3B, 0x01, //   Physical Maximum (315)
    0x65, 0x14,       //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,       //   Report Size (4)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)

    0x65, 0x00, //   Unit (None)
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (0x01)
    0x29, 0x0E, //   Usage Maximum (0x0E)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x0E, //   Report Count (14)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,       //   Usage (0x20)
    0x75, 0x06,       //   Report Size (6)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x05, 0x01,       //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,       //   Usage (Rx)
    0x09, 0x34,       //   Usage (Ry)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,       //   Usage (0x21)
    0x95, 0x36,       //   Report Count (54)
    0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x05, //   Report ID (5)
    0x09, 0x22, //   Usage (0x22)
    0x95, 0x1F, //   Report Count (31)
    0x91, 0x02, //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0x85, 0x03,       //   Report ID (3)
    0x0A, 0x21, 0x27, //   Usage (0x2721)
    0x95, 0x2F,       //   Report Count (47)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0x85, 0x02,       //   Report ID (2)
    0x09, 0x24,       //   Usage (0x24)
    0x95, 0x24,       //   Report Count (36)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x08,       //   Report ID (8)
    0x09, 0x25,       //   Usage (0x25)
    0x95, 0x03,       //   Report Count (3)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x10,       //   Report ID (16)
    0x09, 0x26,       //   Usage (0x26)
    0x95, 0x04,       //   Report Count (4)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x11,       //   Report ID (17)
    0x09, 0x27,       //   Usage (0x27)
    0x95, 0x02,       //   Report Count (2)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x12,       //   Report ID (18)
    0x06, 0x02, 0xFF, //   Usage Page (Vendor Defined 0xFF02)
    0x09, 0x21,       //   Usage (0x21)
    0x95, 0x0F,       //   Report Count (15)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x13,       //   Report ID (19)
    0x09, 0x22,       //   Usage (0x22)
    0x95, 0x16,       //   Report Count (22)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x14,       //   Report ID (20)
    0x06, 0x05, 0xFF, //   Usage Page (Vendor Defined 0xFF05)
    0x09, 0x20,       //   Usage (0x20)
    0x95, 0x10,       //   Report Count (16)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x15,       //   Report ID (21)
    0x09, 0x21,       //   Usage (0x21)
    0x95, 0x2C,       //   Report Count (44)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x06, 0x80, 0xFF, //   Usage Page (Vendor Defined 0xFF80)
    0x85, 0x80,       //   Report ID (128)
    0x09, 0x20,       //   Usage (0x20)
    0x95, 0x06,       //   Report Count (6)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x81,       //   Report ID (129)
    0x09, 0x21,       //   Usage (0x21)
    0x95, 0x06,       //   Report Count (6)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x82,       //   Report ID (130)
    0x09, 0x22,       //   Usage (0x22)
    0x95, 0x05,       //   Report Count (5)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x83,       //   Report ID (131)
    0x09, 0x23,       //   Usage (0x23)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x84,       //   Report ID (132)
    0x09, 0x24,       //   Usage (0x24)
    0x95, 0x04,       //   Report Count (4)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x85,       //   Report ID (133)
    0x09, 0x25,       //   Usage (0x25)
    0x95, 0x06,       //   Report Count (6)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x86,       //   Report ID (134)
    0x09, 0x26,       //   Usage (0x26)
    0x95, 0x06,       //   Report Count (6)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x87,       //   Report ID (135)
    0x09, 0x27,       //   Usage (0x27)
    0x95, 0x23,       //   Report Count (35)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x88,       //   Report ID (136)
    0x09, 0x28,       //   Usage (0x28)
    0x95, 0x22,       //   Report Count (34)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x89,       //   Report ID (137)
    0x09, 0x29,       //   Usage (0x29)
    0x95, 0x02,       //   Report Count (2)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x90,       //   Report ID (144)
    0x09, 0x30,       //   Usage (0x30)
    0x95, 0x05,       //   Report Count (5)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x91,       //   Report ID (145)
    0x09, 0x31,       //   Usage (0x31)
    0x95, 0x03,       //   Report Count (3)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x92,       //   Report ID (146)
    0x09, 0x32,       //   Usage (0x32)
    0x95, 0x03,       //   Report Count (3)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x93,       //   Report ID (147)
    0x09, 0x33,       //   Usage (0x33)
    0x95, 0x0C,       //   Report Count (12)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA0,       //   Report ID (160)
    0x09, 0x40,       //   Usage (0x40)
    0x95, 0x06,       //   Report Count (6)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA1,       //   Report ID (161)
    0x09, 0x41,       //   Usage (0x41)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA2,       //   Report ID (162)
    0x09, 0x42,       //   Usage (0x42)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA3,       //   Report ID (163)
    0x09, 0x43,       //   Usage (0x43)
    0x95, 0x30,       //   Report Count (48)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA4,       //   Report ID (164)
    0x09, 0x44,       //   Usage (0x44)
    0x95, 0x0D,       //   Report Count (13)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA5,       //   Report ID (165)
    0x09, 0x45,       //   Usage (0x45)
    0x95, 0x15,       //   Report Count (21)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA6,       //   Report ID (166)
    0x09, 0x46,       //   Usage (0x46)
    0x95, 0x15,       //   Report Count (21)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA7,       //   Report ID (247)
    0x09, 0x4A,       //   Usage (0x4A)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA8,       //   Report ID (250)
    0x09, 0x4B,       //   Usage (0x4B)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA9,       //   Report ID (251)
    0x09, 0x4C,       //   Usage (0x4C)
    0x95, 0x08,       //   Report Count (8)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAA,       //   Report ID (252)
    0x09, 0x4E,       //   Usage (0x4E)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAB,       //   Report ID (253)
    0x09, 0x4F,       //   Usage (0x4F)
    0x95, 0x39,       //   Report Count (57)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAC,       //   Report ID (254)
    0x09, 0x50,       //   Usage (0x50)
    0x95, 0x39,       //   Report Count (57)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAD,       //   Report ID (255)
    0x09, 0x51,       //   Usage (0x51)
    0x95, 0x0B,       //   Report Count (11)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAE,       //   Report ID (256)
    0x09, 0x52,       //   Usage (0x52)
    0x95, 0x01,       //   Report Count (1)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAF,       //   Report ID (175)
    0x09, 0x53,       //   Usage (0x53)
    0x95, 0x02,       //   Report Count (2)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xB0,       //   Report ID (176)
    0x09, 0x54,       //   Usage (0x54)
    0x95, 0x3F,       //   Report Count (63)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             // End Collection

    0x06, 0xF0, 0xFF, // Usage Page (Vendor Defined 0xFFF0)
    0x09, 0x40,       // Usage (0x40)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0xF0,       //   Report ID (-16) AUTH F0
    0x09, 0x47,       //   Usage (0x47)
    0x95, 0x3F,       //   Report Count (63)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF1,       //   Report ID (-15) AUTH F1
    0x09, 0x48,       //   Usage (0x48)
    0x95, 0x3F,       //   Report Count (63)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF2,       //   Report ID (-14) AUTH F2
    0x09, 0x49,       //   Usage (0x49)
    0x95, 0x0F,       //   Report Count (15)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF3,       //   Report ID (-13) Auth F3 (Reset)
    0x0A, 0x01, 0x47, //   Usage (0x4701)
    0x95, 0x07,       //   Report Count (7)
    0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,             // End Collection
};

#define USBD_PS4_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
const uint8_t ps4_desc_cfg[] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_PS4_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_HID_INOUT_DESCRIPTOR(USBD_ITF_HID, 0, 0, sizeof(ps4_desc_hid_report), 0x03, 0x84, CFG_TUD_HID_EP_BUFSIZE, 1),
};

// MAC Address
static uint8_t ps4_0x81_report[] = {0x39, 0x39, 0x39, 0x68, 0x22, 0x00};

// Paring data
// (Device MAC + {x1C, 0x08, 0x25} + Paired Host MAC)
static uint8_t ps4_0x12_report[] = {0x39, 0x39, 0x39, 0x68, 0x22, 0x00, 0x1C, 0x08,
                                    0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Version Info
static const uint8_t ps4_0xa3_report[] = {0x4a, 0x75, 0x6c, 0x20, 0x31, 0x31, 0x20, 0x32, 0x30, 0x31, 0x36, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x3a, 0x33, 0x33, 0x3a, 0x33, 0x38,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x64,
                                          0x01, 0x00, 0x00, 0x00, 0x09, 0x70, 0x00, 0x02, 0x00, 0x80, 0x03, 0x00};

// Calibration Data
static const uint8_t ps4_0x02_report[] = {0x04, 0x00, 0xf9, 0xff, 0x06, 0x00, 0x1d, 0x22, 0xec, 0xdd, 0x68, 0x22,
                                          0x88, 0xdd, 0xa9, 0x23, 0x62, 0xdc, 0x1c, 0x02, 0x1c, 0x02, 0x05, 0x20,
                                          0xfb, 0xdf, 0x49, 0x20, 0xb7, 0xdf, 0x0d, 0x20, 0xf4, 0xdf, 0x01, 0x00};

// Controller Descriptor
static const uint8_t ps4_0x03_report[] = {0x21, 0x27, 0x04, 0xcf, 0x00, 0x2c, 0x56, 0x08, 0x00, 0x3d, 0x00, 0xe8,
                                          0x03, 0x04, 0x00, 0xff, 0x7f, 0x0d, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static hid_ps4_report_t last_report = {};

bool send_hid_ps4_report(usb_report_t report) {
    bool result = false;
    if (tud_hid_ready()) {
        result = tud_hid_report(0, report.data, report.size);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(hid_ps4_report_t)));

    return result;
}

uint16_t hid_ps4_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)itf;
    (void)reqlen;

    static bool do_init_mac = true;

    if (do_init_mac) {
        pico_unique_board_id_t uid;
        pico_get_unique_board_id(&uid);

        // Genrate manufacturer specific using pico board id
        for (uint8_t i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
            ps4_0x81_report[(i % 3)] ^= uid.id[i];
            ps4_0x12_report[(i % 3)] ^= uid.id[i];
        }

        do_init_mac = false;
    }

    if (report_type == HID_REPORT_TYPE_INPUT) {
        memcpy(buffer, &last_report, sizeof(hid_ps4_report_t));
        return sizeof(hid_ps4_report_t);
    }

    if (report_type == HID_REPORT_TYPE_FEATURE) {
        switch (report_id) {
        case 0x81:
            memcpy(buffer, ps4_0x81_report, sizeof(ps4_0x81_report));
            return sizeof(ps4_0x81_report);
        case 0x12:
            memcpy(buffer, ps4_0x12_report, sizeof(ps4_0x12_report));
            return sizeof(ps4_0x12_report);
        case 0xa3:
            memcpy(buffer, ps4_0xa3_report, sizeof(ps4_0xa3_report));
            return sizeof(ps4_0xa3_report);
        case 0x02:
            memcpy(buffer, ps4_0x02_report, sizeof(ps4_0x02_report));
            return sizeof(ps4_0x02_report);
        case 0x03:
            memcpy(buffer, ps4_0x03_report, sizeof(ps4_0x03_report));
            return sizeof(ps4_0x03_report);
        case 0xF1: { // Signature nonce
            return ps4_auth_get_challenge_report(report_id, buffer);
        }
        case 0xF2: { // Signing state
            return ps4_auth_get_challenge_state_report(report_id, buffer);
        }
        case 0xF3: // Reset auth
            return ps4_auth_get_reset_report(report_id, buffer);
        default:
            break;
        }
    }

    return 0;
}

typedef struct __attribute((packed, aligned(1))) {
    uint8_t content_flags; // 0x01: Rumble, 0x02: Color, 0x04: Flash
    uint8_t unknown1[2];
    uint8_t rumble_weak;
    uint8_t rumble_strong;
    uint8_t led_red;
    uint8_t led_green;
    uint8_t led_blue;
    uint8_t flash_bright_time;
    uint8_t flash_dark_time;
    uint8_t unknown2[21];
} hid_ps4_ouput_report_t;

void hid_ps4_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)itf;

    switch (report_type) {
    case HID_REPORT_TYPE_FEATURE: {
        switch (report_id) {
        case 0xF0: // Auth payload
            ps4_auth_set_challenge_report(report_id, buffer, bufsize);
            break;
        default:
            break;
        }
    } break;
    case HID_REPORT_TYPE_OUTPUT: {
        if (report_id == 0 && bufsize > 0) {
            report_id = buffer[0];
            buffer = &buffer[1];
            bufsize--;
        }

        switch (report_id) {
        case 0x05: // LEDs and Rumble
            if (bufsize == sizeof(hid_ps4_ouput_report_t)) {
                hid_ps4_ouput_report_t *report = (hid_ps4_ouput_report_t *)buffer;
                if (report->content_flags & 0x02) {
                    usb_player_led_t player_led = {.type = USB_PLAYER_LED_COLOR,
                                                   .red = report->led_red,
                                                   .green = report->led_green,
                                                   .blue = report->led_blue};
                    usbd_driver_get_player_led_cb()(player_led);
                }
            }
            break;
        default:
            break;
        }
    } break;
    default:
        break;
    }
}

const usbd_driver_t *get_hid_ds4_device_driver() {
    static const usbd_driver_t hid_ds4_device_driver = {
        .name = "DS4",
        .app_driver = &hid_app_driver,
        .desc_device = &ds4_desc_device,
        .desc_cfg = ps4_desc_cfg,
        .desc_bos = NULL,
        .send_report = send_hid_ps4_report,
    };
    return &hid_ds4_device_driver;
}

const usbd_driver_t *get_hid_ps4_tatacon_device_driver() {
    static const usbd_driver_t hid_ps4_tatacon_device_driver = {
        .name = "PS4 Tatacon",
        .app_driver = &hid_app_driver,
        .desc_device = &ps4_tatacon_desc_device,
        .desc_cfg = ps4_desc_cfg,
        .desc_bos = NULL,
        .send_report = send_hid_ps4_report,
    };
    return &hid_ps4_tatacon_device_driver;
}