#include "EdgeCommunication.h"

// Static instance for callback
EdgeCommunication* EdgeCommunication::instance = nullptr;

EdgeCommunication::EdgeCommunication(const String& deviceId) 
    : mqttClient(wifiClient), deviceId(deviceId) {
    instance = this;
    clientId = "tavolo_" + deviceId;
    setupTopics();
}

void EdgeCommunication::begin() {
    Serial.println("Initializing Edge Communication...");
    
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    mqttClient.setCallback(mqttCallback);
    
    Serial.print("MQTT Server: ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.println(mqttPort);
    Serial.print("Client ID: ");
    Serial.println(clientId);
    
    Serial.println("Edge Communication initialized.");
}

void EdgeCommunication::update() {
    unsigned long currentTime = millis();
    
    // Handle MQTT loop
    if (mqttClient.connected()) {
        mqttClient.loop();
        
        // Send heartbeat periodically
        if (currentTime - lastHeartbeat >= HEARTBEAT_INTERVAL) {
            sendHeartbeat();
            lastHeartbeat = currentTime;
        }
    } else {
        setConnectionState(ConnectionState::DISCONNECTED);
        
        // Attempt reconnection
        if (currentTime - lastConnectionAttempt >= RECONNECT_INTERVAL) {
            Serial.println("Attempting to reconnect to MQTT...");
            connect();
            lastConnectionAttempt = currentTime;
        }
    }
}

bool EdgeCommunication::connect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, cannot connect to MQTT");
        setConnectionState(ConnectionState::ERROR);
        return false;
    }
    
    setConnectionState(ConnectionState::CONNECTING);
    Serial.print("Connecting to MQTT broker: ");
    Serial.println(mqttServer);
    
    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected to MQTT broker");
        
        // Subscribe to command topic
        if (mqttClient.subscribe(commandTopic.c_str())) {
            Serial.print("Subscribed to: ");
            Serial.println(commandTopic);
        }
        
        setConnectionState(ConnectionState::CONNECTED);
        
        // Send initial status
        sendStatusUpdate("CONNECTED");
        
        return true;
    } else {
        Serial.print("Failed to connect to MQTT. State: ");
        Serial.println(mqttClient.state());
        setConnectionState(ConnectionState::ERROR);
        return false;
    }
}

void EdgeCommunication::disconnect() {
    if (mqttClient.connected()) {
        sendStatusUpdate("DISCONNECTING");
        mqttClient.disconnect();
    }
    setConnectionState(ConnectionState::DISCONNECTED);
}

bool EdgeCommunication::isConnected() {
    return currentState == ConnectionState::CONNECTED && mqttClient.connected();
}

bool EdgeCommunication::sendWeightData(const WeightData& data) {
    if (!isConnected()) {
        Serial.println("Cannot send weight data: not connected");
        return false;
    }
    
    StaticJsonDocument<256> doc;
    doc["deviceId"] = data.deviceId;
    doc["weight"] = data.weight;
    doc["timestamp"] = data.timestamp;
    doc["type"] = "weight_data";
    
    String payload;
    serializeJson(doc, payload);
    
    bool success = mqttClient.publish(weightTopic.c_str(), payload.c_str());
    
    if (success) {
        Serial.print("Weight data sent: ");
        Serial.print(data.weight);
        Serial.println("g");
    } else {
        Serial.println("Failed to send weight data");
    }
    
    return success;
}

bool EdgeCommunication::sendStatusUpdate(const String& status) {
    if (!isConnected()) {
        return false;
    }
    
    StaticJsonDocument<256> doc;
    doc["deviceId"] = deviceId;
    doc["status"] = status;
    doc["timestamp"] = millis();
    doc["type"] = "status_update";
    
    String payload;
    serializeJson(doc, payload);
    
    return mqttClient.publish(statusTopic.c_str(), payload.c_str());
}

void EdgeCommunication::setOnCommandCallback(std::function<void(const EdgeCommand&)> callback) {
    onCommandCallback = callback;
}

void EdgeCommunication::setOnConnectionStateCallback(std::function<void(ConnectionState)> callback) {
    onConnectionStateCallback = callback;
}

void EdgeCommunication::setMqttServer(const String& server, int port) {
    mqttServer = server;
    mqttPort = port;
    mqttClient.setServer(server.c_str(), port);
}

void EdgeCommunication::setupTopics() {
    String baseTopicName = deviceId;
    baseTopicName.replace(":", "_"); // Replace colons with underscores for topic compatibility
    
    weightTopic = "tavolo/" + baseTopicName + "/weight";
    commandTopic = "tavolo/" + baseTopicName + "/command";
    statusTopic = "tavolo/" + baseTopicName + "/status";
    
    Serial.println("MQTT Topics configured:");
    Serial.println("Weight: " + weightTopic);
    Serial.println("Command: " + commandTopic);
    Serial.println("Status: " + statusTopic);
}

void EdgeCommunication::onMqttMessage(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print("Received MQTT message on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(message);
    
    // Parse JSON command
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }
    
    EdgeCommand command;
    command.command = doc["command"].as<String>();
    command.value = doc["value"].as<String>();
    command.timestamp = doc["timestamp"] | millis();
    
    if (onCommandCallback) {
        onCommandCallback(command);
    }
}

void EdgeCommunication::setConnectionState(ConnectionState newState) {
    if (currentState != newState) {
        currentState = newState;
        
        Serial.print("Edge connection state changed to: ");
        switch (newState) {
            case ConnectionState::DISCONNECTED:
                Serial.println("DISCONNECTED");
                break;
            case ConnectionState::CONNECTING:
                Serial.println("CONNECTING");
                break;
            case ConnectionState::CONNECTED:
                Serial.println("CONNECTED");
                break;
            case ConnectionState::ERROR:
                Serial.println("ERROR");
                break;
        }
        
        if (onConnectionStateCallback) {
            onConnectionStateCallback(newState);
        }
    }
}

void EdgeCommunication::sendHeartbeat() {
    StaticJsonDocument<128> doc;
    doc["deviceId"] = deviceId;
    doc["type"] = "heartbeat";
    doc["timestamp"] = millis();
    
    String payload;
    serializeJson(doc, payload);
    
    mqttClient.publish(statusTopic.c_str(), payload.c_str());
}

// Static callback wrapper
void EdgeCommunication::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->onMqttMessage(topic, payload, length);
    }
}
