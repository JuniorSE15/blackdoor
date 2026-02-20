# Smart Access Lock System

A security device designed to bridge the gap between traditional physical security and modern, seamless home automation. This project utilizes **Ultra-Wideband (UWB)** technology to achieve centimeter-level precision for hands-free proximity unlocking.

##  Overview

Unlike standard smart locks that rely on Bluetooth (which often suffers from latency and poor range accuracy), this system uses the **DW3000 UWB sensor** to ensure the lock only triggers when an authorized token is within a specific, tight range.

### Key Features

* **Precision Proximity Unlocking:** Hands-free entry using UWB technology with ±10 cm accuracy.


* **Multi-Layered Authentication:** Support for physical RFID tokens and manual PIN entry via a capacitive touch pad.


* **State Awareness:** Integrated **Reed Switch** to detect the physical position of the door, preventing the solenoid from engaging if the door is ajar.


* **Auto-Lock Security:** Automatically secures the door 5 seconds after it has been closed.


* **Safety Lockout:** Implements a "Wrong Try Penalty" that disables inputs for 60 seconds after 5 consecutive failed attempts.

---

## System Architecture

The project follows a **Centralized Embedded Control Architecture** based on the Input-Process-Output model:

1. **Authentication & Sensors:** Reed Switch, Capacitive Touch Pad, RFID Reader, and DW3000 UWB Sensor.


2. **Central Processing Unit:** ESP32 or STM32 microcontroller acting as the decision-making engine.


3. **Actuators & Feedback:** 12V Solenoid Lock, Status LEDs (Red/Green), and Auditory Feedback.


4. **External Interfaces:** UART Serial Debug interface for real-time status logging and maintenance.

---

## Requirements

### Hardware

* **Microcontroller:** ESP32 or STM32.


* **UWB Module:** Decawave DW3000.


* **Security Inputs:** RFID Reader (SPI/I2C/UART) and Capacitive Touch Pad.


* **Physical Hardware:** 12V Solenoid Lock, Relay Module, and Reed Switch.


* **Power:** 12V Power Supply with a Buck Converter for logic voltage (3.3V/5V).

### Software

* **Firmware:** Developed with modular architecture (separating UWB, RFID, and Logic functions).


* **Communication:** SPI for DW3000 and RFID; GPIO for door status and relay control.


---

## Project Plan

The development is executed in four phases:

* **Phase 1:** System Design & Component Research.
* **Phase 2:** Hardware Prototyping (Component Interfacing).
* **Phase 3:** Logic Integration & Firmware Development.
* **Phase 4:** Enclosure Design (3D Printed) & Final Testing.

---

## 👥 Team Members

* **Pannawat Yorkhant** (66011111) [Teemy17](https://github.com/Teemy17)


* **Naphat Umpa** (66011276) [Umbs](https://github.com/Umbs01)


* **Tanakrit Doltanakarn** (66011249) [TrizzlyBare](https://github.com/TrizzlyBare)


**Department of Computer Engineering, King Mongkut's Institute of Technology Ladkrabang (KMITL)** 
