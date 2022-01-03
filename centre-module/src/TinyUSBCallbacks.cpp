#include "TinyUSBCallbacks.h"
#include "TinyUSBDescriptors.h"
#include "tusb.h"


// Invoked when device is mounted.
void tud_mount_cb(void)
{
	blinkIntervalMS = blinkIntervalMounted;
	printf("Device was mounted.\n");
}


// Invoked when device is unmounted.
void tud_umount_cb(void)
{
	blinkIntervalMS = blinkIntervalNotMounted;
	printf("Device was unmounted.\n");
}


// Invoked when usb bus is suspended.
// remote_wakeup_en : if host allow us  to perform remote wakeup.
// Within 7ms, device must draw an average of current less than 2.5 mA from bus.
void tud_suspend_cb(bool remote_wakeup_en)
{
	blinkIntervalMS = blinkIntervalSuspended;
	printf("Device was suspended.\n");
}


// Invoked when usb bus is resumed.
void tud_resume_cb(void)
{
	blinkIntervalMS = blinkIntervalMounted;
	printf("Device was resumed.\n");
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
	return 0;
}


// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )

void tud_hid_set_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
	// if (report_type == HID_REPORT_TYPE_OUTPUT)
	// {
	// 	// Set keyboard LED e.g Capslock, Numlock etc...
	// 	if (report_id == REPORT_ID_KEYBOARD)
	// 	{
	// 		// bufsize should be (at least) 1
	// 		if (bufsize < 1)
	// 			return;

	// 		uint8_t const kbd_leds = buffer[0];

	// 		if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
	// 		{
	// 			// Capslock On: disable blink, turn led on
	// 			blinkIntervalMS = 0;
	// 			board_led_write(true);
	// 		}
	// 		else
	// 		{
	// 			// Caplocks Off: back to normal blink
	// 			board_led_write(false);
	// 			blinkIntervalMS = blinkIntervalMounted;
	// 		}
	// 	}
	// }
}
