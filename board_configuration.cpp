/**
 * Honda OBD1 UAEFI Board Configuration
 *
 * This is an integrated board combining UAEFI121 ECU with Honda OBD1 adapter
 * MCU: STM32F407VGT (100-pin LQFP, 1MB Flash, 192KB RAM)
 *
 * ============================================================================
 * ENABLED FEATURES (Hardware present and configured):
 * ============================================================================
 * ✓ Knock Sensor Detection (MAX9924 VR conditioning + software knock)
 * ✓ EGT Thermocouple Input (MAX31855 on SPI3)
 * ✓ CAN Bus (TJA1051T transceiver on CAN1)
 * ✓ Electronic Throttle Body - 2× TLE9201SG H-bridge drivers
 * ✓ Wideband O2 #1 (CJ125 + LSU 4.9)
 * ✓ VR Sensor Conditioning (MAX9924 for crank/cam)
 * ✓ 4× Ignition Coil Outputs (for direct coil control)
 * ✓ ICM Output (for distributor-based ignition)
 * ✓ 4× Fuel Injector Outputs
 * ✓ All OEM Honda OBD1 sensor inputs (TPS, MAP, CLT, IAT, etc.)
 *
 * ============================================================================
 * DISABLED FEATURES (Hardware conflict - see PA7 notes below):
 * ============================================================================
 * ✗ SD Card (requires SPI1, conflicts with PA7 fuel pressure sensor)
 * ✗ Accelerometer LIS2DH12TR (requires SPI1)
 * ✗ Barometer LPS22HBTR (requires SPI1)
 *
 * ============================================================================
 * FEATURES REQUIRING ADDITIONAL HARDWARE:
 * ============================================================================
 * ⚠ Wideband O2 #2 - Requires CJ125 controller chip at M11 position
 * ⚠ CAN2 - Requires second TJA1051T transceiver chip (not populated)
 * ⚠ Dumb Coil Drivers - Requires 6× ISL9V3040D3ST IGBTs (marked DNP)
 *
 * ============================================================================
 * PA7 PIN CONFLICT RESOLUTION:
 * ============================================================================
 * PA7 is shared between:
 *   - SPI1_MOSI (for SD card + accelerometer + barometer)
 *   - MM100_IN_AUX3_ANALOG (for fuel pressure sensor)
 *
 * Current Configuration: PA7 used for FUEL PRESSURE sensor
 * Alternative: See customBoardConfigOverrides() OPTION B to use SPI1 instead
 *
 * See customBoardConfigOverrides() function for detailed trade-off explanation
 * ============================================================================
 */

#include "pch.h"
#include "mega-uaefi.h"
#include "hellen_meta.h"
#include "board_overrides.h"

// Override function to run AFTER mega-uaefi config
static void customBoardConfigOverrides() {
	// Call the base mega-uaefi overrides first
	setMegaUaefiBoardConfigOverrides();

	// ========== PA7 CONFLICT RESOLUTION ==========
	// PA7 has a hardware conflict: used by SPI1_MOSI (for SD card)
	// AND available as MM100_IN_AUX3_ANALOG for fuel pressure sensor
	//
	// OPTION A (CURRENT): Disable SPI1 to use PA7 for fuel pressure
	//   - GAINS: Fuel pressure sensor on PA7/AUX3
	//   - LOSES: SD card, Accelerometer, Barometer
	//
	// OPTION B: Keep SPI1 enabled (comment out the disabling code below)
	//   - GAINS: SD card logging, Accelerometer, Barometer (LPS22HBTR)
	//   - LOSES: Cannot use PA7 for fuel pressure sensor
	//   - Alternative: Use MM100_IN_AUX4_ANALOG (C5) for fuel pressure instead
	//
	// Current choice: OPTION A (disable SPI1, enable fuel pressure on PA7)
	engineConfiguration->isSdCardEnabled = false;
	engineConfiguration->is_enabled_spi_1 = false;
	engineConfiguration->spi1mosiPin = Gpio::Unassigned;
	engineConfiguration->spi1misoPin = Gpio::Unassigned;
	engineConfiguration->spi1sckPin = Gpio::Unassigned;
	engineConfiguration->sdCardCsPin = Gpio::Unassigned;

	// Configure low pressure fuel sensor on PA7 (AUX3) - only works when SPI1 is disabled
	engineConfiguration->lowPressureFuel.hwChannel = MM100_IN_AUX3_ANALOG;

	// NOTE: To switch to OPTION B, comment out the 7 lines above and uncomment below:
	// engineConfiguration->lowPressureFuel.hwChannel = MM100_IN_AUX4_ANALOG; // Use C5 instead of PA7
}

// board-specific configuration setup
static void customBoardDefaultConfiguration() {
    setMegaUaefiBoardDefaultConfiguration();

	// ========== KNOCK SENSOR ==========
	// Hardware: MAX9924 VR conditioning circuit + knock sensor input
	// Enabled: knock detection using ADC input
	engineConfiguration->enableSoftwareKnock = true;

	// ========== EGT (EXHAUST GAS TEMPERATURE) ==========
	// Hardware: MAX31855 thermocouple interface on SPI3
	// Chip select: A15, MISO: C11, SCK: C10
	engineConfiguration->is_enabled_spi_3 = true;
	engineConfiguration->spi3misoPin = Gpio::C11;
	engineConfiguration->spi3sckPin = Gpio::C10;
	// MOSI not needed for MAX31855 (read-only communication)
	engineConfiguration->max31855_cs[0] = Gpio::A15;
	engineConfiguration->max31855spiDevice = SPI_DEVICE_3;

	// ========== ACCELEROMETER & BAROMETER ==========
	// Hardware: LIS2DH12TR (accelerometer) + LPS22HBTR (barometer) on SPI1 CS2
	// Status: Currently DISABLED due to SPI1 being disabled to free PA7
	// These sensors are automatically initialized by rusEFI when SPI1 is enabled
	// To enable: Switch to OPTION B in customBoardConfigOverrides() above
	// Build flags: EFI_ONBOARD_MEMS=TRUE, ONBOARD_MEMS_TYPE=LIS2DH12

	// ========== CAN BUS ==========
	// Hardware: TJA1051T CAN transceiver on CAN1
	// Already configured by mega-uaefi.cpp: TX=B13, RX=B5
	// Status: ENABLED and ready to use (configured in setMegaUaefiBoardConfigOverrides)
	// Note: CAN2 requires additional hardware - second TJA1051T chip not populated

	// ========== ELECTRONIC THROTTLE BODY (ETB) ==========
	// Hardware: 2× TLE9201SG H-bridge motor drivers (M7/M847 and M8/M848)
	// Already configured by mega-uaefi.cpp via setupTLE9201IncludingStepper()
	// Driver 1: PWM=MM100_OUT_PWM3, Dir1=MM100_OUT_PWM4, Dir2=MM100_SPI2_MISO
	// Driver 2: PWM=MM100_OUT_PWM5, Dir1=MM100_SPI2_MOSI, Dir2=MM100_USB1ID
	// Status: ENABLED - Hardware present and pins configured for ETB or stepper motor control
	// Connectors: J1 pins for ETB motor connections (see honda-obd1.yaml)

	// ========== WIDEBAND O2 SENSORS ==========
	// Hardware: LSU 4.9 wideband with dual CJ125 controller positions (M2 and M11)
	// WBO #1: POPULATED and functional (connector C1-C6)
	// WBO #2: Hardware ready but requires CJ125 controller chip to be soldered (M11 position)
	// Status: WBO #1 ENABLED, WBO #2 requires hardware installation

	// ========== Honda OBD1 Specific Configuration ==========
	engineConfiguration->injectionPins[0] = Gpio::MM100_MEGA_UAEFI_INJ1;
  	engineConfiguration->injectionPins[1] = Gpio::MM100_INJ2;
  	engineConfiguration->injectionPins[2] = Gpio::MM100_INJ3;
  	engineConfiguration->injectionPins[3] = Gpio::MM100_INJ4;

	// ICM Ignition control module
    engineConfiguration->ignitionPins[0] = Gpio::MM100_IGN7; 
	engineConfiguration->ignitionPins[1] = Gpio::MM100_IGN1;
	engineConfiguration->ignitionPins[2] = Gpio::MM100_IGN2;
	engineConfiguration->ignitionPins[3] = Gpio::MM100_IGN3;
	engineConfiguration->ignitionPins[4] = Gpio::MM100_IGN4;

    engineConfiguration->fuelPumpPin = Gpio::MM100_OUT_PWM2;
    engineConfiguration->idle.solenoidPin = Gpio::MM100_INJ8;
    engineConfiguration->tachOutputPin = Gpio::MM100_IGN5;

    engineConfiguration->triggerInputPins[0] = Gpio::MM100_UART8_TX; // VR2 max9924 is the safer default
    engineConfiguration->camInputs[0] = Gpio::MM100_UART8_RX; // VR1+ Discrete
	engineConfiguration->tps1_1AdcChannel = MM100_IN_TPS_ANALOG;
	engineConfiguration->map.sensor.hwChannel = MM100_IN_MAP1_ANALOG;
	engineConfiguration->clt.adcChannel = MM100_IN_CLT_ANALOG;
	engineConfiguration->iat.adcChannel = MM100_IN_IAT_ANALOG;
}

static Gpio OUTPUTS[] = {
	Gpio::MM100_MEGA_UAEFI_INJ1, // 1 A1 INJ_1
	Gpio::MM100_INJ4, // A2 INJ_4
	Gpio::MM100_INJ2, // A3 INJ_2
	Gpio::MM100_INJ3, // A5 INJ_3
	Gpio::MM100_OUT_PWM2, // 5 Fuel Pump Relay
	Gpio::MM100_INJ8, // A9 IAC
	Gpio::MM100_INJ7, // A13 MIL
	Gpio::MM100_IGN7, // A21 ICM Coil Control
	Gpio::MM100_IGN8, // Radiator Fan Control Module
	Gpio::MM100_OUT_PWM1, // 10 A/C compressor clutch relay
	Gpio::MM100_INJ6, // IAB intake manifold butterflies solenoid
	Gpio::MM100_INJ5, // EGR Solenoid
	Gpio::MM100_IGN6, // VTEC Solenoid Valve
	Gpio::MM100_IGN4, // Coil 4
	Gpio::MM100_IGN3, // Coil 3
	Gpio::MM100_IGN2, // Coil 2
	Gpio::MM100_IGN1, // Coil 1
};

int getBoardMetaOutputsCount() {
    return efi::size(OUTPUTS);
}

int getBoardMetaLowSideOutputsCount() {
    return getBoardMetaOutputsCount() - 1;
}

Gpio* getBoardMetaOutputs() {
    return OUTPUTS;
}

void setup_custom_board_overrides() {
	custom_board_DefaultConfiguration = customBoardDefaultConfiguration;
	// Use our custom override function instead of the base mega-uaefi one
	// This allows us to override SPI1 settings AFTER mega-uaefi tries to enable it
	custom_board_ConfigOverrides = customBoardConfigOverrides;
}
