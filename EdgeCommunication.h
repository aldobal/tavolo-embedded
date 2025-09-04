#ifndef EDGE_COMMUNICATION_H
#define EDGE_COMMUNICATION_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <functional>

/**
 * @brief Edge Communication Manager following Single Responsibility Principle
 * 
 * This class handles all communication with the Edge API including:
 * - Sending weight data
 * - Receiving threshold updates and commands
 * - Managing connection state
 */
class EdgeCommunication {
public:
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    };

    struct WeightData {
        float weight;
        unsigned long timestamp;
        String deviceId;
    };

    struct EdgeCommand {
        String command;
        String value;
        unsigned long timestamp;
    };

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    // Connection settings
    String mqttServer = "broker.hivemq.com"; // Public broker for testing
    int mqttPort = 1883;
    String clientId;
    String deviceId;
    
    // Topics
    String weightTopic;
    String commandTopic;
    String statusTopic;
    
    // State management
    ConnectionState currentState = ConnectionState::DISCONNECTED;
    unsigned long lastConnectionAttempt = 0;
    unsigned long lastHeartbeat = 0;
    const unsigned long RECONNECT_INTERVAL = 5000;
    const unsigned long HEARTBEAT_INTERVAL = 30000;
    
    // Callbacks for event-driven architecture
    std::function<void(const EdgeCommand&)> onCommandCallback = nullptr;
    std::function<void(ConnectionState)> onConnectionStateCallback = nullptr;

public:
    EdgeCommunication(const String& deviceId);
    
    void begin();
    void update();
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected();
    ConnectionState getConnectionState() const { return currentState; }
    
    // Data transmission
    bool sendWeightData(const WeightData& data);
    bool sendStatusUpdate(const String& status);
    
    // Event callbacks
    void setOnCommandCallback(std::function<void(const EdgeCommand&)> callback);
    void setOnConnectionStateCallback(std::function<void(ConnectionState)> callback);
    
    // Configuration
    void setMqttServer(const String& server, int port = 1883);

private:
    void setupTopics();
    void onMqttMessage(char* topic, byte* payload, unsigned int length);
    void setConnectionState(ConnectionState newState);
    void sendHeartbeat();
    
    // Static wrapper for MQTT callback
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    static EdgeCommunication* instance;
};

#endif // EDGE_COMMUNICATION_H
