#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <entt/entt.hpp>

#include "TinyUSBCallbacks.h"
#include "TinyUSBDescriptors.h"
#include "tusb.h"

#include "bsp/board.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "AnalogueInput.h"
#include "DigitalInput.h"

#include "FightStick.h"

#include "Components.h"
#include "Systems.h"


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
				report.hat = 1;
				break;
			case 2:
				report.hat = 3;
				break;
			case 4:
				report.hat = 5;
				break;
			case 8:
				report.hat = 7;
				break;

				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_UPLEFT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT: report.hat = HID_HAT_DOWNLEFT; break;
				// case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_UPRIGHT; break;
				// case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: report.hat = HID_HAT_DOWNRIGHT; break;

			default:
				report.hat = GAMEPAD_HAT_CENTERED;
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

	printf("Initialisation complete.\n");

	// Load up some components so we can test this thing.
	CreateTestCases(registry);

	startTaskTime = time_us_32();

	while (true)
	{
		// We'll track the time from startup.
		startTaskTime = time_us_32();

		// TinyUSB device task.
		tud_task();

		// Test the ECS.
		Update(registry, startTaskTime, deltaTime);

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
