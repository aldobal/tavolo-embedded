#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t address, uint8_t cols, uint8_t rows) 
    : lcd(address, cols, rows) {}

void DisplayManager::begin() {
    Serial.println("Initializing Display Manager...");
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    Serial.println("Display Manager initialized successfully.");
}

void DisplayManager::update() {
    unsigned long currentTime = millis();
    
    // Check for message timeout
    if (messageTimeout > 0 && currentTime >= messageTimeout) {
        messageTimeout = 0;
        setMode(DisplayMode::WEIGHT_DISPLAY);
    }
    
    // Update display at regular intervals
    if (needsUpdate || (currentTime - lastUpdate >= UPDATE_INTERVAL)) {
        updateDisplay();
        lastUpdate = currentTime;
        needsUpdate = false;
    }
}

void DisplayManager::setMode(DisplayMode mode) {
    if (currentMode != mode) {
        currentMode = mode;
        needsUpdate = true;
        
        Serial.print("Display mode changed to: ");
        switch (mode) {
            case DisplayMode::BOOT:
                Serial.println("BOOT");
                break;
            case DisplayMode::WEIGHT_DISPLAY:
                Serial.println("WEIGHT_DISPLAY");
                break;
            case DisplayMode::STATUS_MESSAGE:
                Serial.println("STATUS_MESSAGE");
                break;
            case DisplayMode::ERROR_MESSAGE:
                Serial.println("ERROR_MESSAGE");
                break;
        }
    }
}

void DisplayManager::showWeightData(float weight, const String& status) {
    formatWeightDisplay(weight, status);
    setMode(DisplayMode::WEIGHT_DISPLAY);
}

void DisplayManager::showStatusMessage(const String& message, unsigned long timeout) {
    line1Content = centerText("STATUS", 20);
    line2Content = centerText(message.substring(0, 20), 20);
    line3Content = "";
    line4Content = centerText("Press any key...", 20);
    
    setMode(DisplayMode::STATUS_MESSAGE);
    messageTimeout = millis() + timeout;
}

void DisplayManager::showErrorMessage(const String& error, unsigned long timeout) {
    line1Content = centerText("ERROR", 20);
    line2Content = centerText(error.substring(0, 20), 20);
    line3Content = "";
    line4Content = centerText("Check system...", 20);
    
    setMode(DisplayMode::ERROR_MESSAGE);
    messageTimeout = millis() + timeout;
}

void DisplayManager::showBootScreen(const String& deviceId) {
    line1Content = centerText("TAVOLO SYSTEM v1.0", 20);
    line2Content = centerText("by Codares", 20);
    line3Content = centerText(deviceId.substring(0, 20), 20);
    line4Content = centerText("Initializing...", 20);
    
    setMode(DisplayMode::BOOT);
}

void DisplayManager::clear() {
    lcd.clear();
    line1Content = "";
    line2Content = "";
    line3Content = "";
    line4Content = "";
    needsUpdate = true;
}

void DisplayManager::setBrightness(bool on) {
    if (on) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
}

void DisplayManager::updateDisplay() {
    lcd.setCursor(0, 0);
    lcd.print(line1Content);
    
    lcd.setCursor(0, 1);
    lcd.print(line2Content);
    
    lcd.setCursor(0, 2);
    lcd.print(line3Content);
    
    lcd.setCursor(0, 3);
    lcd.print(line4Content);
}

void DisplayManager::formatWeightDisplay(float weight, const String& status) {
    line1Content = centerText("TAVOLO WEIGHT", 20);
    line2Content = centerText(formatWeight(weight), 20);
    line3Content = centerText("Status: " + status, 20);
    
    // Show connection status on line 4
    line4Content = centerText("Connected to Edge", 20);
}

String DisplayManager::centerText(const String& text, int width) {
    if (text.length() >= width) {
        return text.substring(0, width);
    }
    
    int padding = (width - text.length()) / 2;
    String result = "";
    
    for (int i = 0; i < padding; i++) {
        result += " ";
    }
    
    result += text;
    
    while (result.length() < width) {
        result += " ";
    }
    
    return result;
}

String DisplayManager::formatWeight(float weight) {
    if (weight < 1000) {
        return String(weight, 1) + " g";
    } else {
        return String(weight / 1000.0, 2) + " kg";
    }
}
