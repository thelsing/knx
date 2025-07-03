# KNX Demo DIY (ESP-IDF 5.x Native)

This is a native ESP-IDF 5.x example project for KNX, based on the Arduino `knx-demo-diy` example but using the new `Esp32IdfPlatform` for direct ESP-IDF support.

## Features
- Uses the native ESP-IDF APIs (no Arduino layer)
- Demonstrates KNX stack integration on ESP32
- Based on the logic of the Arduino `knx-demo-diy.ino` example

## How to Build

1. Install [ESP-IDF 5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
2. Open a terminal in this directory (`examples/knx-demo-esp-idf`)
3. Run:
   ```sh
   idf.py set-target esp32
   idf.py build
   idf.py -p /dev/ttyUSB0 flash monitor
   ```
   (Replace `/dev/ttyUSB0` with your ESP32 serial port)

## Project Structure
- `main.c` — Main application file (C++ code, but named .c for ESP-IDF compatibility)
- `CMakeLists.txt` — ESP-IDF build configuration

## Notes
- This project uses the new `Esp32IdfPlatform` class for native ESP-IDF support.
- You may need to adapt pin numbers and KNX configuration for your hardware.
- The logic is adapted from the Arduino `knx-demo-diy.ino` example. 