#include "Systems.h"
#include "Components.h"

#include "bsp/board.h"
#include "pico/stdlib.h"


void Update(entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime)
{
	SystemUpdateTimers(registry, startTaskTime, deltaTime);
	SystemUpdateLED(registry);

	// auto view = registry.view<const Delta2Axis16BitComponent, Delta3Axis16BitComponent>();

	// // // use a callback
	// // view.each([](const auto &pos, auto &vel) { /* ... */ });

	// // // use an extended callback
	// // view.each([](const auto entity, const auto &pos, auto &vel) { /* ... */ });

	// // use a range-for
	// for (auto [entity, pos, vel] : view.each())
	// {
	// 	// ...
	// 	printf("posX = %d, velX = %d\n", pos.deltaX, vel.deltaX);
	// }

	// // use forward iterators and get only the components of interest
	// for (auto entity : view)
	// {
	// 	auto &vel = view.get<Delta3Axis16BitComponent>(entity);
	// 	printf("dx = %d, dy = %d, dz = %d\n", vel.deltaX, vel.deltaY, vel.deltaZ);
	// }
}


void SystemUpdateTimers(entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime)
{
	registry.view<TimerUSComponent>().each([deltaTime](auto entity, auto &timer) { timer.microseconds += deltaTime; });
}


void SystemUpdateLED(entt::registry &registry)
{
	registry.view<GPIOComponent, LEDComponent, TimerUSComponent>().each(
	    [](auto entity, auto &gpio, auto &led, auto &timer) {
		    if (timer.microseconds > 100000)
		    {
			    led.isOn = !led.isOn;
			    timer.microseconds = 0U;
			    gpio_put(gpio.gpioId, led.isOn);
		    }
	    });
}