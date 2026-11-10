
# Boardoza Smart Home Four-Channel Wireless Control Project

The **Boardoza Smart Home Project** is an ESP32-based home automation demonstrator designed to provide centralized control of household devices through a 7-inch touchscreen tablet. The system combines an **ESP32-S3 microcontroller**, an **FT810 graphics controller**, and an **ESP32-H2 four-channel relay board** connected through Bluetooth Low Energy (BLE). Four independent relay channels control air conditioning, lighting, a smart curtain, and a music player. The touchscreen features large interactive control cards displaying device icons, channel numbers, and names, with green and red indicators representing the most recently successful ON and OFF commands.

The system features a **7-inch TFT touchscreen with 800 × 480 resolution** and supports wireless communication through BLE, with an alternative Zigbee firmware configuration for network-based relay control. An **ESP-01M module** manages an **8 × 8 RGB LED matrix** featuring five automatic animations and controls a Mini MP3 Player through a **9600-baud serial connection**. The touchscreen provides wireless device scanning, connection status monitoring, and individual channel control. The project is suitable for STEM education, smart home demonstrations, IoT development, and embedded systems prototyping.

<h2 align="center">Smart Home</h2>

<p align="center">
  <img src="./assets/Smart Home.gif" width="400">
</p>

---

## Key Features

- **Interactive Touchscreen Control:** Features a 7-inch TFT touchscreen with four dedicated control cards displaying device icons, channel numbers, and names, allowing users to operate household devices through a centralized interface.
- **Wireless BLE Communication:** Establishes a Bluetooth Low Energy connection between the ESP32-S3 touchscreen controller and ESP32-H2 relay board using an 8-second scanning process with device identification through advertised names or service IDs.
- **Four Independent Relay Channels:** Provides individual ON/OFF control of air conditioning, lighting, a smart curtain, and a music player through four independently assigned relay channels.
- **Connection and Command Status:** Displays connected, disconnected, scanning, and device-not-found states, with green and red indicators representing the last successfully written ON/OFF commands rather than physically measured appliance operation.
- **Animated RGB LED Matrix:** Integrates an ESP-01M-controlled 8 × 8 RGB matrix featuring five automatically cycling animations — KITT Scanner, Beating Heart, Diamond Rings, Rainbow Flow, and Star Field with Shooting Star — each running for 6.5 seconds.
- **Integrated Music Playback:** Uses an ESP-01M module to control a Mini MP3 Player over a 9600-baud serial connection, continuously looping the `0001.mp3` audio file from a microSD card while updating LED animations.
- **Alternative Zigbee Connectivity:** Provides separate ESP32-H2 firmware with four Zigbee On/Off endpoints (1–4), enabling network-based relay operation as an alternative to BLE touchscreen control.

---

## System Architecture

The Smart Home system combines an ESP-01M controller, an MP3 player, a cooling fan, a DC motor, an 8 × 8 ARGB LED display module, and a voltage regulator to create an interactive home-control demonstrator with lighting, motion, and audio functions.

| Component | Model |
|---|---|
| Microcontroller | [Boardoza ESP01M](https://github.com/Boardoza/Boardoza_ESP-01M_Wi-Fi_Module) |
| Microcontroller | [Boardoza ESP32-H2 4-Channel ](https://boardoza.com/tr/products/boardoza-esp32-h2-4-channel-wireless-relay-evaluation-board) |
| MP3 Player | [DFR0299](https://www.ozdisan.com/p/arduino-sensorleri-ve-modulleri-613/dfrobot-dfr0299-727062?s=mini+mp3&tab=productDetails) |
| Fan | [PF6020D24HBL 0.10A](https://www.ozdisan.com/p/genel-tip-fanlar-450/pf-motor-fan-pf6020d24hbl-0-10a-1202399?catSequenceId=450&tab=productDetails) |
| Motor | [12V 600 RPM DC Motor](https://www.ozdisan.com/p/dc-motorlar-832/oem-12v-600-rpm-dc-motor-1575407?catSequenceId=832&tab=productDetails) |
| ARGB LED | [Boardoza 8×8 ARGB Display Module](https://github.com/Boardoza/Boardoza_8x8_ARGB_Display_WS2812B_LED_Matrix_Breakout_Board) |

---

---

## Board Pinout

[Boardoza Smart Home Pinout Diagram](./assets/Smart%20Home%20Pinout%20Diagram.pdf)

---

## Step Files

[Boardoza Smart Home.step](./assets/Smart%20Home.step)

---

## Version History

- v1.0 – Initial release

---

## Support

- For questions or technical support, please contact **support@boardoza.com**

---

## License

This repository contains both hardware and software components:

### **Hardware Design**

[![CC BY-SA 4.0][cc-by-sa-shield]][cc-by-sa]

All hardware design files are licensed under [Creative Commons Attribution-ShareAlike 4.0 International License][cc-by-sa].

[cc-by-sa]: http://creativecommons.org/licenses/by-sa/4.0/
[cc-by-sa-shield]: https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg

### **Software/Firmware**

[![BSD-3-Clause][bsd-shield]][bsd]

All software and firmware are licensed under [BSD 3-Clause License][bsd].

[bsd]: https://opensource.org/licenses/BSD-3-Clause
[bsd-shield]: https://img.shields.io/badge/License-BSD%203--Clause-blue.svg
