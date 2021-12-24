#include "digital_switch.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/time.h"


// Has a digital switch been pressed this frame?
bool g_hasDigitalSwitchStateChanged = false;

// This will contain the switch value if one was pressed. It contains rubbish at other times.
uint8_t digitalSwitchPressed = HID_KEY_NONE;

// A bitmap of the state of all the digital switches on a gamepad.
int g_digitalSwitches = 0;


struct DigitalSwitch switchArray[] =
{
    // Joystick.
	{2, 0, GAMEPAD_BUTTON_5, "Joy Up", false, 0},			// Up - HACK: Should be GAMEPAD_HAT_UP
    {3, 0, GAMEPAD_BUTTON_6, "Joy Down", false, 0},			// Down - HACK: Should be GAMEPAD_HAT_DOWN
    {4, 0, GAMEPAD_BUTTON_7, "Joy Right", false, 0},		// Right -HACK: Should be  GAMEPAD_HAT_RIGHT
    {5, 0, GAMEPAD_BUTTON_8, "Joy Left", false, 0},			// Left - HACK: Should be GAMEPAD_HAT_LEFT

    // Lower row of top panel buttons (left to right).
	{6, 0, GAMEPAD_BUTTON_SOUTH, "B1", false, 0},			// 1k, B1, A, Circle
    {7, 0, GAMEPAD_BUTTON_EAST, "B2", false, 0},			// 2k, B2, B, Cross
    {8, 0, GAMEPAD_BUTTON_9, "R2", false, 0},				// 3k, R2, RT
    {9, 0, GAMEPAD_BUTTON_10, "L2", false, 0},				// 4k, L2, LT

    // Upper row of top panel buttons (left to right).
    {10, 0, GAMEPAD_BUTTON_WEST, "B3", false, 0},			// 1p, B3, X, Triangle
    {11, 0, GAMEPAD_BUTTON_NORTH, "B4", false, 0},			// 2p, B4, Y, Square
    {12, 0, GAMEPAD_BUTTON_11, "R1", false, 0},				// 3p, R1, RB
    {13, 0, GAMEPAD_BUTTON_12, "L1", false, 0},				// 4p, L1, LB

    // Left panel buttons.
    {14, 0, GAMEPAD_BUTTON_13, "-14-", false, 0},			// Used for LED I think.
    {15, 0, GAMEPAD_BUTTON_14, "-15-", false, 0},			// Used for LED I think.

    // Rear panel buttons.
    {16, 0, GAMEPAD_BUTTON_SELECT, "S1", false, 0},			// Select, S1, Back
    {17, 0, GAMEPAD_BUTTON_START, "S2", false, 0},			// Start, S2, Start

    // Right panel buttons.
    {18, 0, GAMEPAD_BUTTON_15, "L3", false, 0},				// LS, L3, LS
    {19, 0, GAMEPAD_BUTTON_16, "R3", false, 0},				// RS, R3, RS

    // Front panel buttons (left to right).
    {20, 0, GAMEPAD_BUTTON_17, "A1", false, 0},				// Home, A1, XBOX
    {21, 0, GAMEPAD_BUTTON_18, "A2", false, 0},				// TP, A2, -
    // {22, 0, GAMEPAD_BUTTON_8, "xxx", false, 0},			// Unassigned
    // {23, 0, GAMEPAD_BUTTON_8, "xxx", false, 0},			// Unassigned

	// Top panel. Extra
    {22, 0, GAMEPAD_BUTTON_19, "Insert Coin", false, 0},	// Insert coin
};


void InitDigitalSwitches()
{
    //uint32_t initTime = time_us_32();

	// Initialise all the switches.
	for (size_t i = 0; i < kDigitalSwitchCount; i++)
	{
		// Initialise the switch pins for input.
		gpio_init(switchArray[i].gpioSwitchId);
		gpio_set_dir(switchArray[i].gpioSwitchId, GPIO_IN);
		gpio_pull_up(switchArray[i].gpioSwitchId);

		// Initialise the LED pins for output.
		if (switchArray[i].gpioLedId)
		{
			gpio_init(switchArray[i].gpioLedId);
			gpio_set_dir(switchArray[i].gpioLedId, GPIO_OUT);
		}

		// Give everything else sensible defaults.
		//switchArray[i].timeStateWasEntered = initTime;
		switchArray[i].isPressed = false;
	}
}


bool DigitalSwitchTask()
{
	uint32_t currentTime = time_us_32();

	// Default is for nothing to happen.
	g_hasDigitalSwitchStateChanged = false;

	// Detect their state.
	for (size_t i = 0; i < kDigitalSwitchCount; i++)
	{
		// Get their pressed state.
		const bool currentSwitchState = gpio_get(switchArray[i].gpioSwitchId);

		// State change - make something happen.
		if (switchArray[i].isPressed != currentSwitchState)
		{
			// The state has changed for this frame.
			g_hasDigitalSwitchStateChanged = true;
			digitalSwitchPressed = switchArray[i].mappedKey;

			// uint32_t timeInThisState = currentTime - switchArray[i].timeStateWasEntered;

			// DEBUG: Testing the time difference on loops.
			// printf("\nLT: %u, CT: %u, TSE: %u, TiS: %u, ",
			// 	lastTaskTime,
			// 	currentTime,
			// 	switchArray[i].timeStateWasEntered,
			// 	timeInThisState);

			// Switch went on or off?
			if (currentSwitchState)
			{
				g_digitalSwitches &= ~switchArray[i].mappedKey;
				if (switchArray[i].gpioLedId)
				{
					gpio_put(switchArray[i].gpioLedId, false);
				}

				printf("-%s  CT: %u\n", switchArray[i].mappedKeyName, currentTime);
			}
			else
			{
				g_digitalSwitches |= switchArray[i].mappedKey;
				
				if (switchArray[i].gpioLedId)
				{
					gpio_put(switchArray[i].gpioLedId, true);
				}

				printf("+%s CT: %u\n", switchArray[i].mappedKeyName, currentTime);
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

    return g_hasDigitalSwitchStateChanged;
}


bool HasDigitalSwitchStateChanged()
{
    return g_hasDigitalSwitchStateChanged;
}


int GetDigitalSwitchesState()
{
    return g_digitalSwitches;
} 
