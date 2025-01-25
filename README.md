# Smart Fluid Despensor using ESP32

## Description
A smart liquid dispenser dispenses liquid specified in application and it fetches required amount of milliliters to another container.

## List of components:
- ESP32-S3-WROOM-2
- L298N Motor Driver
- 5V Water Uni-Directional Pump
- Flow Sensor
- 12K and 8k resistor
- Power Supply (12V, 5V)

## Pin-out configuration:

This project implements a liquid filling system using an ESP32. It controls a pump and monitors liquid flow using a flow sensor. The system allows users to fill a target volume of liquid through a web interface.

## Pin Configuration

| **Pin**              | **Purpose**                     | **Direction** | **Details**                                      |
|-----------------------|---------------------------------|---------------|-------------------------------------------------|
| `PUMP_PIN (40)`       | Pump relay control             | Output        | Controls the pump's ON/OFF state.              |
| `FLOW_SENSOR_PIN (4)` | Flow sensor input              | Input         | Reads pulses from the flow sensor.             |

## Features
- **Start Filling**: Specify the volume of liquid to fill (1–250 mL).
- **Reset System**: Reset the source and sink values to defaults.
- **Web Interface**: Control the system through HTTP requests.

## HTTP Endpoints

| **Endpoint**         | **Description**                                | **Example Request**              |
|-----------------------|-----------------------------------------------|----------------------------------|
| `/start?volume=X`     | Start filling the specified volume (in mL).   | `http://<IP_ADDRESS>/start?volume=100` |
| `/reset`              | Reset source and sink values.                 | `http://<IP_ADDRESS>/reset`      |

## System Variables

| **Variable**          | **Description**                                |
|-----------------------|-----------------------------------------------|
| `source`              | Available liquid in mL.                       |
| `sink`                | Collected liquid in mL.                       |
| `totalLiters`         | Total liquid filled in liters (real-time).     |

## Setup Instructions
1. **Hardware**: Connect the pump relay and flow sensor to the appropriate pins as described in the pin configuration.
2. **Code**:
   - Replace `ssid` and `password` with your WiFi credentials.
   - Adjust `PULSES_PER_LITER` for your specific flow sensor, based on its datasheet and continuous testing.
3. **Compile and Upload**:
   - Use Arduino IDE or PlatformIO to compile and upload the code to your ESP32 board.
4. **Run**:
   - Access the system via the ESP’s IP address in your browser.

## Circuit Diagram:
![WhatsApp Image 2025-01-23 at 16 24 44_c0bc79d0](https://github.com/user-attachments/assets/febd2995-a222-4754-806d-06a7e1062639)


## Approach:
### Calibration:
![Screenshot 2025-01-24 213614](https://github.com/user-attachments/assets/e621dc3b-4924-4070-89d4-34c3c32594ca)

## Final Results:
https://youtu.be/nC2cU2fNJ0U

## To-do list:
- [x] Calibration
- [x] Mobile App  
- [x] Comm with Wifi
- [x] System intergrationa and testing
- [x] Demo video 
- [x] Presentation

## References
