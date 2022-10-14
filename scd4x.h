/*
  This is a library written for the SCD4x family of CO2 sensors
  SparkFun sells these at its website: www.sparkfun.com
  Do you like this library? Help support SparkFun. Buy a board!
  https://www.sparkfun.com/products/18365

  Written by Paul Clark @ SparkFun Electronics, June 2nd, 2021

  The SCD41 measures CO2 from 400ppm to 5000ppm with an accuracy of +/- 40ppm + 5% of reading

  This library handles the initialization of the SCD4x and outputs
  CO2 levels, relative humidty, and temperature.

  https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library


  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for more details.
*/

#ifndef __SparkFun_SCD4x_FLIPPER_LIBARARY_H__
#define __SparkFun_SCD4x_FLIPPER_LIBARARY_H__

// Uncomment the next #define to EXclude any debug logging from the code, by default debug logging code will be included

// #define SCD4x_ENABLE_DEBUGLOG 0 // OFF/disabled/excluded on demand

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>
#include <core/log.h>

//Enable/disable including debug log (to allow saving some space)
#ifndef SCD4x_ENABLE_DEBUGLOG
#if defined(LIBRARIES_NO_LOG) && LIBRARIES_NO_LOG
#define SCD4x_ENABLE_DEBUGLOG 0 // OFF/disabled/excluded on demand
#else
#define SCD4x_ENABLE_DEBUGLOG 1 // ON/enabled/included by default
#endif
#endif

//The default I2C address for the SCD4x is 0x62.
#define SCD4x_ADDRESS (0x62 << 1)
//Available commands

//Basic Commands
#define SCD4x_COMMAND_START_PERIODIC_MEASUREMENT 0x21b1
#define SCD4x_COMMAND_READ_MEASUREMENT 0xec05 // execution time: 1ms
#define SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT 0x3f86 // execution time: 500ms

//On-chip output signal compensation
#define SCD4x_COMMAND_SET_TEMPERATURE_OFFSET 0x241d // execution time: 1ms
#define SCD4x_COMMAND_GET_TEMPERATURE_OFFSET 0x2318 // execution time: 1ms
#define SCD4x_COMMAND_SET_SENSOR_ALTITUDE 0x2427 // execution time: 1ms
#define SCD4x_COMMAND_GET_SENSOR_ALTITUDE 0x2322 // execution time: 1ms
#define SCD4x_COMMAND_SET_AMBIENT_PRESSURE 0xe000 // execution time: 1ms

//Field calibration
#define SCD4x_COMMAND_PERFORM_FORCED_CALIBRATION 0x362f // execution time: 400ms
#define SCD4x_COMMAND_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0x2416 // execution time: 1ms
#define SCD4x_COMMAND_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0x2313 // execution time: 1ms

//Low power
#define SCD4x_COMMAND_START_LOW_POWER_PERIODIC_MEASUREMENT 0x21ac
#define SCD4x_COMMAND_GET_DATA_READY_STATUS 0xe4b8 // execution time: 1ms

//Advanced features
#define SCD4x_COMMAND_PERSIST_SETTINGS 0x3615 // execution time: 800ms
#define SCD4x_COMMAND_GET_SERIAL_NUMBER 0x3682 // execution time: 1ms
#define SCD4x_COMMAND_PERFORM_SELF_TEST 0x3639 // execution time: 10000ms
#define SCD4x_COMMAND_PERFORM_FACTORY_RESET 0x3632 // execution time: 1200ms
#define SCD4x_COMMAND_REINIT 0x3646 // execution time: 20ms

//Low power single shot - SCD41 only
#define SCD4x_COMMAND_MEASURE_SINGLE_SHOT 0x219d // execution time: 5000ms
#define SCD4x_COMMAND_MEASURE_SINGLE_SHOT_RHT_ONLY 0x2196 // execution time: 50ms

typedef union {
    int16_t signed16;
    uint16_t unsigned16;
} scd4x_signedUnsigned16_t; // Avoid any ambiguity casting int16_t to uint16_t

typedef union {
    uint16_t unsigned16;
    uint8_t bytes[2];
} scd4x_unsigned16Bytes_t; // Make it easy to convert 2 x uint8_t to uint16_t

typedef enum { SCD4x_SENSOR_SCD40 = 0, SCD4x_SENSOR_SCD41 } scd4x_sensor_type_e;

bool recvData(const uint8_t* data, uint8_t size);

void SCD4x_init(scd4x_sensor_type_e sensorType);

bool SCD4x_begin(bool measBegin, bool autoCalibrate, bool skipStopPeriodicMeasurements);

void enableDebugging(); //Turn on debug printing.

bool startPeriodicMeasurement(void); // Signal update interval is 5 seconds

// stopPeriodicMeasurement can be called before .begin if required
// Note that the sensor will only respond to other commands after waiting 500 ms after issuing the stop_periodic_measurement command.
bool stopPeriodicMeasurement(uint16_t delayMillis);

bool readMeasurement(
    void); // Check for fresh data; store it. Returns true if fresh data is available

uint16_t
    getCO2(void); // Return the CO2 PPM. Automatically request fresh data is the data is 'stale'
float getHumidity(void); // Return the RH. Automatically request fresh data is the data is 'stale'
float getTemperature(
    void); // Return the temperature. Automatically request fresh data is the data is 'stale'

// Define how warm the sensor is compared to ambient, so RH and T are temperature compensated. Has no effect on the CO2 reading
// Default offset is 4C
bool setTemperatureOffset(
    float offset,
    uint16_t delayMillis); // Returns true if I2C transfer was OK
bool getTemperatureOffset(float* offset); // Returns true if offset is valid

// Define the sensor altitude in metres above sea level, so RH and CO2 are compensated for atmospheric pressure
// Default altitude is 0m
bool setSensorAltitude(uint16_t altitude, uint16_t delayMillis);
bool getSensorAltitude(uint16_t* altitude); // Returns true if altitude is valid

// Define the ambient pressure in Pascals, so RH and CO2 are compensated for atmospheric pressure
// setAmbientPressure overrides setSensorAltitude
bool setAmbientPressure(float pressure, uint16_t delayMillis);

bool performForcedRecalibration(
    uint16_t concentration,
    float* correction); // Returns true if FRC is successful

bool setAutomaticSelfCalibrationEnabled(bool enabled, uint16_t delayMillis);
bool getAutomaticSelfCalibrationEnabledExt(uint16_t* enabled);
bool getAutomaticSelfCalibrationEnabled();

bool startLowPowerPeriodicMeasurement(
    void); // Start low power measurements - receive data every 30 seconds
bool getDataReadyStatus(void); // Returns true if fresh data is available

bool persistSettings(uint16_t delayMillis); // Copy sensor settings from RAM to EEPROM
bool getSerialNumber(char* serialNumber); // Returns true if serial number is read correctly
bool performSelfTest(void); // Takes 10 seconds to complete. Returns true if the test is successful
bool performFactoryReset(uint16_t delayMillis); // Reset all settings to the factory values
bool reInit(uint16_t delayMillis); // Re-initialize the sensor, load settings from EEPROM

bool measureSingleShot(
    void); // SCD41 only. Request a single measurement. Data will be ready in 5 seconds
bool measureSingleShotRHTOnly(
    void); // SCD41 only. Request RH and T data only. Data will be ready in 50ms

bool sendCommandArgs(uint16_t command, uint16_t arguments);
bool sendCommand(uint16_t command);

bool readRegister(uint16_t registerAddress, uint16_t* response, uint16_t delayMillis);

uint8_t computeCRC8(uint8_t data[], uint8_t len);

char convertHexToASCII(uint8_t digit);
#endif
