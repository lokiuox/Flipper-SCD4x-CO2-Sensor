# Flipper SCD4x CO2 sensor

A PoC Sensirion SCD4x CO2 sensor plugin for the Flipper Zero.    
This app is a WIP, the code is a bit of a mess and needs some more work, I just made the GUI functional enough to show the sensor readings (the included library should be fully functional though).    

![Screenshot](/images/flipper_screenshot.png)
## Contributions
Contributions are welcome!    
There are a few things already in the roadmap:
* A menu to configure some chip-specific options such as chip type (SCD40/41), low power mode, auto calibration, altitude, pressure, etc.
* A refactoring of the GUI app to generally improve the code style and to keep the status in a struct instead of global variables
## Credits
* The scd4x library is Sparkfun's [SparkFun SCD4x CO2 Sensor Library](https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library) which I modified a bit to adapt it to the Flipper.
* The basic app structure was adapted from https://github.com/Mywk/FlipperTemperatureSensor
