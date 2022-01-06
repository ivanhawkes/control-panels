#pragma once

#include <stdint.h>

/*

Thoughts:

Add in each type of physical component as a digital equivalent and use an ECS to model their state.
Run the systems each loop through e.g. SystemReadSwitches, SystemReadPots, SystemReadRotaryEncoder, etc
Run output systems after the input systems.

Sample entity:

* GPIOComponent
* EnabledComponent
* SwitchComponent
* LEDComponent

This should be able to set the LED state from the Switch state. Both will need to read / write to a common state.

*/

struct GPIOComponent
{
	// The GPIO pin number which the switch is connected to.
	uint32_t gpioLEDId;
};


struct IsEnabledComponent
{
	// True if enabled, false otherwise. Use this to selectively enable entities.
	// NOTE: This might be better in the opposite configuration.
	bool isEnabled{true};
};


struct SwitchComponent
{
	// Is the switch currently depressed?
	bool isPressed{false};
};


// Idea will need refining...and is likely wrong, just here to remind me to hook things up to state data somewhere.
struct SwitchStateComponent
{
	uint32_t state;
};


struct LEDComponent
{
	// Is the switch currently depressed?
	bool isOn{false};
};


struct PotentiometerComponent
{
	// Present value.
	uint32_t value{0};
};


struct RotaryEncoderComponent
{
	// Need to study how these work before I write a class for them.
	uint8_t rotations{0};
};


struct KeyComponent
{
	// The key code value(s) that this component represents.
	uint8_t keycode[6]{0, 0, 0, 0, 0, 0};
};


struct KeyModifierComponent
{
	// Modifiers that are being pressed at the same time as a key stroke e.g. CTRL.
	uint8_t modifier;
};


struct MouseComponent
{
	// Buttons mask for currently pressed buttons on the mouse.
	uint8_t buttonMask;

	// Current delta x movement of the mouse.
	int8_t deltaX;

	// Current delta y movement of the mouse.
	int8_t deltaY;

	// Current delta of the mouse wheel.
	int8_t deltaWheel;

	// Using AC Pan.
	int8_t pan;
};