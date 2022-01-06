#include "DigitalInput.h"

#include "GamepadState.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"


class DigitalInput switchArray[]{
    // Joystick.
    {0, GAMEPAD_MASK_UP, "Joy Up"},       // Up
    {1, GAMEPAD_MASK_DOWN, "Joy Down"},   // Down
    {2, GAMEPAD_MASK_RIGHT, "Joy Right"}, // Right
    {3, GAMEPAD_MASK_LEFT, "Joy Left"},   // Left

    // Lower row of top panel buttons (left to right).
    {4, GAMEPAD_BUTTON_SOUTH, "B1"}, // 1k, B1, A, Circle
    {5, GAMEPAD_BUTTON_EAST, "B2"},  // 2k, B2, B, Cross
    {6, GAMEPAD_BUTTON_9, "R2"},     // 3k, R2, RT
    {7, GAMEPAD_BUTTON_10, "L2"},    // 4k, L2, LT

    // Upper row of top panel buttons (left to right).
    {8, GAMEPAD_BUTTON_WEST, "B3"},  // 1p, B3, X, Triangle
    {9, GAMEPAD_BUTTON_NORTH, "B4"}, // 2p, B4, Y, Square
    {10, GAMEPAD_BUTTON_11, "R1"},   // 3p, R1, RB
    {11, GAMEPAD_BUTTON_12, "L1"},   // 4p, L1, LB

    // Left panel buttons.
    {12, GAMEPAD_BUTTON_13, "-14-"}, // Used for LED I think.
    {13, GAMEPAD_BUTTON_14, "-15-"}, // Used for LED I think.

    // Rear panel buttons.
    // {14, GAMEPAD_BUTTON_SELECT, "S1"}, // Select, S1, Back
    // {15, GAMEPAD_BUTTON_START, "S2"},  // Start, S2, Start

    // Right panel buttons.
    // {16, GAMEPAD_BUTTON_15, "L3"}, // LS, L3, LS
    // {17, GAMEPAD_BUTTON_16, "R3"}, // RS, R3, RS

    // Front panel buttons (left to right).
    {18, GAMEPAD_BUTTON_17, "A1"}, // Home, A1, XBOX
    {19, GAMEPAD_BUTTON_18, "A2"}, // TP, A2, -

    // Top panel. Extra
    {20, GAMEPAD_BUTTON_19, "Insert Coin"}, // Insert coin
};


void DigitalInputGroup::Init()
{
	// uint32_t initTime = time_us_32();

	printf("Digital pins:\n\n");

	// Initialise all the switches.
	for (size_t i = 0; i < kDigitalInputCount; i++)
	{
		printf("Init PinId: %d - GPIO: %d.\n", i, switchArray[i].gpioSwitchId);

		// Initialise the switch pins for input.
		gpio_init(switchArray[i].gpioSwitchId);
		gpio_set_dir(switchArray[i].gpioSwitchId, GPIO_IN);
		gpio_pull_up(switchArray[i].gpioSwitchId);

		// Give everything else sensible defaults.
		// switchArray[i].timeStateWasEntered = initTime;
		switchArray[i].isPressed = false;
	}

	printf("\n");
}


bool DigitalInputGroup::OnTask()
{
	uint32_t currentTime = time_us_32();
	// uint32_t startTaskTime = time_us_32();
	// uint32_t endTaskTime;

	// Default is for nothing to happen.
	hasStateChanged = false;

	// Get all the GPIO values at once. Mask out the ones we don't want e.g. 0 and 1 for UART, anything above 22.
	uint32_t gpioAll = gpio_get_all();
	gpioAll &= 0x00FFFFFF;

	// Detect their state.
	for (size_t i = 0; i < kDigitalInputCount; i++)
	{
		// Get their pressed state.
		const bool currentSwitchState = gpioAll & (1U << (switchArray[i].gpioSwitchId));

		// State change - make something happen.
		if (switchArray[i].isPressed != currentSwitchState)
		{
			// The state has changed for this frame.
			hasStateChanged = true;

			// Switch went on or off?
			if (currentSwitchState)
			{
				digitalSwitches &= ~switchArray[i].mappedKey;
				printf("-%s  CT: %u\n", switchArray[i].mappedKeyName.c_str(), currentTime);
			}
			else
			{
				digitalSwitches |= switchArray[i].mappedKey;
				printf("+%s CT: %u\n", switchArray[i].mappedKeyName.c_str(), currentTime);
			}

			// Entering a new state, reset the time now.
			switchArray[i].timeStateWasEntered = currentTime;
			switchArray[i].isPressed = currentSwitchState;
		}
	}

	// HACK: DEBUG: checking the button state every so often.
	static int count = 0;
	count++;
	if (count > 50000)
	{
		// printf("digitial = %X\n", gpioAll);
		count = 0;
		// endTaskTime = time_us_32();
		// printf("Digital Duration = %d\n", endTaskTime - startTaskTime);
	}

	return hasStateChanged;
}


bool DigitalInputGroup::HasStateChanged()
{
	return hasStateChanged;
}


uint32_t DigitalInputGroup::GetState()
{
	return digitalSwitches;
}