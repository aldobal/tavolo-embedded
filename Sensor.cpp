#include "Sensor.h"

Sensor::Sensor(int sensorPin) : pin(sensorPin) {}

void Sensor::setOnDataCallback(std::function<void(float)> callback) {
    onDataCallback = callback;
}

void Sensor::notifyDataReady(float data) {
    if (onDataCallback) {
        onDataCallback(data);
    }
}
