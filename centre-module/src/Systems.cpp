#include "Systems.h"
#include "Components.h"

#include "bsp/board.h"
#include "hardware/adc.h"
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
	registry.view<TimerUSComponent>().each([deltaTime](auto entity, auto &timer) { timer.elapsedMS += deltaTime; });
}


void SystemUpdateLED(entt::registry &registry)
{
	registry.view<GPIODigitalOuputComponent, LEDComponent, TimerUSComponent>().each(
	    [](auto entity, auto &gpio, auto &led, auto &timer) {
		    if (timer.elapsedMS > timer.duration)
		    {
			    led.isOn = !led.isOn;
			    timer.elapsedMS = 0U;
			    gpio_put(gpio.gpioId, led.isOn);
		    }
	    });
}


void SystemInit(entt::registry &registry)
{
	SystemInitOnboardDigitalInput(registry);
	SystemInitOnboardDigitalOutput(registry);
	SystemInitOnboardAnalogueInput(registry);
}


void SystemInitOnboardDigitalInput(entt::registry &registry)
{
	printf("\n\nECS - Digital pins input:\n\n");

	registry.view<GPIODigitalInputComponent>().each([](auto entity, auto &gpio) {
		gpio_init(gpio.gpioId);
		gpio_set_dir(gpio.gpioId, GPIO_IN);
		gpio_pull_up(gpio.gpioId);

		printf("Init PinId: %d.\n", gpio);

		// Give everything else sensible defaults.
		// switchArray[i].timeStateWasEntered = initTime;
		// switchArray[i].isPressed = false;
	});
}


void SystemInitOnboardDigitalOutput(entt::registry &registry)
{
	printf("\n\nECS - Digital pins out:\n\n");

	registry.view<GPIODigitalOuputComponent>().each([](auto entity, auto &gpio) {
		gpio_init(gpio.gpioId);
		gpio_set_dir(gpio.gpioId, GPIO_OUT);

		printf("Init PinId: %d.\n", gpio);
	});
}


void SystemInitOnboardAnalogueInput(entt::registry &registry)
{
	printf("\n\nECS - Analogue pins:\n\n");

	registry.view<GPIOAnalogueInputComponent>().each([](auto entity, auto &gpio) {
		adc_gpio_init(gpio.gpioId);

		printf("Init PinId: %d.\n", gpio);

		// Default the raw input values to the mid-position.
		// NOTE: This might be entirely wrong for a controller like a thrust stick.
		// NOTE: Might also just be a bad idea for init.
	});
}


void SystemReadOnboardDigitalInputs(entt::registry &registry, uint32_t &valueBitset)
{
	// Should I read all the onboard in one call?
	// How should this be passed out? Does this even make sense doing one at a time, or all at once?
	// Some might be set to output, what happens then?

	// IDEA: Read in one go, use the GPIO pins to re-order it for the Buttons collection output.
	// Against - .
	// For - fast.
}