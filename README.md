# Moza Custom Wheel

A DIY solution to create custom steering wheels for Moza Wheelbases using an Arduino Pro Micro as an emulator.

## Disclaimer
> Modding your hardware can void your warranty. Use this project at your own risk. Be careful with the voltage settings on the step-down converter!
> **Crucial:** You must calibrate the step-down converter output voltage BEFORE connecting it to the Arduino to avoid permanent damage!
---
## Requirements
### Buy
- 4x [6.5mm Pogo Pins](https://de.aliexpress.com/item/1005008633095807.html)
- 1x [Arduino Pro Micro](https://de.aliexpress.com/item/1005009320469220.html) (5V/16MHz recommended)
- 1x [Stepdown](https://www.amazon.de/dp/B0CGVMRQXB)
- 1x [70mm Quick-Release](https://de.aliexpress.com/item/1005010038135486.html)
- 2x [M3 6mm](https://de.aliexpress.com/item/4000239264456.html) | PCB Pogo
- 6x [M3 Nut]() | Handle
- 6x [M3 25mm](https://de.aliexpress.com/item/4000239264456.html) | Handle
- 4x [M3 8mm](https://de.aliexpress.com/item/4000239264456.html) | Base
- 6x [M5 40mm](https://de.aliexpress.com/item/1005008846077891.html) | Wheel QR
- 12x [Button]()

### Print
- 1x Wheel Wheel Box
- 1x Wheel Wheel Base
- 1x Wheel Handle Left Bottom
- 1x Wheel Handle Right Bottom
- 1x Wheel Handle Left Top
- 1x Wheel Handle Right Top
- 1x Spacer Pi Stepdown
#### Option A: With custom PCB
- 1x Spacer PCB Spacer
#### Option B: With Printed "PCB"
- 1x Connector Pogo Pin 6.5mm
- 1x Connector Pogo Pin 6.5mm V2
- 1x Connector Pogo Pin 7mm

### Optional
- Connector PCB Orderd by JLCPCB
- Wheel Wheel Base Ordered by JLCCNC

### Software Requirements
- Arduino IDE
- [Code](Code/Code.ino)
---

## Documentation & Pinouts
- [Pro Micro Pinout](Documentation/Pinouts/Pro_Micro.md)
- [Base Pinout](Documentation/Pinouts/R9_V2.md)
- [Images](Documentation/Images)
- [Gerber Files Connector](Documentation/PCB/GERBER-Connector.zip)
- [Wiring Diagrams](Documentation/PCB)
- [3D Files](Documentation/3D)
- [Code](Code/Code.ino)

## Credits & Resources
- [Original Post](https://www.reddit.com/r/simracing/comments/1j4k9dj/reverse_engineered_moza_es_wheel/)
- [Github Link](https://github.com/MikeSzklarz/Arduino-Moza-Emulator)
- [More Images](https://imgur.com/a/arduino-moza-wheel-yghlPRm)
- [Original 3D Files](https://www.thingiverse.com/thing:4058270/files)
- [Base-3D Print Split](https://www.thingiverse.com/thing:4891321/files)

# Getting Started

1. **Print** all required files or order parts from a CNC/PCB service.
2. **Flash** `Code.ino` to the Arduino Pro Micro using the Arduino IDE.
3. **Step-down Calibration:** Connect the Step-down to the Wheelbase power pins (12V or 24V) and adjust the potentiometer until it outputs exactly **3.3V** (or 5V depending on your MCU/Setup). **Do this before connecting the Arduino!**
4. **Soldering:**
   - Solder the Pogo-Pin connector pins.
   - Connect the calibrated Step-down output to the Arduino (VCC/GND).
   - Solder SDA & SCL lines from the base to the Arduino (Pins 2 & 3).
   - Solder your Buttons and Shifters to the Arduino pins.
5. **Final Assembly:** Mount all components into the wheel box and screw everything together.
6. **Connection:** Connect your wheel to the Moza Base and check for recognition in Moza Pit House.

# Further Information
- **LED/Input Logic:** In this setup, I used the pins to read a **HIGH** signal for the buttons.
- **Manufacturing:** For a more rigid feel, I highly recommend ordering the Base and the Connector PCB from JLC (CNC/PCB).
