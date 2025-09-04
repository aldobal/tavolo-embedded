#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

#include "Sensor.h"
#include "HX711.h"

/**
 * @brief HX711 Weight Sensor implementation following Single Responsibility Principle
 * 
 * This class handles all weight sensing operations using the HX711 load cell amplifier.
 * It provides calibrated weight readings and follows the Open/Closed Principle.
 */
class WeightSensor : public Sensor {
private:
    int clockPin;
    HX711 scale;
    float calibrationFactor;
    bool calibrated = false;
    unsigned long lastReadTime = 0;
    const unsigned long READ_INTERVAL_MS = 100; // 10 Hz sampling rate
    float lastWeight = 0.0;
    float weightThreshold = 1.0; // Minimum weight change to trigger callback

public:
    WeightSensor(int dataPin, int clockPin, float calibrationFactor = 0.42f);
    virtual ~WeightSensor() = default;

    // Sensor interface implementation
    void begin() override;
    float read() override;
    bool isReady() const override;

    // Weight-specific methods
    void tare();
    void setCalibrationFactor(float factor);
    float getCalibrationFactor() const { return calibrationFactor; }
    void setWeightThreshold(float threshold) { weightThreshold = threshold; }
    
    // Reactive programming support
    void update(); // Non-blocking update method
    bool hasNewData() const;

private:
    bool shouldTriggerCallback(float newWeight) const;
};

#endif // WEIGHT_SENSOR_H
