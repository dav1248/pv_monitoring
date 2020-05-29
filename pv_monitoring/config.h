#include <SPI.h>
#include <SD.h>

#define vbat A7

/************************* System Parameters  *******************************/
/* the following parameters can be modified to correspond to your system:   */

const int sampling_interval = 4000;     // sampling interval
const double max_voltage_count = 2.048; // corresponding to GAIN_TWO

// Voltage divider resistors: change the value of the two resistors to match the ones you are using
const double rdiv1_pv = 33.4; // first resistor
const double rdiv2_pv = 1.470; // second resistor
const double voltage_divider = (rdiv1_pv+rdiv2_pv)/rdiv2_pv;

// Current-sensing resistor: needs to be of high accuracy and power-rating
const double RshuntPV = 0.05;


/************************* ADC *********************************/
#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads1115_pv(0x48); //   PV ADC (ADDR not connected (default GND))
// you can connect other external ADC to the M0, with the following addresses:
//Adafruit_ADS1115 ads1115_pv2(0x49); // second ADC (ADDR connected to VDD)
//Adafruit_ADS1115 ads1115_pv3(0x4a); //    third ADC (ADDR connected to SCL)

/* Table of correspondance of ADS1115 Gain parameters:
 * GAIN_TWOTHIRDS (for an input range of +/- 6.144V)
 * GAIN_ONE (for an input range of +/-4.096V)
 * GAIN_TWO (for an input range of +/-2.048V)
 * GAIN_FOUR (for an input range of +/-1.024V)
 * GAIN_EIGHT (for an input range of +/-0.512V)
 * GAIN_SIXTEEN (for an input range of +/-0.256V)
 */


/************************* OLED Display ***************************/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
//#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5

/************************* SD Card *********************************/
// Set the pins used
#define cardSelect 10 
File logfile;

/************************* RTC *************************************/
// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int offset_year=2000; // offset year for rtc formatting
