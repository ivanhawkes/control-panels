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

The shared state can be in the main system code or available to each system at the least.

Might need some state in the registry, which can be retrieved, then used for a system and updated by the system.

Expansion through I2C and SPI will be required, so we'll need a way to know which bus / component wants is hooked
to which state components.

Bonus points: be able to update a small OLED display.

*/

struct GPIOComponent
{
	// The GPIO pin number which this component represents.
	uint8_t gpioId;
};


struct SwitchComponent
{
	// Is the switch currently depressed?
	bool isPressed{false};
};


struct Bitset32Component
{
	uint32_t state;
};


struct Bitset16Component
{
	uint16_t state;
};


struct Bitset8Component
{
	uint8_t state;
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
	// The key code value that this component represents.
	uint8_t keycode{0};
};


struct KeyModifierComponent
{
	// Modifiers that are being pressed at the same time as a key stroke e.g. CTRL.
	uint8_t modifier;
};


// HINT: Use with a Delta2D8BitComponent (X,Y), Delta1D8BitComponent (mousewheel), Bitset8Component (buttonMask).
struct MouseComponent
{
	// Using AC Pan - what the hell is that?
	// TODO: Remove if not needed.
	int8_t pan;
};


struct Delta1D8BitComponent
{
	int8_t delta;
};


struct Delta2D8BitComponent
{
	// X-axis.
	int8_t deltaX;

	// Y-axis.
	int8_t deltaY;
};


struct Delta3D8BitComponent
{
	// X-axis.
	int8_t deltaX;

	// Y-axis.
	int8_t deltaY;

	// Z-axis.
	int8_t deltaZ;
};


struct Delta1D16BitComponent
{
	int16_t delta;
};


struct Delta2D16BitComponent
{
	// X-axis.
	int16_t deltaX;

	// Y-axis.
	int16_t deltaY;
};


struct Delta3D16BitComponent
{
	// X-axis.
	int16_t deltaX;

	// Y-axis.
	int16_t deltaY;

	// Z-axis.
	int16_t deltaZ;
};


struct TimerUSComponent
{
	// The number of microseconds that have passed.
	uint32_t microseconds;
};


// Toggle like a flip flop.
struct FlipFlopComponent
{
	// Is the flipflop high / low?
	bool isHigh{false};
};


// Simple counting tool.
struct CounterComponent
{
	uint32_t value;
};


struct I2CComponent
{
	// HINT: Use with BusIdComponent and DeviceIdComponent components.
};


struct SPIComponent
{
	// HINT: Use with BusIdComponent and DeviceIdComponent components.
};


struct BusIdComponent
{
	// The unique bus identifier.
	uint8_t busId;
};


struct DeviceIdComponent
{
	// The unique device identifier.
	uint8_t deviceId;
};


struct DirtyComponent
{
	// Indicates some data in the entity has been updated and is now dirty. This data should be flushed by
	// later systems e.g. KeyComponent + Dirty = send keypress report.
};


struct RGBColourComponent
{
	// The level of red.
	uint8_t red;

	// The level of green.
	uint8_t green;

	// The level of blue.
	uint8_t blue;
};


struct LEDStripComponent
{
	// TODO: Support for RGB strip lighting.
};


struct IOExpanderComponent
{
	// HINT: Use with DeviceIdComponent, BusIdComponent or similar, and some state e.g. Bitset32Component.
};
