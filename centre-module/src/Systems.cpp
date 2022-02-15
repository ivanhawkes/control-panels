#include <vector>

#include "Components.h"
#include "Systems.h"

#include "bsp/board.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

//
// INIT
//

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

//
// UPDATE
//

void Update(entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime)
{
	uint32_t valueBitset{0};

	SystemUpdateTimers(registry, startTaskTime, deltaTime);
	SystemUpdateLED(registry);

	// Read values from the inputs.
	SystemReadOnboardDigitalInputs(registry, valueBitset);
	SystemReadOnboardAnalogueInputs(registry);

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

//
// READ
//

void SystemReadOnboardDigitalInputs(entt::registry &registry, uint32_t &valueBitset)
{
	// Should I read all the onboard in one call?
	// How should this be passed out? Does this even make sense doing one at a time, or all at once?
	// Some might be set to output, what happens then?

	// IDEA: Read in one go, use the GPIO pins to re-order it for the Buttons collection output.
	// Against - .
	// For - fast.
}


void SystemReadOnboardAnalogueInputs(entt::registry &registry)
{
	uint32_t startTaskTime = time_us_32();
	uint32_t endTaskTime;
	uint32_t endTaskTime2;
	uint32_t endTaskTime3;
	uint32_t endTaskTime4;
	static int count = 0;
	count++;
	std::vector<GPIOAnalogueInputComponent> gpioCollection{{26, 0}, {27, 1}, {28, 2}, {29, 3}};

	if (count == 20000)
	{
		printf("Analogue Raw Value: ");
	}

	registry.view<PicoBoardComponent, GPIOAnalogueInputComponent, Analogue16Component>().each(
	    [](auto entity, auto &picoBoard, auto &gpio, auto &value) {
		    adc_select_input(gpio.adcInputChannel);
		    value.value = adc_read();
		    if (count == 20000)
		    {
			    printf("%d, ", value.value);
		    }
	    });

	endTaskTime = time_us_32();

	registry.view<PicoBoardComponent, GPIOAnalogueInputComponent, Analogue16Component>().each(
	    [](auto entity, auto &picoBoard, auto &gpio, auto &value) {
		    // int a{1};
		    // a++;
	    });

	endTaskTime2 = time_us_32();

	for (auto &gpio : gpioCollection)
	{
		adc_select_input(gpio.adcInputChannel);
		auto value = adc_read();
		value++;
	}

	endTaskTime3 = time_us_32();

	for (auto &gpio : gpioCollection)
	{
		auto value = gpio.adcInputChannel;
		value++;
	}

	endTaskTime4 = time_us_32();

	if (count > 20001)
	{
		printf("\n");
		printf("Duration ECS              = %d\n", endTaskTime - startTaskTime);
		printf("Duration Empty ECS        = %d\n", endTaskTime2 - endTaskTime);
		printf("Duration Collection       = %d\n", endTaskTime3 - endTaskTime2);
		printf("Duration Empty Collection = %d\n", endTaskTime4 - endTaskTime3);
		count = 0;
	}
}