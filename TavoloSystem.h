#ifndef TAVOLO_SYSTEM_H
#define TAVOLO_SYSTEM_H

#include "Device.h"
#include "WeightSensor.h"
#include "LedActuator.h"
#include "DisplayManager.h"
#include "EdgeCommunication.h"
#include <functional>

/**
 * @brief Main Tavolo System implementing Finite State Machine and Event-Driven Architecture
 * 
 * This class orchestrates all system components following the Single Responsibility Principle
 * and implements a reactive system with state management.
 */
class TavoloSystem : public Device {
public:
    enum class SystemState {
        INITIALIZING,
        CALIBRATING,
        IDLE,
        MEASURING,
        THRESHOLD_EXCEEDED,
        COMMUNICATION_ERROR,
        MAINTENANCE
    };

    struct SystemConfig {
        float weightThreshold = 100.0; // grams
        unsigned long measurementInterval = 500; // ms
        float calibrationFactor = 0.42f;
        bool autoTare = true;
    };

private:
    // Hardware components
    WeightSensor* weightSensor;
    LedActuator* ledActuator;
    DisplayManager* displayManager;
    EdgeCommunication* edgeCommunication;
    
    // State management
    SystemState currentSystemState = SystemState::INITIALIZING;
    SystemConfig config;
    
    // Measurement data
    float currentWeight = 0.0;
    float lastReportedWeight = 0.0;
    unsigned long lastMeasurementTime = 0;
    unsigned long lastReportTime = 0;
    bool thresholdExceeded = false;
    
    // Event-driven callbacks
    std::function<void(float)> onWeightChangeCallback = nullptr;
    std::function<void(bool)> onThresholdStateChangeCallback = nullptr;
    
    // Communication
    const unsigned long REPORT_INTERVAL = 5000; // 5 seconds
    const float WEIGHT_REPORT_THRESHOLD = 5.0; // Report if weight changes by 5g

public:
    TavoloSystem(int weightDataPin, int weightClockPin, int ledPin, 
                 uint8_t lcdAddress = 0x27);
    virtual ~TavoloSystem();

    // Device interface implementation
    void setup() override;
    void loop() override;

    // System control
    void startMeasurement();
    void stopMeasurement();
    void calibrate();
    void tare();
    
    // Configuration
    void setWeightThreshold(float threshold);
    void setCalibrationFactor(float factor);
    void setMeasurementInterval(unsigned long interval);
    
    // State management
    SystemState getSystemState() const { return currentSystemState; }
    String getSystemStateString() const;
    
    // Event callbacks
    void setOnWeightChangeCallback(std::function<void(float)> callback);
    void setOnThresholdStateChangeCallback(std::function<void(bool)> callback);

    // System status
    void showSystemStatus();

private:
    // Initialization
    void setupEventCallbacks();
    
    // State machine implementation
    void updateStateMachine();
    void changeSystemState(SystemState newState);
    void handleStateEntry(SystemState state);
    void handleStateExit(SystemState state);
    
    // Event handlers
    void onWeightDataReceived(float weight);
    void onEdgeCommandReceived(const EdgeCommunication::EdgeCommand& command);
    void onConnectionStateChanged(EdgeCommunication::ConnectionState state);
    
    // System operations
    void updateMeasurements();
    void checkThreshold();
    void updateDisplay();
    void updateCommunication();
    void reportWeightData();
    
    // Utility methods
    bool shouldReportWeight() const;
    String stateToString(SystemState state) const;
};

#endif // TAVOLO_SYSTEM_H
