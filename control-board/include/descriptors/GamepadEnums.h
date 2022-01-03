#pragma once

// The available input modes
enum InputMode
{
	INPUT_MODE_XINPUT,
	INPUT_MODE_SWITCH,
	INPUT_MODE_HID,
	INPUT_MODE_CONFIG = 255,
};

// The available stick emulation modes
enum DpadMode
{
	DPAD_MODE_DIGITAL,
	DPAD_MODE_LEFT_ANALOG,
	DPAD_MODE_RIGHT_ANALOG,
};

// The available SOCD cleaning methods
enum SOCDMode
{
	SOCD_MODE_UP_PRIORITY,           // U+D=U, L+R=N
	SOCD_MODE_NEUTRAL,               // U+D=N, L+R=N
	SOCD_MODE_SECOND_INPUT_PRIORITY, // U>D=D, L>R=R (Last Input Priority, aka Last Win)
};

// Enum for tracking last direction state of Second Input SOCD method
enum DpadDirection
{
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT
};

// The available hotkey actions
enum GamepadHotkey
{
	HOTKEY_NONE = 0x00,
	HOTKEY_DPAD_DIGITAL = 0x01,
	HOTKEY_DPAD_LEFT_ANALOG = 0x02,
	HOTKEY_DPAD_RIGHT_ANALOG = 0x04,
	HOTKEY_HOME_BUTTON = 0x08,
	HOTKEY_CAPTURE_BUTTON = 0x10,
	HOTKEY_SOCD_UP_PRIORITY = 0x20,
	HOTKEY_SOCD_NEUTRAL = 0x40,
	HOTKEY_SOCD_LAST_INPUT = 0x80,
};
