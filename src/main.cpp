#include "peripherals/Controller.h"
#include "peripherals/Display.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"
#include "usb/device/hid/ps4_auth.h"
#include "usb/device_driver.h"
#include "utils/Menu.h"
#include "utils/PS4AuthProvider.h"
#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"
#include "PS4AuthConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <stdio.h>

using namespace Doncon;

queue_t control_queue;
queue_t menu_display_queue;
queue_t drum_input_queue;
queue_t controller_input_queue;

queue_t auth_challenge_queue;
queue_t auth_signed_challenge_queue;

enum class ControlCommand {
    SetUsbMode,
    SetPlayerLed,
    SetLedBrightness,
    SetLedEnablePlayerColor,
    EnterMenu,
    ExitMenu,
};

struct ControlMessage {
    ControlCommand command;
    union {
        usb_mode_t usb_mode;
        usb_player_led_t player_led;
        uint8_t led_brightness;
        bool led_enable_player_color;
    } data;
};

void core1_task() {
    multicore_lockout_victim_init();

    // Init i2c port here because Controller and Display share it and
    // therefore can't init it themself.
    gpio_set_function(Config::Default::i2c_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(Config::Default::i2c_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(Config::Default::i2c_config.sda_pin);
    gpio_pull_up(Config::Default::i2c_config.scl_pin);
    i2c_init(Config::Default::i2c_config.block, Config::Default::i2c_config.speed_hz);

    Peripherals::Controller controller(Config::Default::controller_config);
    Peripherals::StatusLed led(Config::Default::led_config);
    Peripherals::Display display(Config::Default::display_config);

    Utils::PS4AuthProvider ps4authprovider;
    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge;

    Utils::InputState input_state;
    Utils::Menu::State menu_display_msg;
    ControlMessage control_msg;

    while (true) {
        controller.updateInputState(input_state);

        queue_try_add(&controller_input_queue, &input_state.controller);
        queue_try_remove(&drum_input_queue, &input_state.drum);

        if (queue_try_remove(&control_queue, &control_msg)) {
            switch (control_msg.command) {
            case ControlCommand::SetUsbMode:
                display.setUsbMode(control_msg.data.usb_mode);
                break;
            case ControlCommand::SetPlayerLed:
                switch (control_msg.data.player_led.type) {
                case USB_PLAYER_LED_ID:
                    display.setPlayerId(control_msg.data.player_led.id);
                    break;
                case USB_PLAYER_LED_COLOR:
                    led.setPlayerColor({control_msg.data.player_led.red, control_msg.data.player_led.green,
                                        control_msg.data.player_led.blue});
                }
                break;
            case ControlCommand::SetLedBrightness:
                led.setBrightness(control_msg.data.led_brightness);
                break;
            case ControlCommand::SetLedEnablePlayerColor:
                led.setEnablePlayerColor(control_msg.data.led_enable_player_color);
                break;
            case ControlCommand::EnterMenu:
                display.showMenu();
                break;
            case ControlCommand::ExitMenu:
                display.showIdle();
                break;
            }
        }
        if (queue_try_remove(&menu_display_queue, &menu_display_msg)) {
            display.setMenuState(menu_display_msg);
        }
        if (queue_try_remove(&auth_challenge_queue, auth_challenge.data())) {
            const auto signed_challenge = ps4authprovider.sign(auth_challenge);
            queue_try_remove(&auth_signed_challenge_queue, nullptr); // clear queue first
            queue_try_add(&auth_signed_challenge_queue, &signed_challenge);
        }

        led.setInputState(input_state);
        display.setInputState(input_state);

        led.update();
        display.update();
    }
}

int main() {
    queue_init(&control_queue, sizeof(ControlMessage), 1);
    queue_init(&menu_display_queue, sizeof(Utils::Menu::State), 1);
    queue_init(&drum_input_queue, sizeof(Utils::InputState::Drum), 1);
    queue_init(&controller_input_queue, sizeof(Utils::InputState::Controller), 1);
    queue_init(&auth_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);
    queue_init(&auth_signed_challenge_queue, sizeof(std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH>), 1);

    Utils::InputState input_state;
    std::array<uint8_t, Utils::PS4AuthProvider::SIGNATURE_LENGTH> auth_challenge_response;

    auto settings_store = std::make_shared<Utils::SettingsStore>();
    Utils::Menu menu(settings_store);

    const auto mode = settings_store->getUsbMode();

    Peripherals::Drum drum(Config::Default::drum_config);

    multicore_launch_core1(core1_task);

    usbd_driver_init(mode);
    usbd_driver_set_player_led_cb([](usb_player_led_t player_led) {
        const auto ctrl_message = ControlMessage{ControlCommand::SetPlayerLed, {.player_led = player_led}};
        queue_try_add(&control_queue, &ctrl_message);
    });

    if (Config::PS4Auth::config.enabled) {
        ps4_auth_init(Config::PS4Auth::config.key_pem.c_str(), Config::PS4Auth::config.key_pem.size() + 1,
                      Config::PS4Auth::config.serial.data(), Config::PS4Auth::config.signature.data(),
                      [](const uint8_t *challenge) { queue_try_add(&auth_challenge_queue, challenge); });
    }

    stdio_init_all();

    const auto readSettings = [&]() {
        ControlMessage ctrl_message;

        ctrl_message = {ControlCommand::SetUsbMode, {.usb_mode = mode}};
        queue_add_blocking(&control_queue, &ctrl_message);

        ctrl_message = {ControlCommand::SetLedBrightness, {.led_brightness = settings_store->getLedBrightness()}};
        queue_add_blocking(&control_queue, &ctrl_message);
        ctrl_message = {ControlCommand::SetLedEnablePlayerColor,
                        {.led_enable_player_color = settings_store->getLedEnablePlayerColor()}};
        queue_add_blocking(&control_queue, &ctrl_message);

        drum.setDebounceDelay(settings_store->getDebounceDelay());
        drum.setThresholds(settings_store->getTriggerThresholds());
    };

    readSettings();

    while (true) {
        drum.updateInputState(input_state);
        queue_try_remove(&controller_input_queue, &input_state.controller);

        const auto drum_message = input_state.drum;

        if (menu.active()) {
            menu.update(input_state.controller);
            if (menu.active()) {
                const auto display_msg = menu.getState();
                queue_add_blocking(&menu_display_queue, &display_msg);
            } else {
                settings_store->store();

                ControlMessage ctrl_message = {ControlCommand::ExitMenu, {}};
                queue_add_blocking(&control_queue, &ctrl_message);
            }

            readSettings();
            input_state.releaseAll();

        } else if (input_state.checkHotkey()) {
            menu.activate();

            ControlMessage ctrl_message{ControlCommand::EnterMenu, {}};
            queue_add_blocking(&control_queue, &ctrl_message);
        }

        usbd_driver_send_report(input_state.getReport(mode));
        usbd_driver_task();

        queue_try_add(&drum_input_queue, &drum_message);

        if (queue_try_remove(&auth_signed_challenge_queue, auth_challenge_response.data())) {
            ps4_auth_set_signed_challenge(auth_challenge_response.data());
        }
    }

    return 0;
}