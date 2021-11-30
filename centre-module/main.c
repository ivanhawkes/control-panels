#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "pico/stdlib.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 1000,
	BLINK_SUSPENDED = 2500,
};

uint32_t blinkIntervalMS = BLINK_NOT_MOUNTED;
const uint32_t checkIntervalMS = 1;

const uint Button1 = 0;
const uint Button2 = 1;
const uint Button3 = 2;
const uint Button4 = 3;
const uint LED1 = 4;
const uint LED2 = 5;
const uint LED3 = 6;

const uint GPIO15 = 15;

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted

void tud_mount_cb(void)
{
	blinkIntervalMS = BLINK_MOUNTED;
}

// Invoked when device is unmounted

void tud_umount_cb(void)
{
	blinkIntervalMS = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus

void tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	blinkIntervalMS = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed

void tud_resume_cb(void)
{
	blinkIntervalMS = BLINK_MOUNTED;
}

//
// USB HID
//

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
	// skip if hid is not ready yet
	if (!tud_hid_ready()) return;

	switch (report_id)
	{
		case REPORT_ID_KEYBOARD:
		{
			// use to avoid send multiple consecutive zero report for keyboard
			static bool has_keyboard_key = false;

			if (btn)
			{
				uint8_t keycode[6] = {0};
				keycode[0] = HID_KEY_A;

				tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
				has_keyboard_key = true;
			}
			else
			{
				// send empty key report if previously has key pressed
				if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
				has_keyboard_key = false;
			}
		}
		break;

		case REPORT_ID_MOUSE:
		{
			int8_t const delta = 5;

			// no button, right + down, no scroll, no pan
			tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
		}
		break;

		case REPORT_ID_CONSUMER_CONTROL:
		{
			// use to avoid send multiple consecutive zero report
			static bool has_consumer_key = false;

			if (btn)
			{
				// volume down
				uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
				tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
				has_consumer_key = true;
			}
			else
			{
				// send empty key report (release key) if previously has key pressed
				uint16_t empty_key = 0;
				if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
				has_consumer_key = false;
			}
		}
		break;

		case REPORT_ID_GAMEPAD:
		{
			// use to avoid send multiple consecutive zero report for keyboard
			static bool has_gamepad_key = false;

			hid_gamepad_report_t report =
			{
				.x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
				.hat = 0, .buttons = 0
			};

			if (btn)
			{
				report.hat = GAMEPAD_HAT_UP;
				report.buttons = GAMEPAD_BUTTON_A;
				tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

				has_gamepad_key = true;
			}
			else
			{
				report.hat = GAMEPAD_HAT_CENTERED;
				report.buttons = 0;
				if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
				has_gamepad_key = false;
			}
		}
		break;

		default: break;
	}
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete

void hid_task(void)
{
	// Poll every 10ms
	const uint32_t interval_ms = 10;
	static uint32_t startMS = 0;

	if (board_millis() - startMS < interval_ms) return; // not enough time
	startMS += interval_ms;

	uint32_t const btn = board_button_read();

	// Remote wakeup
	if (tud_suspended() && btn)
	{
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	}
	else
	{
		// Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
		send_hid_report(REPORT_ID_KEYBOARD, btn);
	}
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
	(void)instance;
	(void)len;

	uint8_t next_report_id = report[0] + 1;

	if (next_report_id < REPORT_ID_COUNT)
	{
		send_hid_report(next_report_id, board_button_read());
	}
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	// TODO not Implemented
	(void)instance;
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	(void)instance;

	if (report_type == HID_REPORT_TYPE_OUTPUT)
	{
		// Set keyboard LED e.g Capslock, Numlock etc...
		if (report_id == REPORT_ID_KEYBOARD)
		{
			// bufsize should be (at least) 1
			if (bufsize < 1) return;

			uint8_t const kbd_leds = buffer[0];

			if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
			{
				// Capslock On: disable blink, turn led on
				blinkIntervalMS = 0;
				board_led_write(true);
			}
			else
			{
				// Caplocks Off: back to normal blink
				board_led_write(false);
				blinkIntervalMS = BLINK_MOUNTED;
			}
		}
	}
}


void ButtonTask(void)
{
	static uint32_t startMS = 0;

	// Shortcut.
	if (board_millis() - startMS < checkIntervalMS)
		return;

	printf("Tick\n");
	startMS += checkIntervalMS;
	
	bool value1 = gpio_get(Button1);
	bool value2 = gpio_get(Button2);
	bool value3 = gpio_get(Button3);
	//bool value4 = gpio_get(Button4);
	
	value1 ? gpio_put(LED1, false) : gpio_put(LED1, true);
	value2 ? gpio_put(LED2, false) : gpio_put(LED2, true);
	value3 ? gpio_put(LED3, false) : gpio_put(LED3, true);

	gpio_put(GPIO15, true);
}


void LEDBlinkingTask(void)
{
	static uint32_t startMS = 0;
	static bool ledState = false;

	// blink is disabled
	if (!blinkIntervalMS) return;

	// Blink every interval ms
	if (board_millis() - startMS < blinkIntervalMS)
		return;

	startMS += blinkIntervalMS;

	board_led_write(ledState);
	ledState = 1 - ledState; // toggle

	gpio_put(GPIO15, true);
}


int main(void)
{
	board_init();
	tusb_init();
	
	// Init the USB / UART IO.
	stdio_init_all();

	gpio_init(Button1);
    gpio_set_dir(Button1, GPIO_IN);
	gpio_init(Button2);
    gpio_set_dir(Button2, GPIO_IN);
	gpio_init(Button3);
    gpio_set_dir(Button3, GPIO_IN);
	gpio_init(Button4);
    gpio_set_dir(Button4, GPIO_IN);
	
	gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
	gpio_put(LED1, true);

	gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);
	gpio_put(LED2, true);

	gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);
	gpio_put(LED3, true);

	gpio_init(GPIO15);
    gpio_set_dir(GPIO15, GPIO_OUT);
	gpio_put(GPIO15, true);

	while (1)
	{
		// TinyUSB device task.
		tud_task();

		// Blinky blink.
		LEDBlinkingTask();

		// Check all our buttons.
		ButtonTask();
		
		// Keep them informed about HID changes.
		hid_task();
	}

	return 0;
}
