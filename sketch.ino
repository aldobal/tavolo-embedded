/*
    ================================================================
    TAVOLO SMART WEIGHT DETECTION SYSTEM - ESP32 IoT Device
    ================================================================
    
    Developer: Aldo Alberto Baldeon Fabian
    Company: Codares
    Project: Tavolo IoT Weight Detection System
    Version: 1.0.0
    Created: September 2025
    License: CC BY-NC-SA 4.0 (Non-Commercial)
    
    ================================================================
    SYSTEM DESCRIPTION
    ================================================================
    
    This is an advanced IoT embedded system that implements:
    - Event-driven architecture
    - Finite state machine
    - Reactive programming principles
    - SOLID design principles
    - Clean code practices

    Hardware Configuration (based on diagram.json):
    * HX711 Weight Sensor: DT=Pin2, SCK=Pin4
    * LED Indicator: Pin5 (with 220Ω resistor)
    * LCD Display I2C: SDA=Pin21, SCL=Pin22
    * ESP32 DevKit C v4

    System Features:
    - Real-time weight monitoring
    - Edge API communication via MQTT
    - Threshold-based LED alerts
    - LCD status display
    - Remote configuration capabilities
    - Automatic calibration and tare

    Architecture:
    - TavoloSystem: Main orchestrator implementing FSM
    - WeightSensor: HX711 interface with reactive updates
    - LedActuator: Smart LED control with patterns
    - DisplayManager: LCD display management
    - EdgeCommunication: MQTT communication with Edge API
    
    ================================================================
    COPYRIGHT & LICENSE
    ================================================================
    
    © 2025 Aldo Alberto Baldeon Fabian - Codares
    This project is licensed under CC BY-NC-SA 4.0 (Non-Commercial)
    Commercial use is strictly prohibited.
    
    ================================================================

*/

#include "TavoloSystem.h"
#include <WiFi.h>
#include <time.h>

// Hardware pin configuration (matching diagram.json)
const int WEIGHT_DATA_PIN = 2;    // HX711 DT pin
const int WEIGHT_CLOCK_PIN = 4;   // HX711 SCK pin
const int LED_PIN = 5;            // LED indicator pin
const uint8_t LCD_I2C_ADDRESS = 0x27; // LCD I2C address

// WiFi configuration
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// NTP configuration
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = -5 * 3600;  // GMT-5 (adjust for your timezone)
const int DAYLIGHT_OFFSET_SEC = 0;

// System instance
TavoloSystem* tavoloSystem = nullptr;

// Timing for non-blocking operations
unsigned long lastStatusReport = 0;
const unsigned long STATUS_REPORT_INTERVAL = 30000; // 30 seconds

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow serial to initialize
    
    // Display developer and project information
    printWelcomeBanner();
    
    Serial.println("Initializing...");
    
    // Initialize WiFi
    setupWiFi();
    
    // Initialize NTP
    setupNTP();
    
    // Create and initialize the Tavolo system
    tavoloSystem = new TavoloSystem(
        WEIGHT_DATA_PIN, 
        WEIGHT_CLOCK_PIN, 
        LED_PIN, 
        LCD_I2C_ADDRESS
    );
    
    // Set up event callbacks for demonstration
    setupEventCallbacks();
    
    // Initialize the system
    tavoloSystem->setup();
    
    Serial.println("System ready!");
    Serial.println("=====================================");
    
    // Show initial system status
    tavoloSystem->showSystemStatus();
}

void loop() {
    // Main system loop - all operations are reactive and non-blocking
    if (tavoloSystem != nullptr) {
        tavoloSystem->loop();
    }
    
    // Periodic status reporting
    periodicStatusReport();
    
    // Handle serial commands for testing
    handleSerialCommands();
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Address: ");
        Serial.println(WiFi.macAddress());
    } else {
        Serial.println();
        Serial.println("WiFi connection failed! System will continue in offline mode.");
    }
}

void setupNTP() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Configuring NTP time synchronization...");
        configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.println("Time synchronized successfully!");
            Serial.print("Current time: ");
            Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        } else {
            Serial.println("Failed to obtain time from NTP server");
        }
    }
}

void setupEventCallbacks() {
    if (tavoloSystem == nullptr) return;
    
    // Weight change callback
    tavoloSystem->setOnWeightChangeCallback([](float weight) {
        // This could be used for additional processing
        // For now, just log significant changes
        static float lastLoggedWeight = 0;
        if (abs(weight - lastLoggedWeight) > 10.0) { // Log every 10g change
            Serial.print("Weight changed: ");
            Serial.print(weight, 1);
            Serial.println("g");
            lastLoggedWeight = weight;
        }
    });
    
    // Threshold state change callback
    tavoloSystem->setOnThresholdStateChangeCallback([](bool exceeded) {
        if (exceeded) {
            Serial.println("⚠️  WEIGHT THRESHOLD EXCEEDED!");
        } else {
            Serial.println("✅ Weight back to normal range");
        }
    });
}

void periodicStatusReport() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastStatusReport >= STATUS_REPORT_INTERVAL) {
        if (tavoloSystem != nullptr) {
            tavoloSystem->showSystemStatus();
        }
        lastStatusReport = currentTime;
    }
}

void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();
        
        if (tavoloSystem == nullptr) return;
        
        Serial.print("Processing command: ");
        Serial.println(command);
        
        if (command == "STATUS") {
            tavoloSystem->showSystemStatus();
        } else if (command == "TARE") {
            tavoloSystem->tare();
        } else if (command == "CALIBRATE") {
            tavoloSystem->calibrate();
        } else if (command.startsWith("THRESHOLD=")) {
            float threshold = command.substring(10).toFloat();
            tavoloSystem->setWeightThreshold(threshold);
            Serial.print("Threshold set to: ");
            Serial.print(threshold);
            Serial.println("g");
        } else if (command == "START") {
            tavoloSystem->startMeasurement();
        } else if (command == "STOP") {
            tavoloSystem->stopMeasurement();
        } else if (command == "HELP") {
            printHelp();
        } else {
            Serial.println("Unknown command. Type HELP for available commands.");
        }
    }
}

void printHelp() {
    Serial.println("\n=== AVAILABLE COMMANDS ===");
    Serial.println("STATUS       - Show system status");
    Serial.println("TARE         - Perform tare (zero weight)");
    Serial.println("CALIBRATE    - Start calibration process");
    Serial.println("THRESHOLD=X  - Set weight threshold to X grams");
    Serial.println("START        - Start weight measurements");
    Serial.println("STOP         - Stop weight measurements");
    Serial.println("HELP         - Show this help message");
    Serial.println("===========================\n");
}

// Error handling for system crashes
void handleSystemError(const String& error) {
    Serial.print("SYSTEM ERROR: ");
    Serial.println(error);
    
    // Implement recovery mechanisms here
    // For now, just log and continue
}

// System health monitoring
void monitorSystemHealth() {
    // Check memory usage
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 10000) { // Less than 10KB free
        Serial.print("⚠️  Low memory warning: ");
        Serial.print(freeHeap);
        Serial.println(" bytes free");
    }
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️  WiFi connection lost - attempting reconnection...");
        setupWiFi();
    }
}

void printWelcomeBanner() {
    Serial.println();
    Serial.println("================================================================");
    Serial.println("           TAVOLO SMART WEIGHT DETECTION SYSTEM");
    Serial.println("================================================================");
    Serial.println();
    Serial.println("Developer Information:");
    Serial.println("  • Name: Aldo Alberto Baldeon Fabian");
    Serial.println("  • Company: Codares");
    Serial.println("  • Project: Tavolo IoT Weight Detection System");
    Serial.println("  • Version: 1.0.0");
    Serial.println("  • Date: September 2025");
    Serial.println();
    Serial.println("System Features:");
    Serial.println("  ✓ Event-Driven Architecture");
    Serial.println("  ✓ Finite State Machine");
    Serial.println("  ✓ Reactive Programming");
    Serial.println("  ✓ SOLID Design Principles");
    Serial.println("  ✓ Edge API Communication (MQTT)");
    Serial.println("  ✓ Real-time Weight Monitoring");
    Serial.println("  ✓ Smart LED Indicators");
    Serial.println("  ✓ LCD Status Display");
    Serial.println();
    Serial.println("Hardware Components:");
    Serial.println("  • ESP32 DevKit C v4");
    Serial.println("  • HX711 Weight Sensor (50kg)");
    Serial.println("  • LCD I2C 20x4 Display");
    Serial.println("  • LED Status Indicator");
    Serial.println();
    Serial.println("Copyright © 2025 Codares - All Rights Reserved");
    Serial.println("================================================================");
    Serial.println();
}
