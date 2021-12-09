#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "pico/stdlib.h"
#include "pico/time.h"


// Blink pattern times.
enum
{
	blinkIntervalNotMounted = 250,
	blinkIntervalMounted = 1000,
	blinkIntervalSuspended = 2500,
};

uint32_t lastTaskTime;

uint32_t blinkIntervalMS = blinkIntervalNotMounted;

const size_t switchCount = 4;

struct DigitalSwitch
{
	// The GPIO pin number which the switch is connected to.
	uint32_t switchId;
	
	// If there is an LED associated with this switch it will have a GPIO pin number here.
	uint ledID;
	
	// The key code we will send when activating this switch?
	uint8_t mappedKey;
	
	// Friendly name for the switch.
	char mappedKeyName[25];
	
	// Is the switch currently depressed?
	bool isPressed;
	
	// Track the number of microseconds the switch has been in it's current state.
	uint32_t timeStateWasEntered;
};


struct DigitalSwitch switchArray[] = 
{
    {2, 6, HID_KEY_PAGE_UP, "PageUp", false},
    {3, 7, HID_KEY_PAGE_DOWN, "PageDown", false},
    {4, 8, HID_KEY_ARROW_LEFT, "ArrowLeft", false},
    {5, 9, HID_KEY_ARROW_RIGHT, "ArrowRight", false}
};

// Indicate we have power. It's a handy diagnostic that shows the code is running.
const uint POWER_LED = 15;

uint8_t keyPressed = HID_KEY_NONE;

//--------------------------------------------------------------------+
// Device callbacks
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

//
// USB HID
//

static void send_hid_report(uint8_t report_id, bool isKeyPressed)
{
	// skip if hid is not ready yet
	if (!tud_hid_ready()) return;

	switch (report_id)
	{
		case REPORT_ID_KEYBOARD:
		{
			// use to avoid send multiple consecutive zero report for keyboard
			static bool has_keyboard_key = false;

			if (isKeyPressed)
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
			// int8_t const delta = 5;

			// no button, right + down, no scroll, no pan
			// tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
		}
		break;

		case REPORT_ID_CONSUMER_CONTROL:
		{
			// use to avoid send multiple consecutive zero report
			static bool has_consumer_key = false;

			if (isKeyPressed)
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
				.x = 0,
				.y = 0,
				.z = 0,
				.rz = 0,
				.rx = 0,
				.ry = 0,
				.hat = 0,
				.buttons = 0
			};

			if (isKeyPressed)
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

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
	(void)instance;
	(void)len;

	bool const isKeyPressed = (keyPressed != HID_KEY_NONE);

	uint8_t next_report_id = report[0] + 1;

	if (next_report_id < REPORT_ID_COUNT)
	{
		send_hid_report(next_report_id, isKeyPressed);
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
				blinkIntervalMS = blinkIntervalMounted;
			}
		}
	}
}


// Every x ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is completed.

void hid_task(void)
{
	bool const isKeyPressed = (keyPressed != HID_KEY_NONE);
	gpio_put(POWER_LED, isKeyPressed);

	// Remote wakeup
	if (tud_suspended() && isKeyPressed)
	{
		// Wake up host if we are in suspend mode and REMOTE_WAKEUP feature is enabled by host.
		tud_remote_wakeup();
	}
	else
	{
		// Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
		send_hid_report(REPORT_ID_KEYBOARD, isKeyPressed);
	}
}


void SwitchTask()
{
	uint32_t currentTime = time_us_32();

	// Default is for nothing to happen.
	keyPressed = HID_KEY_NONE;

	// Detect their state.
	for (size_t i = 0; i < switchCount; i++)
	{
		// Get their pressed state.
		const bool currentSwitchState = gpio_get(switchArray[i].switchId);

		// State change - make something happen.
		if (switchArray[i].isPressed != currentSwitchState)
		{
			uint32_t timeInThisState = currentTime - switchArray[i].timeStateWasEntered;

			// DEBUG: Testing the time difference on loops.
			printf("\nLT: %u, CT: %u, TSE: %u, TiS: %u, ",
				lastTaskTime,
				currentTime,
				switchArray[i].timeStateWasEntered,
				timeInThisState);

			if (currentSwitchState)
			{
				gpio_put(switchArray[i].ledID, false);
				printf("%s\n", switchArray[i].mappedKeyName);
			}
			else
			{
				keyPressed = switchArray[i].mappedKey;
				gpio_put(switchArray[i].ledID, true);
				printf("%s\n", switchArray[i].mappedKeyName);
			}

			// Entering a new state, reset the time now.
			switchArray[i].timeStateWasEntered = currentTime;
			switchArray[i].isPressed = currentSwitchState;
		}
		else
		{
			// Same state as last run, make something less exciting happen.

			// DEBUG: Testing the time difference on loops.
			// printf(".");
		}
	}
}


void LEDBlinkingTask(void)
{
	static uint32_t elapsedMS = 0;
	static bool ledState = false;

	// blink is disabled
	if (!blinkIntervalMS) return;

	// Blink every interval ms
	if (board_millis() - elapsedMS < blinkIntervalMS)
		return;
	elapsedMS += blinkIntervalMS;

	//printf(".");
	board_led_write(ledState);
	ledState = 1 - ledState; // toggle
}


int main(void)
{
	// Init the USB / UART IO.
	stdio_init_all();
	board_init();
	tusb_init();
	
	printf("Centre console online.\n");

	// We'll track the time from startup.
	lastTaskTime = time_us_32();

	// Initialise all the switches.
	for (size_t i = 0; i < switchCount; i++)
	{
		// Initialise the switch pins for input.
		gpio_init(switchArray[i].switchId);
		gpio_set_dir(switchArray[i].switchId, GPIO_IN);

		// Initialise the LED pins for output.
		gpio_init(switchArray[i].ledID);
		gpio_set_dir(switchArray[i].ledID, GPIO_OUT);

		// Give everything else sensible defaults.
		switchArray[i].timeStateWasEntered = lastTaskTime;
		switchArray[i].isPressed = false;
	}

	// Give ourselves a power LED indicator for debugging.
	gpio_init(POWER_LED);
    gpio_set_dir(POWER_LED, GPIO_OUT);
	gpio_put(POWER_LED, true);

	while (1)
	{
		// TinyUSB device task.
		tud_task();

		// Blinky blink.
		LEDBlinkingTask();

		// Check all our switches.
		SwitchTask();
		
		// Keep them informed about HID changes.
		hid_task();

		// Track time.
		lastTaskTime = time_us_32();
	}

	return 0;
}
