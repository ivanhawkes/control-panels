#ifndef _DIGITAL_SWITCH_H_
#define _DIGITAL_SWITCH_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#define kDigitalSwitchCount 21


struct DigitalSwitch
{
	// The GPIO pin number which the switch is connected to.
	uint32_t gpioSwitchId;
	
	// If there is an LED associated with this switch it will have a GPIO pin number here.
	uint gpioLedId;
	
	// The key code we will send when activating this switch?
	int mappedKey;
	
	// Friendly name for the switch.
	char mappedKeyName[25];
	
	// Is the switch currently depressed?
	bool isPressed;
	
	// Track the number of microseconds the switch has been in it's current state.
	uint32_t timeStateWasEntered;
};

// Call to initialise.
void InitDigitalSwitches();

// Called each frame to process the switches.
bool DigitalSwitchTask();

// True if one or more switches changed their state during this frame.
bool HasDigitalSwitchStateChanged();

// Get the current state of the digital switches as a bitset.
int GetDigitalSwitchesState();

#endif
