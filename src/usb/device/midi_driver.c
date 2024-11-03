#include "usb/device/midi_driver.h"

#include "class/midi/midi_device.h"

#include "tusb.h"

const tusb_desc_device_t midi_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x1209,
    .idProduct = 0x3902,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUFACTURER,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

enum {
    USBD_ITF_MIDI,
    USBD_ITF_MIDI_STREAMING,
    USBD_ITF_MAX,
};

#define EPNUM_MIDI_OUT 0x01
#define EPNUM_MIDI_IN 0x01

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

const uint8_t midi_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_LANGUAGE, USBD_DESC_LEN, 0, USBD_MAX_POWER_MAX),
    TUD_MIDI_DESCRIPTOR(USBD_ITF_MIDI, 0, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), CFG_TUD_MIDI_EP_BUFSIZE),
};

static midi_report_t last_report = {};

static void write_midi_message(uint8_t status, uint8_t byte1, uint8_t byte2) {
    uint8_t midi_message[3] = {status, byte1, byte2};
    tud_midi_stream_write(0, midi_message, sizeof(midi_message));
}

static void set_note(uint8_t channel, bool on, uint8_t pitch, uint8_t velocity) {
    uint8_t status = on ? (0x90 | channel) : (0x80 | channel);
    velocity = velocity > 127 ? 127 : velocity;
    write_midi_message(status, pitch, velocity);
}

bool send_midi_report(usb_report_t report) {
    static uint8_t percussion_channel = 9;

    midi_report_t *midi_report = (midi_report_t *)report.data;

    if (midi_report->status.acoustic_bass_drum != last_report.status.acoustic_bass_drum) {
        set_note(percussion_channel, midi_report->status.acoustic_bass_drum, 35,
                 midi_report->velocity.acoustic_bass_drum);
    }
    if (midi_report->status.electric_bass_drum != last_report.status.electric_bass_drum) {
        set_note(percussion_channel, midi_report->status.electric_bass_drum, 36,
                 midi_report->velocity.electric_bass_drum);
    }
    if (midi_report->status.drumsticks != last_report.status.drumsticks) {
        set_note(percussion_channel, midi_report->status.drumsticks, 31, midi_report->velocity.drumsticks);
    }
    if (midi_report->status.side_stick != last_report.status.side_stick) {
        set_note(percussion_channel, midi_report->status.side_stick, 37, midi_report->velocity.side_stick);
    }

    memcpy(&last_report, report.data, tu_min16(report.size, sizeof(midi_report_t)));

    return true;
}

void tud_midi_rx_cb(uint8_t itf) {
    (void)itf;

    // Read and discard incoming data to avoid blocking the sender
    uint8_t packet[4];
    while (tud_midi_available()) {
        tud_midi_packet_read(packet);
    }
}

static const usbd_class_driver_t midi_app_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "MIDI",
#endif
    .init = midid_init,
    .reset = midid_reset,
    .open = midid_open,
    .control_xfer_cb = midid_control_xfer_cb,
    .xfer_cb = midid_xfer_cb,
    .sof = NULL};

const usbd_driver_t midi_device_driver = {
    .name = "MIDI",
    .app_driver = &midi_app_driver,
    .desc_device = &midi_desc_device,
    .desc_cfg = midi_desc_cfg,
    .desc_bos = NULL,
    .send_report = send_midi_report,
};
