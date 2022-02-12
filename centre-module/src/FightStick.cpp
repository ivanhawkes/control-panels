#include "FightStick.h"
#include "Components.h"


void CreateTestCases(entt::registry &registry)
{
	CreateFlashingLED(registry);
	CreateDigitalInput(registry);
	CreateAnalogueInput(registry);


	// for (int16_t i = 0; i < 10; ++i)
	// {
	// 	const auto entity = registry.create();
	// 	registry.emplace<Delta2Axis16BitComponent>(entity, int16_t(i * 2), int16_t(i * 3));
	// 	if (i % 2 == 0)
	// 	{
	// 		registry.emplace<Delta3Axis16BitComponent>(entity, int16_t(i * 4), int16_t(i * 5), int16_t(i * 6));
	// 	}
	// }
}

void CreateFlashingLED(entt::registry &registry)
{
	const auto entity = registry.create();
	registry.emplace<GPIODigitalOuputComponent>(entity, uint8_t(25U));
	registry.emplace<LEDComponent>(entity, false);
	registry.emplace<TimerUSComponent>(entity, 100000U, 0U);
}


void CreateDigitalInput(entt::registry &registry)
{
	for (int i = 0; i < 16; ++i)
	{
		const auto entity = registry.create();
		registry.emplace<GPIODigitalInputComponent>(entity, uint8_t(i));
		registry.emplace<SwitchComponent>(entity, false);
	}

	for (int i = 18; i < 21; ++i)
	{
		const auto entity = registry.create();
		registry.emplace<GPIODigitalInputComponent>(entity, uint8_t(i));
		registry.emplace<SwitchComponent>(entity, false);
	}
}


void CreateAnalogueInput(entt::registry &registry)
{
	for (int i = 26; i < 30; ++i)
	{
		const auto entity = registry.create();
		registry.emplace<GPIOAnalogueInputComponent>(entity, uint8_t(i));
	}
}
