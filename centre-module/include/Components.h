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

// TODO: Have the input / output components initialise their pins when added to an entity or during
// an init routine.

struct GPIODigitalInputComponent
{
	// The GPIO pin number which this component represents.
	uint8_t gpioId{0};
};


struct GPIODigitalOuputComponent
{
	// The GPIO pin number which this component represents.
	uint8_t gpioId{0};
};


struct GPIOAnalogueInputComponent
{
	// The GPIO pin number which this component represents.
	uint8_t gpioId{0};

	// Different to the GPIO pin, this is the channel ID for the SDK. Ranges 0-4 are excepted.
	// 0-3 is the standard ADC pins. 4 is the temperature sensor.
	uint16_t adcInputChannel{0};
};


struct GPIOAnalogueOutputComponent
{
	// The GPIO pin number which this component represents.
	uint8_t gpioId{0};
};


// Flags an entity as affecting the in-built Pico board GPIO.
struct PicoBoardComponent
{
	// HINT: Use with BusIdComponent and DeviceIdComponent components.
	uint8_t emptyStruct{0};
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
	uint8_t busId{0};
};


struct DeviceIdComponent
{
	// The unique device identifier.
	uint8_t deviceId{0};
};


struct SwitchComponent
{
	// Is the switch currently depressed?
	bool isPressed{false};
};


struct ButtonMaskComponent
{
	// Use to set button bits on for the USB report.
	uint32_t mask{0};
};


struct Bitset32Component
{
	uint32_t state{0};
};


struct Bitset16Component
{
	uint16_t state{0};
};


struct Bitset8Component
{
	uint8_t state{0};
};


struct Analogue32Component
{
	int32_t value{0};
};


struct Analogue16Component
{
	int16_t value{0};
};


struct Analogue8Component
{
	int8_t value{0};
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
	uint8_t modifier{0};
};


// HINT: Use with a Delta2Axis8BitComponent (X,Y), Delta1Axis8BitComponent (mousewheel), Bitset8Component (buttonMask).
struct MouseComponent
{
	// Using AC Pan - what the hell is that?
	// TODO: Remove if not needed.
	int8_t pan{0};
};


struct Delta1Axis8BitComponent
{
	int8_t delta{0};
};


struct Delta2Axis8BitComponent
{
	// X-axis.
	int8_t deltaX{0};

	// Y-axis.
	int8_t deltaY{0};
};


struct Delta3Axis8BitComponent
{
	// X-axis.
	int8_t deltaX{0};

	// Y-axis.
	int8_t deltaY{0};

	// Z-axis.
	int8_t deltaZ{0};
};


struct Delta1Axis16BitComponent
{
	int16_t delta{0};
};


struct Delta2Axis16BitComponent
{
	// X-axis.
	int16_t deltaX{0};

	// Y-axis.
	int16_t deltaY{0};
};


struct Delta3Axis16BitComponent
{
	// X-axis.
	int16_t deltaX{0};

	// Y-axis.
	int16_t deltaY{0};

	// Z-axis.
	int16_t deltaZ{0};
};


struct TimerUSComponent
{
	// The duration that must pass before the timer is activated.
	uint32_t duration{0};

	// The number of microseconds that have passed since the timer was started.
	uint32_t elapsedMS{0};
};


struct TimestampUS32Component
{
	// The timestamp in microseconds.
	uint32_t timestamp{0};
};


struct TimestampUS64Component
{
	// The full timestamp in microseconds.
	uint64_t timestamp{0};
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
