# Tavolo Smart Weight Detection System

## Project Information

**Developer**: Aldo Alberto Baldeon Fabian  
**Company**: Codares  
**Version**: 1.0.0  
**Date**: September 2025  
**License**: CC BY-NC-SA 4.0 (Non-Commercial)  
**Wokwi simulation**: https://wokwi.com/projects/441116631742611457  

## Overview

Tavolo es un sistema embebido IoT inteligente para detección de peso que implementa principios avanzados de programación como:

- **Arquitectura dirigida por eventos (Event-Driven Architecture)**
- **Máquina de estados finitos (Finite State Machine)**
- **Programación reactiva (Reactive Programming)**
- **Principios SOLID**
- **Clean Code**

## Recent Updates (v1.0.0)

### Bug Fixes Implemented:
- ✅ Fixed `WeightSensor::isReady()` const-correctness issue
- ✅ Resolved duplicate `showSystemStatus()` method declarations
- ✅ Corrected `EdgeCommunication::isConnected()` const qualifier
- ✅ Fixed infinite recursion in `getSystemStateString()` method
- ✅ Removed unused libraries (`AsyncMqttClient`, `TaskScheduler`)
- ✅ Added proper state string handling with `stateToString()` helper method

### New Features:
- ✅ Enhanced developer information display on startup
- ✅ Improved boot screen with company branding
- ✅ Better error handling and system stability
- ✅ Clean, production-ready codebase

## Arquitectura del Sistema

### Componentes Principales

1. **TavoloSystem** - Orquestador principal con FSM
2. **WeightSensor** - Interface para sensor HX711
3. **LedActuator** - Control inteligente de LED con patrones
4. **DisplayManager** - Gestión de pantalla LCD
5. **EdgeCommunication** - Comunicación MQTT con Edge API

### Hardware Requirements

- ESP32 DevKit C v4
- Sensor de peso HX711 (50kg)
- LED rojo con resistencia 220Ω
- Pantalla LCD I2C 20x4
- Celda de carga

### Pin Configuration

```
HX711 Weight Sensor:
- DT (Data): Pin 2
- SCK (Clock): Pin 4
- VCC: 5V
- GND: GND

LED Indicator:
- Anode: Pin 5 (through 220Ω resistor)
- Cathode: GND

LCD I2C Display:
- SDA: Pin 21
- SCL: Pin 22
- VCC: 5V
- GND: GND
```

## Funcionalidades

### Estados del Sistema

1. **INITIALIZING** - Inicialización de componentes
2. **CALIBRATING** - Calibración automática del sensor
3. **IDLE** - Sistema listo, esperando actividad
4. **MEASURING** - Midiendo peso activamente
5. **THRESHOLD_EXCEEDED** - Peso excede el umbral configurado
6. **COMMUNICATION_ERROR** - Error de comunicación con Edge
7. **MAINTENANCE** - Modo de mantenimiento

### Patrones LED

- **OFF** - Sistema inactivo
- **ON** - Umbral excedido
- **SLOW_BLINK** - Modo mantenimiento
- **FAST_BLINK** - Error o inicialización
- **PULSE** - Calibrando

### Comunicación Edge API

El sistema se comunica con un Edge API a través de MQTT:

#### Topics MQTT

```
tavolo/{deviceId}/weight   - Envío de datos de peso
tavolo/{deviceId}/command  - Recepción de comandos
tavolo/{deviceId}/status   - Envío de estado del sistema
```

#### Comandos Soportados

- `SET_THRESHOLD` - Establecer umbral de peso
- `LED_ON/LED_OFF` - Control manual del LED
- `TARE` - Realizar tara (poner en cero)
- `CALIBRATE` - Iniciar calibración
- `MAINTENANCE` - Entrar en modo mantenimiento
- `RESUME` - Salir del modo mantenimiento

## Principios de Diseño Implementados

### SOLID Principles

- **S** - Single Responsibility: Cada clase tiene una responsabilidad específica
- **O** - Open/Closed: Extensible sin modificar código existente
- **L** - Liskov Substitution: Las subclases pueden reemplazar a sus clases base
- **I** - Interface Segregation: Interfaces específicas y no monolíticas
- **D** - Dependency Inversion: Dependencias hacia abstracciones

### Clean Code

- Nombres descriptivos y significativos
- Funciones pequeñas y enfocadas
- Comentarios donde añaden valor
- Manejo de errores consistente
- Código autodocumentado

### Event-Driven Architecture

- Callbacks para eventos de peso
- Eventos de cambio de estado
- Comunicación asíncrona
- Reactividad a comandos externos

### Finite State Machine

- Estados claramente definidos
- Transiciones controladas
- Manejo de entrada/salida de estados
- Comportamiento predecible

### Reactive Programming

- Updates no-bloqueantes
- Flujo de datos reactivo
- Callbacks y eventos
- Gestión de tiempo asíncrona

## Configuración y Uso

### Serial Commands

Mientras el sistema está ejecutándose, puedes enviar comandos por Serial:

```
STATUS       - Mostrar estado del sistema
TARE         - Realizar tara
CALIBRATE    - Iniciar calibración
THRESHOLD=X  - Establecer umbral a X gramos
START        - Iniciar mediciones
STOP         - Detener mediciones
HELP         - Mostrar ayuda
```

### WiFi Configuration

Edita en `sketch.ino`:
```cpp
const char* WIFI_SSID = "tu_red_wifi";
const char* WIFI_PASSWORD = "tu_password";
```

### MQTT Configuration

El sistema usa un broker MQTT público por defecto. Para cambiar:

```cpp
// En EdgeCommunication.cpp
String mqttServer = "tu_broker_mqtt.com";
int mqttPort = 1883;
```

## Edge API Integration

El sistema está diseñado para trabajar con un Edge API que:

1. Recibe datos de peso del dispositivo
2. Aplica lógica de negocio compleja
3. Determina si se excede el umbral
4. Envía comandos de vuelta al dispositivo

### Ejemplo de mensaje de peso:

```json
{
  "deviceId": "TAVOLO_ABC123",
  "weight": 150.5,
  "timestamp": 1234567890,
  "type": "weight_data"
}
```

### Ejemplo de comando desde Edge:

```json
{
  "command": "SET_THRESHOLD",
  "value": "200",
  "timestamp": 1234567890
}
```

## Development Guidelines

### Añadir Nuevos Sensores

1. Heredar de la clase base `Sensor`
2. Implementar `begin()`, `read()`, `isReady()`
3. Usar callbacks para notificación de datos
4. Integrar en `TavoloSystem`

### Añadir Nuevos Actuadores

1. Heredar de la clase base `Actuator`
2. Implementar `begin()`, `setState()`, `getState()`
3. Usar callbacks para notificación de cambios
4. Integrar en `TavoloSystem`

### Añadir Nuevos Estados

1. Añadir al enum `SystemState`
2. Implementar lógica en `updateStateMachine()`
3. Manejar entrada/salida en `handleStateEntry()/Exit()`
4. Actualizar `getSystemStateString()`

## Testing

### Simulación en Wokwi

El proyecto está configurado para ejecutarse en Wokwi:
1. Abre el proyecto en wokwi.com
2. Ejecuta la simulación
3. Usa el monitor serial para comandos
4. Simula peso presionando sobre el sensor

### Unit Testing

Para desarrollo local, se recomienda:
- Usar Google Test para unit tests
- Mock de hardware components
- Test de estado machine transitions
- Test de comunicación MQTT

## Future Enhancements

- [ ] Soporte para múltiples sensores
- [ ] Calibración automática inteligente
- [ ] Almacenamiento local de datos
- [ ] OTA (Over-The-Air) updates
- [ ] Interfaz web de configuración
- [ ] Análisis de tendencias de peso
- [ ] Alertas push
- [ ] Dashboard en tiempo real

## Version History

### v1.0.0 (September 2025)
- Initial production release
- Complete implementation of SOLID principles
- Event-driven architecture with FSM
- MQTT Edge API communication
- Comprehensive error handling
- Production-ready stability

## License

MIT License

Copyright (c) 2025 Aldo Alberto Baldeon Fabian - Codares

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributors

**Primary Developer**: Aldo Alberto Baldeon Fabian  
**Company**: Codares  
**Role**: Lead IoT Engineer & System Architect  
**Contributions**: Complete system design, architecture, and implementation  

**Project Scope**: Tavolo IoT Smart Weight Detection System  
**Technologies**: ESP32, C++, MQTT, Event-Driven Architecture, SOLID Principles
