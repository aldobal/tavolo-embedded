#include "Device.h"

Device::Device() : currentState(DeviceState::INITIALIZING) {
    deviceMacAddress = WiFi.macAddress();
    deviceId = "TAVOLO_" + deviceMacAddress;
    deviceId.replace(":", "");
}

void Device::setup() {
    Serial.println("=== Tavolo Smart Weight Detection System ===");
    Serial.print("Device ID: ");
    Serial.println(deviceId);
    Serial.print("MAC Address: ");
    Serial.println(deviceMacAddress);
    Serial.println("==========================================");
    
    setState(DeviceState::READY);
}

void Device::loop() {
    // Base implementation - can be overridden
}

String Device::getStateString() const {
    switch (currentState) {
        case DeviceState::INITIALIZING: return "INITIALIZING";
        case DeviceState::READY: return "READY";
        case DeviceState::ACTIVE: return "ACTIVE";
        case DeviceState::ERROR: return "ERROR";
        case DeviceState::MAINTENANCE: return "MAINTENANCE";
        default: return "UNKNOWN";
    }
}

void Device::setState(DeviceState newState) {
    DeviceState oldState = currentState;
    if (oldState != newState) {
        currentState = newState;
        notifyStateChange(oldState, newState);
    }
}

void Device::setOnStateChangeCallback(std::function<void(DeviceState, DeviceState)> callback) {
    onStateChangeCallback = callback;
}

void Device::notifyStateChange(DeviceState oldState, DeviceState newState) {
    Serial.print("Device state changed: ");
    Serial.print(getStateString());
    Serial.print(" -> ");
    
    // Temporarily change state to get new state string
    DeviceState temp = currentState;
    currentState = newState;
    Serial.println(getStateString());
    currentState = temp;
    
    if (onStateChangeCallback) {
        onStateChangeCallback(oldState, newState);
    }
}
