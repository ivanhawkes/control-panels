#include "Systems.h"
#include "Components.h"


void Update(entt::registry &registry)
{
	auto view = registry.view<const Delta2Axis16BitComponent, Delta3Axis16BitComponent>();

	// // use a callback
	// view.each([](const auto &pos, auto &vel) { /* ... */ });

	// // use an extended callback
	// view.each([](const auto entity, const auto &pos, auto &vel) { /* ... */ });

	// use a range-for
	for (auto [entity, pos, vel] : view.each())
	{
		// ...
		printf("posX = %d, velX = %d\n", pos.deltaX, vel.deltaX);
	}

	// use forward iterators and get only the components of interest
	for (auto entity : view)
	{
		auto &vel = view.get<Delta3Axis16BitComponent>(entity);
		printf("dx = %d, dy = %d, dz = %d\n", vel.deltaX, vel.deltaY, vel.deltaZ);
	}
}
