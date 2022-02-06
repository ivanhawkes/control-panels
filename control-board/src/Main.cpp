#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "AnalogueInput.h"
#include "DigitalInput.h"
#include "TinyUSBCallbacks.h"

#include "MPG.h"


// Blink pattern times.
enum
{
	blinkIntervalNotMounted = 250,
	blinkIntervalMounted = 1000,
	blinkIntervalSuspended = 2500,
};

uint32_t lastTaskTime;

// uint32_t blinkIntervalMS = blinkIntervalNotMounted;

static DigitalInputGroup g_digitalInputGroup;
static AnalogueInputGroup g_analogueSwitchGroup;


void SendGamepadHIDReport(uint8_t report_id)
{
	// skip if hid is not ready yet
	if (!tud_hid_ready())
	{
		// TODO: Is this a way we can use to find out the right timing?
		// NOTE: Code might run fast enough to not even be bothered, maybe.
		// printf("HID not ready.\n");
		return;
	}

	// use to avoid send multiple consecutive zero report for keyboard
	// static bool hasGamepadKey = false;

	hid_gamepad_report_t report = {
	    .x = static_cast<int8_t>(g_analogueSwitchGroup.GetXBox(0)),
	    .y = static_cast<int8_t>(g_analogueSwitchGroup.GetXBox(1)),
	    .z = 0,
	    .rz = 0,
	    .rx = 0,
	    .ry = 0,
	    .hat = 0,
	    .buttons = 0};

	if (g_digitalInputGroup.HasStateChanged() || g_analogueSwitchGroup.HasStateChanged())
	{
		// Normal report.
		uint8_t state = g_digitalInputGroup.GetState() & 0x0F;
		switch (state & 0x0F)
		{
			case 1:
				report.hat = GAMEPAD_HAT_UP;
				break;
			case 2:
				report.hat = GAMEPAD_HAT_DOWN;
				break;
			case 4:
				report.hat = GAMEPAD_HAT_LEFT;
				break;
			case 8:
				report.hat = GAMEPAD_HAT_RIGHT;
				break;

				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_UPLEFT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_DOWNLEFT; break;
				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_UPRIGHT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_DOWNRIGHT; break;

			default:
				report.hat = HID_HAT_NOTHING;
				break;
		}

		// HACK: TEST: Send raw bits out.
		report.hat = state;

		// Digital buttons.
		report.buttons = g_digitalInputGroup.GetState();
		tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

		// hasGamepadKey = true;
	}
	else
	{
		// // Empty report.
		// report.hat = g_digitalInputGroup.GetState() & 0x0F;
		// report.buttons = g_digitalInputGroup.GetState();

		// if (hasGamepadKey)
		// 	tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

		// hasGamepadKey = false;
	}
}

//--------------------------------------------------------------------+
// START TINY USB CALLBACKS
//--------------------------------------------------------------------+

// Invoked when device is mounted

void tud_mount_cb(void)
{
	blinkIntervalMS = blinkIntervalMounted;
}


// Invoked when device is unmounted

void tud_umount_cb(void)
{
	blinkIntervalMS = blinkIntervalNotMounted;
}


// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus

void tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	blinkIntervalMS = blinkIntervalSuspended;
}


// Invoked when usb bus is resumed

void tud_resume_cb(void)
{
	blinkIntervalMS = blinkIntervalMounted;
}


// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID

// TODO: ILH - figure out if this is a way to know the timing in advance.

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
	(void)instance;
	(void)len;

	uint8_t next_report_id = report[0] + 1;

	if (next_report_id < REPORT_ID_COUNT)
	{
		SendGamepadHIDReport(next_report_id);
	}
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request

uint16_t tud_hid_get_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
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

void tud_hid_set_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
	(void)instance;

	if (report_type == HID_REPORT_TYPE_OUTPUT)
	{
		// Set keyboard LED e.g Capslock, Numlock etc...
		if (report_id == REPORT_ID_KEYBOARD)
		{
			// bufsize should be (at least) 1
			if (bufsize < 1)
				return;

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
				blinkIntervalMS = blinkIntervalMounted;
			}
		}
	}
}

//--------------------------------------------------------------------+
// END TINY USB CALLBACKS
//--------------------------------------------------------------------+


// Every x ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is completed.

void SendHIDTask(void)
{
	// Remote wakeup
	if (tud_suspended())
	{
		// Wake up host if we are in suspend mode and REMOTE_WAKEUP feature is enabled by host.
		tud_remote_wakeup();
	}
	else
	{
		// Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
		SendGamepadHIDReport(REPORT_ID_GAMEPAD);
	}
}


void LEDBlinkingTask(void)
{
	static uint32_t elapsedMS = 0;
	static bool ledState = false;

	// blink is disabled
	if (!blinkIntervalMS)
		return;

	// Blink every interval ms
	if (board_millis() - elapsedMS < blinkIntervalMS)
		return;
	elapsedMS += blinkIntervalMS;

	board_led_write(ledState);
	ledState = 1 - ledState; // toggle
}


int main(void)
{
	// Init the USB / UART IO.
	stdio_init_all();
	adc_init();
	board_init();
	tusb_init();

	// Init the uart using our preferred settings.
	stdio_uart_init_full(uart0, 115200, 16, 17);

	printf("Centre console online.\n\n");

	// We'll track the time from startup.
	lastTaskTime = time_us_32();

	// Init our input handlers.
	g_digitalInputGroup.Init();
	g_analogueSwitchGroup.Init();

	printf("Initialisation complete.\n");

	while (true)
	{
		// TinyUSB device task.
		tud_task();

		// Blinky blink.
		LEDBlinkingTask();

		// Check all our switches.
		if (g_digitalInputGroup.OnTask()) {}

		// Check the analogue inputs.
		if (g_analogueSwitchGroup.OnTask()) {}

		// Keep them informed about HID changes.
		SendHIDTask();

		// Track time.
		lastTaskTime = time_us_32();
	}

	return 0;
}
