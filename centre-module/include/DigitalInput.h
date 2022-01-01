#pragma once

#include "IPicoInput.h"
#include <stdint.h>
#include <stdlib.h>
#include <string>


class DigitalInput
{
  public:
	DigitalInput(uint32_t gpioSwitchId, int mappedKey, std::string mappedKeyName)
	    : gpioSwitchId(gpioSwitchId), mappedKey(mappedKey), mappedKeyName(mappedKeyName){};
	virtual ~DigitalInput(){};

	// The GPIO pin number which the switch is connected to.
	uint32_t gpioSwitchId;

	// The key code we will send when activating this switch?
	int mappedKey;

	// Friendly name for the switch.
	std::string mappedKeyName;

	// Is the switch currently depressed?
	bool isPressed;

	// Track the number of microseconds the switch has been in it's current state.
	uint32_t timeStateWasEntered;
};


class DigitalInputGroup : IPicoInput
{
  public:
	const size_t kDigitalInputCount = 21;

	// Call to initialise.
	virtual void Init() override;

	// Called each frame to process the inputs.
	// Returns true if the state has changed.
	virtual bool OnTask() override;

	// True if our state has changed this frame.
	virtual bool HasStateChanged() override;

	// Get the current state of the digital switches as a bitset.
	uint32_t GetState();

  private:
	// Has a digital switch been pressed this frame?
	bool hasStateChanged = false;

	// A bitmap of the state of all the digital switches on a gamepad.
	uint32_t digitalSwitches = 0;
};