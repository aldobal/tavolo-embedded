#include "TavoloSystem.h"

TavoloSystem::TavoloSystem(int weightDataPin, int weightClockPin, int ledPin, uint8_t lcdAddress)
    : Device() {
    
    // Initialize hardware components
    weightSensor = new WeightSensor(weightDataPin, weightClockPin, config.calibrationFactor);
    ledActuator = new LedActuator(ledPin);
    displayManager = new DisplayManager(lcdAddress);
    edgeCommunication = new EdgeCommunication(getDeviceId());
    
    // Set up event-driven callbacks
    setupEventCallbacks();
}

void TavoloSystem::setupEventCallbacks() {
    // Weight sensor callback
    weightSensor->setOnDataCallback([this](float weight) {
        this->onWeightDataReceived(weight);
    });
    
    // Edge communication callbacks
    edgeCommunication->setOnCommandCallback([this](const EdgeCommunication::EdgeCommand& cmd) {
        this->onEdgeCommandReceived(cmd);
    });
    
    edgeCommunication->setOnConnectionStateCallback([this](EdgeCommunication::ConnectionState state) {
        this->onConnectionStateChanged(state);
    });
}

TavoloSystem::~TavoloSystem() {
    delete weightSensor;
    delete ledActuator;
    delete displayManager;
    delete edgeCommunication;
}

void TavoloSystem::setup() {
    Device::setup();
    
    Serial.println("=== TAVOLO SMART WEIGHT DETECTION SYSTEM ===");
    Serial.println("Developed by: Aldo Alberto Baldeon Fabian (Codares)");
    Serial.println("Project: Tavolo IoT Weight Detection System v1.0.0");
    Serial.println("Initializing system components...");
    
    // Show boot screen
    displayManager->begin();
    displayManager->showBootScreen(getDeviceId());
    
    changeSystemState(SystemState::INITIALIZING);
    
    // Initialize components in order
    Serial.println("Initializing Weight Sensor...");
    weightSensor->begin();
    
    Serial.println("Initializing LED Actuator...");
    ledActuator->begin();
    ledActuator->setPattern(LedActuator::BlinkPattern::SLOW_BLINK);
    
    Serial.println("Initializing Edge Communication...");
    edgeCommunication->begin();
    
    // Move to calibration state
    changeSystemState(SystemState::CALIBRATING);
    
    Serial.println("System initialization complete!");
    showSystemStatus();
}

void TavoloSystem::loop() {
    Device::loop();
    
    // Update all components (reactive programming)
    weightSensor->update();
    ledActuator->update();
    displayManager->update();
    edgeCommunication->update();
    
    // Update state machine
    updateStateMachine();
    
    // Update system operations
    updateMeasurements();
    updateDisplay();
    updateCommunication();
}

void TavoloSystem::updateStateMachine() {
    switch (currentSystemState) {
        case SystemState::INITIALIZING:
            // Automatically move to calibrating after components are ready
            if (weightSensor->isReady() && ledActuator->isInitialized()) {
                changeSystemState(SystemState::CALIBRATING);
            }
            break;
            
        case SystemState::CALIBRATING:
            // Move to idle after a short calibration period
            if (millis() > 5000) { // 5 seconds calibration time
                changeSystemState(SystemState::IDLE);
            }
            break;
            
        case SystemState::IDLE:
            // Move to measuring when weight sensor has new data
            if (weightSensor->hasNewData()) {
                changeSystemState(SystemState::MEASURING);
            }
            break;
            
        case SystemState::MEASURING:
            // Check threshold and decide next state
            checkThreshold();
            break;
            
        case SystemState::THRESHOLD_EXCEEDED:
            // Return to measuring if weight drops below threshold
            if (currentWeight < config.weightThreshold * 0.9) { // 10% hysteresis
                changeSystemState(SystemState::MEASURING);
            }
            break;
            
        case SystemState::COMMUNICATION_ERROR:
            // Try to recover by returning to measuring
            if (edgeCommunication->isConnected()) {
                changeSystemState(SystemState::MEASURING);
            }
            break;
            
        case SystemState::MAINTENANCE:
            // Manual state - requires external command to exit
            break;
    }
}

void TavoloSystem::changeSystemState(SystemState newState) {
    if (currentSystemState != newState) {
        SystemState oldState = currentSystemState;
        
        handleStateExit(oldState);
        currentSystemState = newState;
        handleStateEntry(newState);
        
        Serial.print("System state changed: ");
        Serial.print(stateToString(oldState));
        Serial.print(" -> ");
        Serial.println(stateToString(newState));
    }
}

void TavoloSystem::handleStateEntry(SystemState state) {
    switch (state) {
        case SystemState::INITIALIZING:
            ledActuator->setPattern(LedActuator::BlinkPattern::FAST_BLINK);
            setState(DeviceState::INITIALIZING);
            break;
            
        case SystemState::CALIBRATING:
            ledActuator->setPattern(LedActuator::BlinkPattern::PULSE);
            displayManager->showStatusMessage("Calibrating...", 3000);
            if (config.autoTare) {
                tare();
            }
            break;
            
        case SystemState::IDLE:
            ledActuator->setPattern(LedActuator::BlinkPattern::OFF);
            setState(DeviceState::READY);
            break;
            
        case SystemState::MEASURING:
            setState(DeviceState::ACTIVE);
            break;
            
        case SystemState::THRESHOLD_EXCEEDED:
            ledActuator->setPattern(LedActuator::BlinkPattern::ON);
            if (onThresholdStateChangeCallback) {
                onThresholdStateChangeCallback(true);
            }
            break;
            
        case SystemState::COMMUNICATION_ERROR:
            ledActuator->setPattern(LedActuator::BlinkPattern::FAST_BLINK);
            displayManager->showErrorMessage("Comm Error", 5000);
            setState(DeviceState::ERROR);
            break;
            
        case SystemState::MAINTENANCE:
            ledActuator->setPattern(LedActuator::BlinkPattern::SLOW_BLINK);
            displayManager->showStatusMessage("Maintenance Mode", 0);
            setState(DeviceState::MAINTENANCE);
            break;
    }
}

void TavoloSystem::handleStateExit(SystemState state) {
    switch (state) {
        case SystemState::THRESHOLD_EXCEEDED:
            if (onThresholdStateChangeCallback) {
                onThresholdStateChangeCallback(false);
            }
            break;
        default:
            break;
    }
}

void TavoloSystem::onWeightDataReceived(float weight) {
    currentWeight = weight;
    lastMeasurementTime = millis();
    
    if (onWeightChangeCallback) {
        onWeightChangeCallback(weight);
    }
    
    // Report weight data if needed
    if (shouldReportWeight()) {
        reportWeightData();
    }
}

void TavoloSystem::onEdgeCommandReceived(const EdgeCommunication::EdgeCommand& command) {
    Serial.print("Received edge command: ");
    Serial.print(command.command);
    Serial.print(" = ");
    Serial.println(command.value);
    
    if (command.command == "SET_THRESHOLD") {
        float newThreshold = command.value.toFloat();
        setWeightThreshold(newThreshold);
    } else if (command.command == "LED_ON") {
        ledActuator->setPattern(LedActuator::BlinkPattern::ON);
    } else if (command.command == "LED_OFF") {
        ledActuator->setPattern(LedActuator::BlinkPattern::OFF);
    } else if (command.command == "TARE") {
        tare();
    } else if (command.command == "CALIBRATE") {
        calibrate();
    } else if (command.command == "MAINTENANCE") {
        changeSystemState(SystemState::MAINTENANCE);
    } else if (command.command == "RESUME") {
        changeSystemState(SystemState::IDLE);
    }
}

void TavoloSystem::onConnectionStateChanged(EdgeCommunication::ConnectionState state) {
    if (state == EdgeCommunication::ConnectionState::ERROR) {
        if (currentSystemState != SystemState::COMMUNICATION_ERROR) {
            changeSystemState(SystemState::COMMUNICATION_ERROR);
        }
    } else if (state == EdgeCommunication::ConnectionState::CONNECTED) {
        if (currentSystemState == SystemState::COMMUNICATION_ERROR) {
            changeSystemState(SystemState::IDLE);
        }
    }
}

void TavoloSystem::updateMeasurements() {
    // This is handled by the reactive callbacks
    // The weight sensor will call onWeightDataReceived when new data is available
}

void TavoloSystem::checkThreshold() {
    bool wasExceeded = thresholdExceeded;
    thresholdExceeded = currentWeight > config.weightThreshold;
    
    if (thresholdExceeded != wasExceeded) {
        if (thresholdExceeded) {
            changeSystemState(SystemState::THRESHOLD_EXCEEDED);
        } else if (currentSystemState == SystemState::THRESHOLD_EXCEEDED) {
            changeSystemState(SystemState::MEASURING);
        }
    }
}

void TavoloSystem::updateDisplay() {
    if (currentSystemState == SystemState::MEASURING || 
        currentSystemState == SystemState::THRESHOLD_EXCEEDED ||
        currentSystemState == SystemState::IDLE) {
        
        String status = thresholdExceeded ? "OVER LIMIT" : "NORMAL";
        displayManager->showWeightData(currentWeight, status);
    }
}

void TavoloSystem::updateCommunication() {
    // This is handled by the EdgeCommunication component itself
    // It will automatically try to reconnect and handle MQTT operations
}

void TavoloSystem::reportWeightData() {
    if (edgeCommunication->isConnected()) {
        EdgeCommunication::WeightData data;
        data.weight = currentWeight;
        data.timestamp = millis();
        data.deviceId = getDeviceId();
        
        if (edgeCommunication->sendWeightData(data)) {
            lastReportedWeight = currentWeight;
            lastReportTime = millis();
        }
    }
}

bool TavoloSystem::shouldReportWeight() const {
    unsigned long currentTime = millis();
    
    // Report if enough time has passed
    if (currentTime - lastReportTime >= REPORT_INTERVAL) {
        return true;
    }
    
    // Report if weight changed significantly
    if (abs(currentWeight - lastReportedWeight) >= WEIGHT_REPORT_THRESHOLD) {
        return true;
    }
    
    return false;
}

void TavoloSystem::startMeasurement() {
    if (currentSystemState == SystemState::IDLE) {
        changeSystemState(SystemState::MEASURING);
    }
}

void TavoloSystem::stopMeasurement() {
    if (currentSystemState == SystemState::MEASURING || 
        currentSystemState == SystemState::THRESHOLD_EXCEEDED) {
        changeSystemState(SystemState::IDLE);
    }
}

void TavoloSystem::calibrate() {
    changeSystemState(SystemState::CALIBRATING);
}

void TavoloSystem::tare() {
    weightSensor->tare();
    displayManager->showStatusMessage("Tare Complete", 2000);
}

void TavoloSystem::setWeightThreshold(float threshold) {
    config.weightThreshold = threshold;
    Serial.print("Weight threshold updated to: ");
    Serial.print(threshold);
    Serial.println("g");
}

void TavoloSystem::setCalibrationFactor(float factor) {
    config.calibrationFactor = factor;
    weightSensor->setCalibrationFactor(factor);
}

void TavoloSystem::setMeasurementInterval(unsigned long interval) {
    config.measurementInterval = interval;
}

String TavoloSystem::getSystemStateString() const {
    switch (currentSystemState) {
        case SystemState::INITIALIZING: return "INITIALIZING";
        case SystemState::CALIBRATING: return "CALIBRATING";
        case SystemState::IDLE: return "IDLE";
        case SystemState::MEASURING: return "MEASURING";
        case SystemState::THRESHOLD_EXCEEDED: return "THRESHOLD_EXCEEDED";
        case SystemState::COMMUNICATION_ERROR: return "COMMUNICATION_ERROR";
        case SystemState::MAINTENANCE: return "MAINTENANCE";
        default: return "UNKNOWN";
    }
}

void TavoloSystem::setOnWeightChangeCallback(std::function<void(float)> callback) {
    onWeightChangeCallback = callback;
}

void TavoloSystem::setOnThresholdStateChangeCallback(std::function<void(bool)> callback) {
    onThresholdStateChangeCallback = callback;
}

void TavoloSystem::showSystemStatus() {
    Serial.println("\n=== SYSTEM STATUS ===");
    Serial.print("Device ID: ");
    Serial.println(getDeviceId());
    Serial.print("System State: ");
    Serial.println(getSystemStateString());
    Serial.print("Current Weight: ");
    Serial.print(currentWeight);
    Serial.println("g");
    Serial.print("Weight Threshold: ");
    Serial.print(config.weightThreshold);
    Serial.println("g");
    Serial.print("Threshold Exceeded: ");
    Serial.println(thresholdExceeded ? "YES" : "NO");
    Serial.print("Edge Connected: ");
    Serial.println(edgeCommunication->isConnected() ? "YES" : "NO");
    Serial.println("====================\n");
}

String TavoloSystem::stateToString(SystemState state) const {
    switch (state) {
        case SystemState::INITIALIZING: return "INITIALIZING";
        case SystemState::CALIBRATING: return "CALIBRATING";
        case SystemState::IDLE: return "IDLE";
        case SystemState::MEASURING: return "MEASURING";
        case SystemState::THRESHOLD_EXCEEDED: return "THRESHOLD_EXCEEDED";
        case SystemState::COMMUNICATION_ERROR: return "COMMUNICATION_ERROR";
        case SystemState::MAINTENANCE: return "MAINTENANCE";
        default: return "UNKNOWN";
    }
}
