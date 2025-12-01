# Honda OBD1 UAEFI Firmware

Custom rusEFI firmware for the integrated Honda OBD1 UAEFI ECU board.

## Overview

This firmware is designed for an integrated board combining the UAEFI121 ECU platform with a Honda OBD1 adapter into a single PCB. It provides modern engine management capabilities for Honda OBD1 engines with support for advanced features like electronic throttle control, knock detection, and wideband oxygen sensing.

**Board Specifications:**
- **MCU:** STM32F407VGT (100-pin LQFP, 1MB Flash, 192KB RAM)
- **Platform:** Based on rusEFI UAEFI121 (megamodule form factor)
- **Connector:** Honda OBD1 (A/B/D connectors) + auxiliary expansion connectors

## Features

### ✅ Enabled Features

| Feature | Hardware | Status |
|---------|----------|--------|
| **Knock Sensor** | MAX9924 VR conditioning | ✅ Enabled |
| **EGT Input** | MAX31855 thermocouple interface (SPI3) | ✅ Enabled |
| **CAN Bus** | TJA1051T transceiver (CAN1) | ✅ Enabled |
| **ETB/Stepper** | 2× TLE9201SG H-bridge motor drivers | ✅ Enabled |
| **Wideband O2 #1** | CJ125 + LSU 4.9 | ✅ Enabled |
| **VR Sensors** | MAX9924 conditioning for crank/cam | ✅ Enabled |
| **Ignition Outputs** | 4× direct coil + 1× ICM output | ✅ Enabled |
| **Injector Outputs** | 4× low-impedance injector drivers | ✅ Enabled |
| **Fuel Pressure** | Analog input on PA7 (AUX3) | ✅ Enabled |

### ❌ Disabled Features

These features are disabled due to PA7 pin conflict (see Configuration Notes below):

- SD Card logging
- Accelerometer (LIS2DH12TR)
- Barometer (LPS22HBTR)

### ⚠️ Features Requiring Additional Hardware

- **Wideband O2 #2:** Requires CJ125 controller chip at M11 position
- **CAN2:** Requires second TJA1051T transceiver (not populated)
- **Dumb Coil Drivers:** Requires 6× ISL9V3040D3ST IGBTs (marked DNP on PCB)

## Hardware Pin Conflict (PA7)

The PA7 pin has a hardware conflict and requires choosing between two configurations:

### Current Configuration (Option A)
- **PA7 used for:** Fuel Pressure Sensor (AUX3 analog input)
- **Trade-off:** SD card, accelerometer, and barometer are unavailable

### Alternative Configuration (Option B)
- **PA7 used for:** SPI1_MOSI (enables SD card + accelerometer + barometer)
- **Trade-off:** Fuel pressure must use AUX4 (C5) instead

To switch configurations, see `board_configuration.cpp:54-85` for detailed instructions.

## Building the Firmware

### Prerequisites

- Linux environment (or WSL on Windows)
- ARM GCC toolchain
- Java Runtime Environment (for code generation)
- Git with submodules

### Clone the Repository

```bash
git clone --recursive https://github.com/YOUR_USERNAME/fw-uaefi-Honda-OBD1.git
cd fw-uaefi-Honda-OBD1
```

If you already cloned without `--recursive`, initialize submodules:

```bash
git submodule update --init --recursive
```

### Compile Firmware

```bash
./compile_firmware.sh
```

The build outputs will be in `ext/rusefi/firmware/build/`:
- `rusefi.bin` - Binary firmware file
- `rusefi.hex` - Hex firmware file
- `rusefi.elf` - ELF file with debug symbols

## Flashing the Firmware

### Via DFU (USB)

1. Connect ECU to computer via USB
2. Put ECU into DFU mode (hold button during power-on, or use TunerStudio command)
3. Flash using DFU utility:

```bash
cd ext/rusefi/firmware
make -f bin/Makefile dfu
```

### Via ST-Link

```bash
cd ext/rusefi/firmware
make -f bin/Makefile stlink
```

## Configuration

### Board Configuration

The main board configuration is in:
- `board_configuration.cpp` - Pin assignments and feature configuration
- `connectors/honda-obd1.yaml` - Connector pinout definitions
- `board.mk` - Build configuration

### TunerStudio

TunerStudio configuration files are auto-generated in:
- `generated/tunerstudio/generated/rusefi_uaefi-obd1.ini`

## Connector Pinouts

### OBD1 Connector A (Outputs)
- A1: Injector 1
- A2: Injector 4
- A3: Injector 2
- A5: Injector 3
- A7: Fuel Pump Relay
- A9: Idle Air Control Valve
- A13: MIL (Check Engine Light)
- A21: ICM Coil Control
- And more (see `connectors/honda-obd1.yaml`)

### OBD1 Connector B (Sensor Inputs)
- B10: Vehicle Speed Sensor
- B11: Cam Position Sensor (CYP)
- B15: Crank Position Sensor (CKP)

### OBD1 Connector D (Analog Sensors)
- D11: TPS (Throttle Position)
- D13: CLT (Coolant Temperature)
- D15: IAT (Intake Air Temperature)
- D17: MAP (Manifold Pressure)

### Expansion Connectors
- **J1:** TPS/ETB (6-pin) - Electronic Throttle Body
- **J2:** PPS (6-pin) - Pedal Position Sensors
- **J3:** CAN/UART/LEDs (6-pin)
- **J4:** Ignition Coils (8-pin) - 4× coil outputs + tachometer
- **J5:** VR Sensors (6-pin) - Optional crank/cam inputs

For complete pinout details, see `connectors/honda-obd1.yaml`

## Development

### Directory Structure

```
.
├── board_configuration.cpp    # Main board configuration
├── board.mk                    # Build system configuration
├── connectors/                 # Connector pinout definitions
│   └── honda-obd1.yaml
├── generated/                  # Auto-generated files
├── ext/rusefi/                 # rusEFI submodule
└── compile_firmware.sh         # Build script
```

### Modifying Configuration

1. Edit `board_configuration.cpp` for pin assignments and features
2. Edit `connectors/honda-obd1.yaml` for connector documentation
3. Run `./compile_firmware.sh` to rebuild

## Resources

- **rusEFI Wiki:** https://wiki.rusefi.com/
- **UAEFI Documentation:** https://wiki.rusefi.com/uaefi
- **rusEFI Forum:** https://rusefi.com/forum/
- **Discord:** https://discord.gg/XXXXXXXXX (add your Discord link)

## License

This project is based on rusEFI, which is licensed under GPLv3.

See `ext/rusefi/LICENSE` for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Credits

- Based on [rusEFI](https://github.com/rusefi/rusefi)
- Board design: UAEFI121 platform by rusEFI community
- Honda OBD1 integration: Custom

## Support

For issues and questions:
- Open an issue on GitHub
- Visit the rusEFI forum
- Join the Discord community

---

**⚠️ Disclaimer:** This is experimental engine management software. Use at your own risk. Improper configuration can cause engine damage. Always have proper safety measures and backup systems in place when testing.
