#include <vector>

#include "Components.h"
#include "Systems.h"

#include "bsp/board.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"


// NOTE: Need this to be somewhere generic. It can only be used on ranges of less than
// 1 hour 11 mins since it wraps around.
uint32_t GetTimeDifference(uint32_t start, uint32_t end)
{
	if (start < end)
		return end - start;
	else
		return std::numeric_limits<uint32_t>::max() - (start - end);
}


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

void Update(
    entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime, bool &hasStateChanged,
    uint32_t &digitalSwitches, uint32_t &dpadBitset)
{
	SystemUpdateTimers(registry, startTaskTime, deltaTime);
	SystemUpdateLED(registry);

	// Read values from the inputs.
	SystemReadOnboardDigitalInputs(registry, hasStateChanged, digitalSwitches, dpadBitset);
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

void SystemReadOnboardDigitalInputs(
    entt::registry &registry, bool &hasStateChanged, uint32_t &digitalSwitchBitset, uint32_t &dpadBitset)
{
	// Zero out. We'll add in masks as needed.
	digitalSwitchBitset = 0U;
	dpadBitset = 0U;

	uint32_t currentTime = time_us_32();

	// Get all the GPIO values at once. We flip them now since we're using pull ups, so
	// a zero voltage is actually a button pressed down.
	uint32_t gpioAll = ~gpio_get_all();

	// Mask out the ones we don't want e.g. 0 and 1 for UART, anything above 22.
	// gpioAll &= 0x00FFFFFF;
	gpioAll &= 0x00000FFF;

	registry
	    .view<
	        PicoBoardComponent, GPIODigitalInputComponent, SwitchComponent, ButtonMaskComponent, DPadMaskComponent,
	        TimestampUS32Component>()
	    .each([&gpioAll, &digitalSwitchBitset, &dpadBitset, &hasStateChanged, currentTime](
	              auto entity, auto &picoBoard, auto &gpio, auto &switchComponent, auto &buttonMask, auto &dpadMask,
	              auto &timestamp) {
		    const bool currentSwitchState = gpioAll & (1U << gpio.gpioId);

		    // See if we're still in the debounce period.
		    const bool bouncePeriod = (GetTimeDifference(timestamp.timestamp, currentTime) < 4000U);

		    // State change - make something happen.
		    if ((switchComponent.isPressed != currentSwitchState) && (!bouncePeriod))
		    {
			    // The state has changed for this frame.
			    hasStateChanged = true;

			    // DEBUG: Let us know if the switch changed states.
			    uint32_t theMask;
			    if (buttonMask.mask != 0U)
				    theMask = buttonMask.mask;
			    else
				    theMask = dpadMask.mask;

			    if (currentSwitchState)
				    printf("+%u\n", theMask);
			    else
				    printf("-%u\n", theMask);

			    // Entering a new state, reset the time now.
			    timestamp.timestamp = currentTime;
			    switchComponent.isPressed = currentSwitchState;
		    }

		    // If the switch is on, we should add it to the button state. The state is zeroed out each
		    // loop, so no need to handle off cases.
		    if (currentSwitchState)
		    {
			    digitalSwitchBitset |= buttonMask.mask;
			    dpadBitset |= dpadMask.mask;
		    }
	    });
}


void SystemReadOnboardAnalogueInputs(entt::registry &registry)
{
	// uint32_t startTaskTime = time_us_32();
	// uint32_t endTaskTime;
	static int count{0};
	count++;

	if (count == 20000)
	{
		// printf("Analogue Raw Value: ");
	}

	registry.view<PicoBoardComponent, GPIOAnalogueInputComponent, Analogue16Component>().each(
	    [](auto entity, auto &picoBoard, auto &gpio, auto &value) {
		    adc_select_input(gpio.adcInputChannel);
		    value.value = adc_read();
		    if (count == 20000)
		    {
			    // printf("%d, ", value.value);
		    }
	    });

	// endTaskTime = time_us_32();

	if (count > 20001)
	{
		// printf("\n");
		// printf("Duration ECS: %d\n", endTaskTime - startTaskTime);
		count = 0;
	}
}