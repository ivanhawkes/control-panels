#pragma once

#include <stdint.h>


// Blink pattern times.
enum
{
	blinkIntervalNotMounted = 250,
	blinkIntervalMounted = 1000,
	blinkIntervalSuspended = 2500,
};

static uint32_t blinkIntervalMS{blinkIntervalNotMounted};


void SendGamepadHIDReport(uint8_t report_id);