#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/**
 * @brief Display Manager for LCD screen following Single Responsibility Principle
 * 
 * This class manages all display operations including weight display,
 * status messages, and screen management.
 */
class DisplayManager {
public:
    enum class DisplayMode {
        BOOT,
        WEIGHT_DISPLAY,
        STATUS_MESSAGE,
        ERROR_MESSAGE
    };

private:
    LiquidCrystal_I2C lcd;
    DisplayMode currentMode = DisplayMode::BOOT;
    unsigned long lastUpdate = 0;
    unsigned long messageTimeout = 0;
    bool needsUpdate = true;
    
    // Display content
    String line1Content = "";
    String line2Content = "";
    String line3Content = "";
    String line4Content = "";
    
    // Constants
    static const unsigned long UPDATE_INTERVAL = 200; // 5 Hz refresh rate
    static const unsigned long DEFAULT_MESSAGE_TIMEOUT = 3000; // 3 seconds

public:
    DisplayManager(uint8_t address = 0x27, uint8_t cols = 20, uint8_t rows = 4);
    
    void begin();
    void update();
    
    // Display control
    void setMode(DisplayMode mode);
    DisplayMode getMode() const { return currentMode; }
    
    // Content management
    void showWeightData(float weight, const String& status);
    void showStatusMessage(const String& message, unsigned long timeout = DEFAULT_MESSAGE_TIMEOUT);
    void showErrorMessage(const String& error, unsigned long timeout = DEFAULT_MESSAGE_TIMEOUT);
    void showBootScreen(const String& deviceId);
    
    // Utility methods
    void clear();
    void setBrightness(bool on);
    
private:
    void updateDisplay();
    void formatWeightDisplay(float weight, const String& status);
    String centerText(const String& text, int width);
    String formatWeight(float weight);
};

#endif // DISPLAY_MANAGER_H
