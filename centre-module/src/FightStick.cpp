#include "FightStick.h"
#include "Components.h"


void CreateTestCases(entt::registry &registry)
{
	CreateFlashingLED(registry);

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
	registry.emplace<GPIOComponent>(entity, uint8_t(25U));
	registry.emplace<LEDComponent>(entity, false);
	registry.emplace<TimerUSComponent>(entity, 0U);
}