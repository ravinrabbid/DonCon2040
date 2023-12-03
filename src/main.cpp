#include "peripherals/Controller.h"
#include "peripherals/Display.h"
#include "peripherals/Drum.h"
#include "peripherals/StatusLed.h"
#include "usb/usb_driver.h"
#include "utils/Menu.h"
#include "utils/SettingsStore.h"

#include "GlobalConfiguration.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include <stdio.h>

using namespace Doncon;

queue_t control_queue;
queue_t menu_display_queue;
queue_t drum_input_queue;
queue_t controller_input_queue;

enum class ControlCommand {
    SetUsbMode,
    SetPlayerLed,
    SetLedBrightness,
    EnterMenu,
    ExitMenu,
};

struct ControlMessage {
    ControlCommand command;
    union {
        usb_mode_t usb_mode;
        usb_player_led_t player_led;
        uint8_t brightness;
    } data;
};

void core1_task() {
    multicore_lockout_victim_init();

    gpio_set_function(Config::Default::i2c_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(Config::Default::i2c_config.scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(Config::Default::i2c_config.sda_pin);
    gpio_pull_up(Config::Default::i2c_config.scl_pin);
    i2c_init(Config::Default::i2c_config.block, Config::Default::i2c_config.speed_hz);

    Peripherals::Buttons buttons(Config::Default::button_config);
    Peripherals::StatusLed led(Config::Default::led_config);
    Peripherals::Display display(Config::Default::display_config);

    Utils::InputState input_state;
    Utils::Menu::State menu_display_msg;
    ControlMessage control_msg;

    while (true) {
        buttons.updateInputState(input_state);

        queue_try_add(&controller_input_queue, &input_state.controller);
        queue_try_remove(&drum_input_queue, &input_state.drum);

        if (queue_try_remove(&control_queue, &control_msg)) {
            switch (control_msg.command) {
            case ControlCommand::SetUsbMode:
                display.setUsbMode(control_msg.data.usb_mode);
                break;
            case ControlCommand::SetPlayerLed:
                if (control_msg.data.player_led.type == USB_PLAYER_LED_ID) {
                    display.setPlayerId(control_msg.data.player_led.id);
                } else if (control_msg.data.player_led.type == USB_PLAYER_LED_COLOR) {
                }
                break;
            case ControlCommand::SetLedBrightness:
                led.setBrightness(control_msg.data.brightness);
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
    multicore_launch_core1(core1_task);

    Utils::InputState input_state;

    auto settings_store = std::make_shared<Utils::SettingsStore>();
    Utils::Menu menu(settings_store);

    Peripherals::Drum drum(Config::Default::drum_config);

    auto mode = settings_store->getUsbMode();
    usb_driver_init(mode);
    usb_driver_set_player_led_cb([](usb_player_led_t player_led) {
        auto ctrl_message = ControlMessage{ControlCommand::SetPlayerLed, {.player_led = player_led}};
        queue_add_blocking(&control_queue, &ctrl_message);
    });

    stdio_init_all();

    auto readSettings = [&]() {
        ControlMessage ctrl_message;

        ctrl_message = {ControlCommand::SetUsbMode, {.usb_mode = mode}};
        queue_add_blocking(&control_queue, &ctrl_message);

        ctrl_message = {ControlCommand::SetLedBrightness, {.brightness = settings_store->getLedBrightness()}};
        queue_add_blocking(&control_queue, &ctrl_message);

        drum.setDebounceDelay(settings_store->getDebounceDelay());
        drum.setThresholds(settings_store->getTriggerThresholds());
    };

    readSettings();

    while (true) {
        drum.updateInputState(input_state);
        queue_try_remove(&controller_input_queue, &input_state.controller);

        if (menu.active()) {
            menu.update(input_state.controller);
            if (menu.active()) {
                auto display_msg = menu.getState();
                queue_add_blocking(&menu_display_queue, &display_msg);
            } else {
                settings_store->store();

                ControlMessage ctrl_message = {ControlCommand::ExitMenu, {}};
                queue_add_blocking(&control_queue, &ctrl_message);
            }

            readSettings();

        } else if (input_state.checkHotkey()) {
            menu.activate();

            input_state.releaseAll();
            usb_driver_send_and_receive_report(input_state.getReport(mode));

            ControlMessage ctrl_message{ControlCommand::EnterMenu, {}};
            queue_add_blocking(&control_queue, &ctrl_message);
        } else {
            usb_driver_send_and_receive_report(input_state.getReport(mode));
        }

        usb_driver_task();

        queue_try_add(&drum_input_queue, &input_state);
    }

    return 0;
}