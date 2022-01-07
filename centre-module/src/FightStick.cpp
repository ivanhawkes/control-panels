#include "FightStick.h"
#include "Components.h"


void CreateTestCases(entt::registry &registry)
{
	for (int16_t i = 0; i < 10; ++i)
	{
		const auto entity = registry.create();
		registry.emplace<Delta2Axis16BitComponent>(entity, int16_t(i * 2), int16_t(i * 3));
		if (i % 2 == 0)
		{
			registry.emplace<Delta3Axis16BitComponent>(entity, int16_t(i * 4), int16_t(i * 5), int16_t(i * 6));
		}
	}
}