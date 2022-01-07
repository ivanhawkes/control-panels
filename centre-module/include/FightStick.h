#pragma once

#include <entt/entt.hpp>


// Create some data in the registry which will allow us to test the systems.
void CreateTestCases(entt::registry &registry);

// Add an entity that will control a flashing LED.
void CreateFlashingLED(entt::registry &registry);