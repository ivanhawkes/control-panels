#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <entt/entt.hpp>

#include "TinyUSBCallbacks.h"
#include "TinyUSBDescriptors.h"
#include "tusb.h"

#include "GamepadState.h"
#include "bsp/board.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "AnalogueInput.h"
#include "DigitalInput.h"

#include "FightStick.h"

#include "Components.h"
#include "Systems.h"

//#include <toml.h>


static DigitalInputGroup g_digitalInputGroup;
static AnalogueInputGroup g_analogueSwitchGroup;
uint32_t g_digitalSwitchBitset;
uint32_t g_dpadBitset;
bool g_hasStateChanged{false};


void SendGamepadHIDReport(uint8_t report_id)
{
	if (g_hasStateChanged)
	{
		// Skip if hid is not ready yet.
		// TODO: Is this a way we can use to find out the right timing?
		// NOTE: This will never be ready if the USB device craps out, which it does a lot.
		// unplug and replug the device to reset it or you'll be working with stale binaries.
		if (!tud_hid_ready())
		{
			// printf("HID not ready.\n");
			return;
		}

		hid_gamepad_report_t report = {
		    .x = static_cast<int8_t>(g_analogueSwitchGroup.GetXBox(0)),
		    .y = static_cast<int8_t>(g_analogueSwitchGroup.GetXBox(1)),
		    .z = 0,
		    .rz = 0,
		    .rx = 0,
		    .ry = 0,
		    .hat = 0,
		    .buttons = 0};

		// Digital buttons.
		report.buttons = g_digitalSwitchBitset;

		// Hat / Dpad.
		switch (g_dpadBitset & 0x0F)
		{
			// TODO: Joystick wiring doesn't match the way the GAMEPAD_MASK_* expects it to be. Need to swap
			// up and down. Already swapped left and right. This would make it incompatible with the way
			// PFB does it, I think. Maybe should add my own masks...might not matter since this can be
			// refactored long term.
			case GAMEPAD_MASK_UP:
				report.hat = GAMEPAD_HAT_UP;
				break;
			case GAMEPAD_MASK_DOWN:
				report.hat = GAMEPAD_HAT_DOWN;
				break;
			case GAMEPAD_MASK_LEFT:
				report.hat = GAMEPAD_HAT_LEFT;
				break;
			case GAMEPAD_MASK_RIGHT:
				report.hat = GAMEPAD_HAT_RIGHT;
				break;

				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_UPLEFT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_DOWNLEFT; break;
				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_UPRIGHT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_DOWNRIGHT; break;

			default:
				report.hat = GAMEPAD_HAT_CENTERED;
				break;
		}

		// NOTE: ILH: WTF? Is this the wrong report for this information? Look into it.
		tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

		// Our reported state should now be up to date with our present state.
		g_hasStateChanged = false;
	}
}


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


int main(void)
{
	uint32_t startTaskTime;
	uint32_t endTaskTime;
	uint32_t deltaTime{0};

	// Our registry used for all the ECS operations.
	entt::registry registry;

	// Init the USB / UART IO.
	stdio_init_all();
	adc_init();
	board_init();
	tusb_init();

	// Init the uart using our preferred settings.
	stdio_uart_init_full(uart0, 115200, 16, 17);

	printf("Centre console online.\n\n");

	// Init our input handlers.
	g_digitalInputGroup.Init();
	g_analogueSwitchGroup.Init();

	printf("\n\nInitialisation complete.\n");

	// Load up some components so we can test this thing.
	CreateTestCases(registry);

	// Perform any initalisation required based on the registry. This will set pin input / output, directions, etc.
	SystemInit(registry);

	startTaskTime = time_us_32();

	// Literally only doing this to shut up a compiler warning about this variable not being used.
	blinkIntervalMS = blinkIntervalMounted;

	while (true)
	{
		// We'll track the time from startup.
		startTaskTime = time_us_32();

		// TinyUSB device task.
		tud_task();

		// Test the ECS.
		Update(registry, startTaskTime, deltaTime, g_hasStateChanged, g_digitalSwitchBitset, g_dpadBitset);

		// Check all our switches.
		if (g_digitalInputGroup.OnTask()) {}

		// Check the analogue inputs.
		if (g_analogueSwitchGroup.OnTask()) {}

		// Keep them informed about HID changes.
		SendHIDTask();

		// Track time.
		endTaskTime = time_us_32();
		deltaTime = endTaskTime - startTaskTime;

		// printf("start = %u, end = %u, delta = %u\n", startTaskTime, endTaskTime, deltaTime);
	}

	return 0;
}
