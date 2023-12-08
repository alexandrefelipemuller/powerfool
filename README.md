![](https://raw.githubusercontent.com/alexandrefelipemuller/powerfool/main/assets/Powerfool.jpg)


An Arduino Automobile Programmable Module

What is
---------------
It is a low cost automotive module used for many feature you want. The main idea is using Arduino Nano. It's not, at least not at first objective, a fuel injection and  spark management system. But, as it is opensource, the main idea is allow you to do anything.

Features
---------------
* Speedometer fix, by changing wheel size or other difference it can drift the signal
* Oil and fuel pressure sensor and warnings
* old Renault/General Motors Computer Board with distance per consume and autonomy (may work on other manufacturers)
* Shift light, shift stage, low battery warning, etc...
* Air temperature and oxygen sensor adjustment for tuning
* Emergency stop signal

Where to buy
---------------
You can make your own hardware using diagrams and source code on this site.

Support
---------------
Please contact autor and use Wiki here no Github

Diagram
---------------
![](https://github.com/alexandrefelipemuller/powerfool/blob/main/diagrams/Schematic_Modulo%20AutoDuino_2023-01-04.png?raw=true)
A better explanation here: [Schematic Overview](Schematic.md)

# Understanding the Arduino Code for Vehicle Monitoring

This Arduino sketch (a term for an Arduino program) is designed for a vehicle or engine management system. It uses various sensors to monitor and control the engine's performance. Let's break down what each part does:

## 1. Sensor Inputs and Actuator Outputs:
   - The code sets up digital and analog inputs and outputs (I/O). Inputs read data from the environment (like a button press or a sensor reading), while outputs allow the Arduino to interact with other devices (like turning on a light or activating a motor).

## 2. Display and Bluetooth Communication:
   - The system is capable of displaying information on an LCD screen and communicating via Bluetooth. This might be used to show real-time data or for remote diagnostics.

## 3. Reading Sensor Values:
   - Sensors for pressure (`sensorPressure`), intake air temperature (`intakeAirTemp`), and another pressure sensor (`sensorPressure2`) are read by the Arduino. These measurements are crucial for understanding how well the engine is running.

## 4. Fuel Consumption Calculation:
   - The code computes fuel consumption based on the injector's pulsing frequency and their flow rate, which is defined as 20 lbs/h at 12V.

## 5. Speed Management and Alerts:
   - There are functions to handle speed limits, automatic door locking at certain speeds, and emergency braking indicators.

## 6. Odometer and Mileage Tracking:
   - The system keeps track of distance traveled and fuel used, functioning as an odometer and trip meter, which are saved in non-volatile memory (EEPROM) to be retained even after the power is turned off.

## 7. RPM and Pressure Management:
   - Alerts are generated if the engine's RPM exceeds a set limit or if sensor pressure drops below a certain threshold while at high RPMs.

## 8. Diagnostic Mode:
   - A diagnostic mode can display detailed reports on the LCD, which could be helpful for troubleshooting.

## 9. Timers and Pulse Outputs:
   - Timers are used to precisely manage pulse outputs for controlling various timed functions, possibly related to the engine control.

### Code Components:

- `#define`: These lines define constants or settings that can be easily changed at the top of the code.
- `#include`: This includes libraries that add functionality, like controlling an LCD screen or handling Bluetooth communication.
- Variables: Variables like `sensorPressure` are named locations in memory where sensor data is stored.
- `setup()`: A function that runs once when the Arduino is powered on. It's used to initialize I/O pins and set up communication.
- `loop()`: This function runs repeatedly, forming the main part of the program. It reads sensors, calculates values, and responds to changes.
- Functions: These are blocks of code that perform specific tasks, like `calculateDistance()` or `alertsManager()`, and can be called whenever those particular tasks need to be performed.

### EEPROM Memory:
- The EEPROM is a type of memory that retains its contents even when the power is turned off. It's used here to store important values like total mileage and settings.

### Arduino Pins:
- Pins are the points on the Arduino board where you connect sensors and actuators. In the code, `A0`, `A1`, etc., refer to analog input pins, and numbers like `10`, `11`, `12` refer to digital I/O pins.

## In Conclusion:
This sketch is a comprehensive program for a vehicle monitoring system. It's designed to interact with the physical world through sensors and actuators, perform calculations, and save important information for future use. For a beginner, understanding this code can be a fantastic introduction to the world of embedded systems and automotive electronics.

---

*Remember, the best way to learn is by doing. So, if you're a beginner, try uploading this code to an Arduino and see what each part does. Change values, see how the system responds, and get a feel for how software can control hardware.*

