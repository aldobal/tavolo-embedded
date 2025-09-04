#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

/**
 * @brief Base class for IoT devices following SOLID principles
 * 
 * This class provides common functionality for IoT devices including:
 * - Device identification
 * - State management
 * - Event-driven architecture support
 */
class Device {
public:
    enum class DeviceState {
        INITIALIZING,
        READY,
        ACTIVE,
        ERROR,
        MAINTENANCE
    };

protected:
    String deviceMacAddress;
    String deviceId;
    DeviceState currentState;
    std::function<void(DeviceState, DeviceState)> onStateChangeCallback = nullptr;

public:
    Device();
    virtual ~Device() = default;

    // Core device interface
    virtual void setup();
    virtual void loop();

    // State management
    DeviceState getState() const { return currentState; }
    String getStateString() const;
    void setState(DeviceState newState);

    // Device identification
    String getDeviceMacAddress() const { return deviceMacAddress; }
    String getDeviceId() const { return deviceId; }

    // Event-driven programming support
    void setOnStateChangeCallback(std::function<void(DeviceState, DeviceState)> callback);

protected:
    void notifyStateChange(DeviceState oldState, DeviceState newState);
};

#endif // DEVICE_H
