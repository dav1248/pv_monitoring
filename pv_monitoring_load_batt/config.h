#include <SPI.h>
#include <SD.h>


/************************* System *******************************/
#define SYSTEM_VOLTAGE 24

#define vbat A7

const int sampling_interval = 4000;
const double max_voltage_count = 2.048; // for a gain of TWO

// MAIN PARAMETERS
const double calibrage_rdiv1_12v = 0;
const double calibrage_rdiv2_12v = 0;
const double calibrage_rdiv1_24v = 0.03;
const double calibrage_rdiv2_24v = 0;
const double calibrage_shunt = 0.001;

const double rdiv1_load_12 = 32.7+calibrage_rdiv1_12v; 
const double rdiv2_load_12 = 1.788;

const double rdiv1_load_24 = 32.5+calibrage_rdiv1_24v; 
const double rdiv2_load_24 = 1.476;

const double v_divider_12 = (rdiv1_load_12+rdiv2_load_12)/rdiv2_load_12;
const double v_divider_24 = (rdiv1_load_24+rdiv2_load_24)/rdiv2_load_24;
const double RshuntLoad = 0.05015;

const double rdiv1_1_bat = 32.80;
const double rdiv1_2_bat = 1.452+0.02;
const double rdiv2_1_bat = 32.9;
const double rdiv2_2_bat = 1.462;
const double vdiv_bat_1 = (rdiv1_1_bat+rdiv1_2_bat)/rdiv1_2_bat;
const double vdiv_bat_2 = (rdiv2_1_bat+rdiv2_2_bat)/rdiv2_2_bat;
const double RshuntBatt = 0.05;
// ---------------


/************************* ADC *********************************/
#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads1115_load(0x48); // load ADC (ADDR connected to VDD)
//Adafruit_ADS1115 ads1115(0x4a); //    Battery ADC (ADDR connected to ...)

/*
 * GAIN_TWOTHIRDS (for an input range of +/- 6.144V)
 * GAIN_ONE (for an input range of +/-4.096V)
 * GAIN_TWO (for an input range of +/-2.048V)
 * GAIN_FOUR (for an input range of +/-1.024V)
 * GAIN_EIGHT (for an input range of +/-0.512V)
 * GAIN_SIXTEEN (for an input range of +/-0.256V)
 */
 

/************************ OLED Display **************************/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
//#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5
double display_awake = 0;


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
