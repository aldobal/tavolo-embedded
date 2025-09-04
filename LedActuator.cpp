#include "LedActuator.h"

LedActuator::LedActuator(int ledPin) : Actuator(ledPin) {}

void LedActuator::begin() {
    Serial.println("Initializing LED Actuator...");
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    initialized = true;
    currentState = false;
    
    Serial.print("LED Actuator initialized on pin: ");
    Serial.println(pin);
}

void LedActuator::setState(bool state) {
    if (state) {
        setPattern(BlinkPattern::ON);
    } else {
        setPattern(BlinkPattern::OFF);
    }
    
    notifyStateChange(state);
}

bool LedActuator::getState() const {
    return currentPattern != BlinkPattern::OFF;
}

void LedActuator::setPattern(BlinkPattern pattern) {
    if (currentPattern != pattern) {
        currentPattern = pattern;
        lastToggleTime = millis();
        
        Serial.print("LED pattern changed to: ");
        switch (pattern) {
            case BlinkPattern::OFF:
                Serial.println("OFF");
                writeLed(false);
                break;
            case BlinkPattern::ON:
                Serial.println("ON");
                writeLed(true);
                break;
            case BlinkPattern::SLOW_BLINK:
                Serial.println("SLOW_BLINK");
                break;
            case BlinkPattern::FAST_BLINK:
                Serial.println("FAST_BLINK");
                break;
            case BlinkPattern::PULSE:
                Serial.println("PULSE");
                brightness = 0;
                fadeDirection = true;
                break;
        }
    }
}

void LedActuator::setBrightness(int newBrightness) {
    brightness = constrain(newBrightness, 0, 255);
}

void LedActuator::update() {
    if (!initialized) return;
    
    switch (currentPattern) {
        case BlinkPattern::OFF:
            // Nothing to update
            break;
        case BlinkPattern::ON:
            // Nothing to update
            break;
        case BlinkPattern::SLOW_BLINK:
        case BlinkPattern::FAST_BLINK:
            updateBlinkPattern();
            break;
        case BlinkPattern::PULSE:
            updatePulsePattern();
            break;
    }
}

void LedActuator::updateBlinkPattern() {
    unsigned long currentTime = millis();
    unsigned long interval = (currentPattern == BlinkPattern::SLOW_BLINK) ? 
                           SLOW_BLINK_INTERVAL : FAST_BLINK_INTERVAL;
    
    if (currentTime - lastToggleTime >= interval) {
        ledState = !ledState;
        writeLed(ledState);
        lastToggleTime = currentTime;
    }
}

void LedActuator::updatePulsePattern() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastToggleTime >= PULSE_INTERVAL) {
        if (fadeDirection) {
            brightness += 5;
            if (brightness >= 255) {
                brightness = 255;
                fadeDirection = false;
            }
        } else {
            brightness -= 5;
            if (brightness <= 0) {
                brightness = 0;
                fadeDirection = true;
            }
        }
        
        writeLedPWM(brightness);
        lastToggleTime = currentTime;
    }
}

void LedActuator::writeLed(bool state) {
    digitalWrite(pin, state ? HIGH : LOW);
    ledState = state;
}

void LedActuator::writeLedPWM(int value) {
    analogWrite(pin, value);
}
