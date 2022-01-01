#pragma once

#include "IPicoInput.h"
#include <stdint.h>
#include <vector>


class AnalogueInput
{
  public:
	AnalogueInput(uint32_t gpioSwitchId) : gpioSwitchId(gpioSwitchId){};
	virtual ~AnalogueInput(){};

	// Minimum value returned by the ADC.
	const static int16_t minADCValue{0};

	// Maximum value returned by the ADC.
	const static int16_t maxADCValue{4096};

	// Mid-point value.
	const static int16_t midPointADCValue{(minADCValue + maxADCValue) / 2};

	// The GPIO pin number which the switch is connected to.
	uint32_t gpioSwitchId;

	// Convert the raw value to something useful to XInput.
	int8_t GetXBoxValue() const
	{
		return static_cast<int8_t>((value - midPointADCValue) >> 4);
	};

	// Value measured at the ADC input.
	int16_t value{midPointADCValue};

	// Is this input enabled for use? If true, poll it, otherwise it contains junk values.
	bool isEnabled{true};
};


class AnalogueInputGroup : IPicoInput
{
  public:
	// The number of analogue pins available for use.
	const static size_t kPinCount{3};

	// Call to initialise.
	virtual void Init() override;

	// Called each frame to process the inputs.
	// Returns true if the state has changed.
	virtual bool OnTask() override;

	// True if our state has changed this frame.
	virtual bool HasStateChanged() override
	{
		return true;
	};

	// Get the raw value read from the analogue pin. Using 0-3 as pin IDs.
	int16_t GetRawValue(size_t pinID) const
	{
		return analogueInputs[pinID].value;
	};

	// Convert the raw value to something useful to XInput. Using 0-3 as pin IDs.
	int8_t GetXBox(size_t pinID) const
	{
		return analogueInputs[pinID].GetXBoxValue();
	};

  private:
	// Has a digital switch been pressed this frame?
	bool hasStateChanged = false;

	// Private store of the raw values from the inputs.
	AnalogueInput analogueInputs[kPinCount]{{26}, {27}, {29}};
};