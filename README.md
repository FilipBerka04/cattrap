# Cat Trap Controller

## Overview

This project is a smart cat trap controller developed by my friend. It uses an ESP32-S3 microcontroller to create a local Wi-Fi network and host a web-based user interface. The system allows users to monitor the trap in real-time via a live camera feed and remotely trigger a servo-operated door to safely catch the cat. It is powered through the usb port using a regular powerbank.

## Implementation Details

* **Mechanics:** The device operates as a standalone Wi-Fi Access Point. The web UI features a live video feed and a safety lock button to prevent accidentally closing the trap.
* **Entities:** The system relies on an ESP32-S3 camera for video, an LED strip for adjustable lighting, and a servo motor for the door mechanism.
* **Control:** The web server handles specific endpoints to stream video (`/stream`), close the trap (`/close`), and reset the trapdoor (`/reset`). It also features an auto-power-off timer for the servo to limit the battery usage and periodically request power from the power bank to make sure it doesn't go into stand by mode.

## Constraints and System Requirements

* The code relies on several specific Arduino libraries: `esp_camera`, `WiFi`, `WebServer`, `Adafruit_NeoPixel`, and `ESP32Servo`.
* To operate the trap, you must connect a smartphone or computer directly to the device's local Wi-Fi network.
