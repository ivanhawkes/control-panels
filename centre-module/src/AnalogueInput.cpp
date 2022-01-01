#include "AnalogueInput.h"

#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <stdio.h>


void AnalogueInputGroup::Init()
{
	printf("Analogue pins:\n\n");

	// Initialise all the analogue pins.
	for (uint i = 0; i < kPinCount; i++)
	{
		printf("Init PinId: %d - GPIO: %d.\n", i, analogueInputs[i].gpioSwitchId);
		adc_gpio_init(analogueInputs[i].gpioSwitchId);

		// Default the raw input values to the mid-position.
		// NOTE: This might be entirely wrong for a controller like a thrust stick.
		analogueInputs[i] = AnalogueInput::midPointADCValue;
	}
}


bool AnalogueInputGroup::OnTask()
{
	// uint32_t startTaskTime = time_us_32();
	// uint32_t endTaskTime;

	for (size_t i = 0; i < kPinCount; i++)
	{
		if (analogueInputs[i].isEnabled)
		{
			adc_select_input(i);
			analogueInputs[i].value = adc_read();
		}
	}

	// HACK: DEBUG: checking the button state every so often.
	static int count = 0;
	count++;
	if (count > 20000)
	{
		printf("0 = %d, 1 = %d, 2 = %d\n", GetRawValue(0), GetRawValue(1), GetRawValue(2));
		count = 0;

		// endTaskTime = time_us_32();
		// printf("Analogue Duration = %d\n", endTaskTime - startTaskTime);
	}

	return true;
}
