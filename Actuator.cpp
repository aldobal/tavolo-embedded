#include "Actuator.h"

Actuator::Actuator(int actuatorPin) : pin(actuatorPin) {}

void Actuator::setOnStateChangeCallback(std::function<void(bool)> callback) {
    onStateChangeCallback = callback;
}

void Actuator::notifyStateChange(bool newState) {
    if (currentState != newState) {
        currentState = newState;
        if (onStateChangeCallback) {
            onStateChangeCallback(newState);
        }
    }
}
