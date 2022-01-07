#pragma once

#include <entt/entt.hpp>


void Update(entt::registry &registry);

void SystemReadDigitalInputs(entt::registry &registry);

void SystemReadAnalogueInputs(entt::registry &registry);

void SystemUSBReportHID(entt::registry &registry);

void SystemUSBReportXInput(entt::registry &registry);

void SystemUSBReportKeyboard(entt::registry &registry);

void SystemUSBReportMouse(entt::registry &registry);

void SystemUpdateLED(entt::registry &registry);
