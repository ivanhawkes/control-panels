#pragma once

#include <entt/entt.hpp>


// Initialise the onboard GPIOs and any connected devices.

void SystemInit(entt::registry &registry);

void SystemInitOnboardDigitalInput(entt::registry &registry);

void SystemInitOnboardDigitalOutput(entt::registry &registry);

void SystemInitOnboardAnalogueInput(entt::registry &registry);


// Update.

void Update(entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime);

void SystemUpdateTimers(entt::registry &registry, uint32_t startTaskTime, uint32_t deltaTime);

uint32_t SystemReadOnboardDigitalInputs(entt::registry &registry);

void SystemReadOnboardAnalogueInputs(entt::registry &registry);

// Reports.

void SystemUSBReportHID(entt::registry &registry);

void SystemUSBReportXInput(entt::registry &registry);

void SystemUSBReportKeyboard(entt::registry &registry);

void SystemUSBReportMouse(entt::registry &registry);

void SystemUpdateLED(entt::registry &registry);
