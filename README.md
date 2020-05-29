# PV Monitoring System

Offline PV Monitoring System working on Adafruit M0 Feather. The system is composed of two devices, each containing:
* Adafruit M0 Feather
* Adafruit Adalogger RTC
* Adafruit OLED display
* Adafruit ADS1115 16-bit ADC
* 1x lipo battery (to power the system)
* 50mOhm high precision shunt resistor
* various voltage measuring resistors

![IMS](/images/ims.jpg)


## How to build

![IMS](/images/ims.jpg)


![Schema](/images/schema_irr.png)

list of components
<!---
IMAGE OF CIRCUIT
-->

## Code walkthrough

Before getting anything done, you can check the [Adafruit M0 page](https://learn.adafruit.com/adafruit-feather-m0-basic-proto) and follow the instructions on how to setup the M0.

To run, you just have to click the Upload button on the Arduino IDE.

#### config.h

The *config.h* contains a few parameters that will influence the monitoring:

**sampling_interval**: defines the rate of sampling and logging to SD card. A 4s sampling rate offers good balance between accuracy and reasonable file length, especially when using Excel.

**max_voltage_count**: the voltage value when the ADC of the ADS1115 measures the maximum value (in counts). For example, with a gain of TWO, the maximum voltage count of 32768 corresponds to 2.048 Volts. It is possible to modify the gain.

**rdiv1_pv**: first resistor in voltage divider.

**rdiv2_pv**: second resistor in voltage divider.

**RshuntPV**: the value of the shunt resistor.


#### setup()

SD card reader: the code looks at existing log files and creates a new one with an incremented number, before opening it to insert the headers.

#### loop()

For each loop, the program will check if the current time is inside the sleep window, if yes, it will go into low-power sleep mode. If not, it will enter the main logic:

* At each loop, check if *lastUpdate* was more than *sampling_interval* milliseconds ago. 
* If yes, read the ADC pins of the M0 and compute the potential difference at the resistance.
* From the previous values, compute the short circuit current and irradiance
* save the computed values into the *.tsv* file
* handles the OLED screen and buttons

By using *millis()*, we get a seemingly uninterrupted program that updates in real time the values into the SD card and on the OLED display while listening for buttons inputs.



## Code Libraries

The following libraries are needed (just type the name in the Arduino IDE under tools->Manage Libraries)

* Adafruit_ADS1015 (external ADC)
* RTClib (RTC module)
* Adafruit SleepyDog
* Adafruit SSD1306 (OLED Display)


## Useful links

* [ADS1115 Adafruit page](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/)