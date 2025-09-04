#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <functional>

/**
 * @brief Abstract base class for all sensors following the Interface Segregation Principle
 * 
 * This class defines the contract that all sensors must implement.
 * It follows the Single Responsibility Principle by focusing only on sensor operations.
 */
class Sensor {
protected:
    int pin;
    bool initialized = false;
    std::function<void(float)> onDataCallback = nullptr;

public:
    explicit Sensor(int sensorPin);
    virtual ~Sensor() = default;

    // Core sensor interface - ISP compliance
    virtual void begin() = 0;
    virtual float read() = 0;
    virtual bool isReady() const = 0;

    // Event-driven programming support
    void setOnDataCallback(std::function<void(float)> callback);
    
    // State checking
    bool isInitialized() const { return initialized; }

protected:
    void notifyDataReady(float data);
};

#endif // SENSOR_H
