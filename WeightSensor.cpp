#include "WeightSensor.h"

WeightSensor::WeightSensor(int dataPin, int clockPin, float calibrationFactor)
    : Sensor(dataPin), clockPin(clockPin), calibrationFactor(calibrationFactor) {}

void WeightSensor::begin() {
    Serial.println("Initializing Weight Sensor (HX711)...");
    
    scale.begin(pin, clockPin);
    scale.set_gain(128);
    scale.set_scale(calibrationFactor);
    
    // Wait for the scale to stabilize
    Serial.println("Stabilizing scale...");
    delay(1000);
    
    // Perform initial tare
    tare();
    
    initialized = true;
    calibrated = true;
    
    Serial.println("Weight Sensor initialized successfully.");
    Serial.print("Calibration Factor: ");
    Serial.println(calibrationFactor, 6);
}

float WeightSensor::read() {
    if (!initialized || !calibrated) {
        Serial.println("Warning: Weight sensor not properly initialized");
        return 0.0;
    }

    if (scale.is_ready()) {
        float weight = scale.get_units(3); // Average of 3 readings
        
        // Apply basic filtering
        if (weight < 0) weight = 0; // No negative weights
        
        return weight;
    }
    
    return lastWeight; // Return last known good reading
}

bool WeightSensor::isReady() const {
    return initialized && calibrated;
}

void WeightSensor::tare() {
    if (!initialized) {
        Serial.println("Error: Cannot tare - sensor not initialized");
        return;
    }
    
    Serial.println("Performing tare...");
    scale.tare();
    Serial.println("Tare completed.");
}

void WeightSensor::setCalibrationFactor(float factor) {
    calibrationFactor = factor;
    if (initialized) {
        scale.set_scale(calibrationFactor);
        Serial.print("Calibration factor updated to: ");
        Serial.println(calibrationFactor, 6);
    }
}

void WeightSensor::update() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastReadTime >= READ_INTERVAL_MS) {
        if (isReady() && scale.is_ready()) {
            float newWeight = read();
            
            if (shouldTriggerCallback(newWeight)) {
                lastWeight = newWeight;
                notifyDataReady(newWeight);
            }
            
            lastReadTime = currentTime;
        }
    }
}

bool WeightSensor::hasNewData() const {
    return millis() - lastReadTime < READ_INTERVAL_MS && initialized && calibrated;
}

bool WeightSensor::shouldTriggerCallback(float newWeight) const {
    return abs(newWeight - lastWeight) >= weightThreshold;
}
