#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>
#include <functional>

/**
 * @brief Abstract base class for all actuators following the Interface Segregation Principle
 * 
 * This class defines the contract that all actuators must implement.
 * It follows the Single Responsibility Principle by focusing only on actuator operations.
 */
class Actuator {
protected:
    int pin;
    bool initialized = false;
    bool currentState = false;
    std::function<void(bool)> onStateChangeCallback = nullptr;

public:
    explicit Actuator(int actuatorPin);
    virtual ~Actuator() = default;

    // Core actuator interface - ISP compliance
    virtual void begin() = 0;
    virtual void setState(bool state) = 0;
    virtual bool getState() const = 0;

    // Event-driven programming support
    void setOnStateChangeCallback(std::function<void(bool)> callback);
    
    // State checking
    bool isInitialized() const { return initialized; }

protected:
    void notifyStateChange(bool newState);
};

#endif // ACTUATOR_H
