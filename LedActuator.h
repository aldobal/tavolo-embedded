#ifndef LED_ACTUATOR_H
#define LED_ACTUATOR_H

#include "Actuator.h"

/**
 * @brief LED Actuator implementation following Single Responsibility Principle
 * 
 * This class handles LED control operations including blinking patterns.
 * It supports both digital and PWM control modes.
 */
class LedActuator : public Actuator {
public:
    enum class BlinkPattern {
        OFF,
        ON,
        SLOW_BLINK,    // 1 Hz
        FAST_BLINK,    // 5 Hz
        PULSE          // Breathing effect
    };

private:
    BlinkPattern currentPattern = BlinkPattern::OFF;
    unsigned long lastToggleTime = 0;
    bool ledState = false;
    int brightness = 255;
    bool fadeDirection = true;
    
    // Timing constants
    static const unsigned long SLOW_BLINK_INTERVAL = 500;  // 500ms on/off
    static const unsigned long FAST_BLINK_INTERVAL = 100;  // 100ms on/off
    static const unsigned long PULSE_INTERVAL = 20;       // 20ms for smooth pulse

public:
    explicit LedActuator(int ledPin);
    virtual ~LedActuator() = default;

    // Actuator interface implementation
    void begin() override;
    void setState(bool state) override;
    bool getState() const override;

    // LED-specific methods
    void setPattern(BlinkPattern pattern);
    BlinkPattern getPattern() const { return currentPattern; }
    void setBrightness(int brightness); // 0-255
    int getBrightness() const { return brightness; }
    
    // Non-blocking update method for reactive programming
    void update();

private:
    void updateBlinkPattern();
    void updatePulsePattern();
    void writeLed(bool state);
    void writeLedPWM(int value);
};

#endif // LED_ACTUATOR_H
