# Smart Fluid Despensor using ESP32

## Description
A smart liquid dispenser dispenses liquid specified in application and it fetches required amount of milliliters to another container.

## List of components:
-> ESP32-S3-WROOM-2
-> L298N Motor Driver
-> 12V Water Uni-Directional Pump
-> Flow Sensor
-> 12K and 8k resistor (doubtful need to confirm with vasoya by checking)
-> Power Supply (12V, 5V)

## Pin-out configuration:

## Circuit Diagram:

## Approach:
### Calibration:
- Excel File with plots of pulse per litres used in algorithm.

### Circuit:
- Level shifter using a MOSFET (Fixed with L298N) 
- Potential divider for PWM (step down from 5v to 3.3V) (Flow Sensor Test)

### Code:
- Interrupt (Flow sensor)
- Started with BLE module but then shifted to Wifi instead
- App (Wifi.h, etc.)
- MIT App Blocks

## Final Results:
Video


## To-do list:
- [x] Calibration
- [x] Mobile App  
- [x] Comm with Wifi
- [ ] System intergrationa and testing
- [ ] Demo video 
- [ ] Presentation

## References
