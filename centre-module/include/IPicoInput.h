#pragma once

class IPicoInput
{
  public:
	// Call to initialise.
	virtual void Init() = 0;

	// Called each frame to process the inputs.
	// Returns true if the state has changed.
	virtual bool OnTask() = 0;

	// True if our state has changed this frame.
	virtual bool HasStateChanged() = 0;
};