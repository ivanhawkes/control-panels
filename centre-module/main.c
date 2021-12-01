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

const uint button1 = 0;
const uint button2 = 1;
const uint button3 = 2;
const uint button4 = 3;

const uint LED1 = 6;
const uint LED2 = 7;
const uint LED3 = 8;
const uint LED4 = 9;

// Indicate we have power. It's a handy diagnostic that shows the code is running.
const uint POWER_LED = 15;

uint8_t keyPressed = HID_KEY_NONE;
const uint8_t keyPressed1 = HID_KEY_PAGE_UP;
const uint8_t keyPressed2 = HID_KEY_PAGE_DOWN;
const uint8_t keyPressed3 = HID_KEY_ARROW_LEFT;
const uint8_t keyPressed4 = HID_KEY_ARROW_RIGHT;


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
				keycode[0] = keyPressed;

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
	const uint32_t interval_ms = 5;
	static uint32_t startMS = 0;

	if (board_millis() - startMS < interval_ms) return; // not enough time
	startMS += interval_ms;

//	uint32_t const btn = board_button_read();
	uint32_t const btn = (keyPressed != HID_KEY_NONE);

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
	
	bool value1 = gpio_get(button1);
	bool value2 = gpio_get(button2);
	bool value3 = gpio_get(button3);
	bool value4 = gpio_get(button4);
	
	keyPressed = HID_KEY_NONE;

	if (value1)
	{
		gpio_put(LED1, false);
	}
	else
	{
		keyPressed = keyPressed1;
		gpio_put(LED1, true);
	}

	if (value2)
	{
		gpio_put(LED2, false);
	}
	else
	{
		keyPressed = keyPressed2;
		gpio_put(LED2, true);
	}

	if (value3)
	{
		gpio_put(LED3, false);
	}
	else
	{
		keyPressed = keyPressed3;
		gpio_put(LED3, true);
	}

	if (value4)
	{
		gpio_put(LED4, false);
	}
	else
	{
		keyPressed = keyPressed4;
		gpio_put(LED4, true);
	}

	gpio_put(POWER_LED, true);
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
}


int main(void)
{
	board_init();
	tusb_init();
	
	// Init the USB / UART IO.
	stdio_init_all();

	gpio_init(button1);                                      
    gpio_set_dir(button1, GPIO_IN);
	gpio_init(button2);
    gpio_set_dir(button2, GPIO_IN);
	gpio_init(button3);
    gpio_set_dir(button3, GPIO_IN);
	gpio_init(button4);
    gpio_set_dir(button4, GPIO_IN);
	
	gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);

	gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);

	gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);

	gpio_init(LED4);
    gpio_set_dir(LED4, GPIO_OUT);

	gpio_init(POWER_LED);
    gpio_set_dir(POWER_LED, GPIO_OUT);

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
