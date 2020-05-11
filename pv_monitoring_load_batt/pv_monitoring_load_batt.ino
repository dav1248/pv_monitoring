/* 
 *  Code for the Adafruit M0 Feather microcontroller with logging, 
 *  OLED and external 16-bit ADC module ADS1115.
 *  It allows the reading and logging of external ADC values, in the case of 
 *  voltage and current measurement of the solar controller output.
 */

#include "config.h"

unsigned long lastUpdate=0;

/************************* System Parameters **************************/
double lipo_voltage=0;

int32_t vcount_load=0;
int32_t icount_load=0;
double voltage_load=0;
double voltage_divider=0;
double current_load=0;

double vbat1=0;
double vbat2=0;
double voltage_battery=0;
double current_battery=0;

double power_load=0;
double power_battery=0;

double energy_load=0;
double energy_battery=0;

int Ainput0 = 0; // used for battery voltage and current input
int Ainput1 = 0;
int Ainput2 = 0;


/**********************************************************************/
/************************* Setup **************************************/
/**********************************************************************/
void setup() {
  
  Serial.begin(9600);
  delay(1000);
  //while (!Serial) ;
  Serial.println("\r\nAnalog logger test");
  
  pinMode(A0, INPUT);
  pinMode(13, OUTPUT);

  analogReadResolution(12); // resolution setup for internal ADC

  // ADC INIT //
  ads1115_load.begin();
  
  ads1115_load.setGain(GAIN_TWO); // for an input range of +/-2.048V. See config.h for GAIN values

  // voltage divider setup
  if(SYSTEM_VOLTAGE==12){
    voltage_divider = v_divider_12;
  } else if(SYSTEM_VOLTAGE==24){
    voltage_divider = v_divider_24;
  }
  
  // OLED INIT //
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println("OLED begun");
   
  // Clear the buffer.
  display.clearDisplay();   // Clear the buffer.
  display.display();

  //pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
  // text display init
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("hiLyte Load ");
  display.println("monitoring");


  // SD CARD INIT //
  if (!SD.begin(cardSelect)) {   // see if the card is present and can be initialized:
    Serial.println("No card inserted!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("No card inserted!");
    display.display();
    delay(3000);
    error(2);
  }
  char filename[15];
  strcpy(filename, "/LD000.tsv");
  for (uint8_t i = 0; i < 1000; i++) {
    filename[3] = '0' + i/100;
    filename[4] = '0' + i/10;
    filename[5] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to "); 
  Serial.println(filename);

  init_logfile();
  
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.println("Ready!");


  // display the name of the newly created file
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("file :");
  display.println(filename);
  display.display(); 
  delay(2000);
  
  // RTC INIT //
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //can use this line to recalibrate date&time
    // rtc.adjust(DateTime(2020, 1, 1, 23, 59, 50));    //to do so, remove the small battery 
  }
  digitalWrite(13, HIGH);
  
}


/**********************************************************************/
/************************* Loop ***************************************/
/**********************************************************************/
void loop() {

  if(millis() > (lastUpdate + sampling_interval)){

    lipo_voltage = (3.3/4096.0)*(2*analogRead(vbat));

    vcount_load=0;
    icount_load=0;
    for(int i=0;i<10;i++){
      vcount_load += ads1115_load.readADC_Differential_0_1(); 
      icount_load -= ads1115_load.readADC_Differential_2_3(); 
    }
    
    vcount_load = vcount_load/10.0;
    icount_load = icount_load/10.0;


    voltage_load = max_voltage_count*voltage_divider*vcount_load/32768; // current resolution: 1bit = 1mV
    current_load = max_voltage_count*icount_load/(RshuntLoad*32768);

    
    Ainput0 = analogRead(A0); //+(1) batt
    Ainput1 = analogRead(A1); //+(2) batt
    Ainput2 = analogRead(A2); //- batt

    Serial.println(Ainput0);
    Serial.println(Ainput1);
    Serial.println(Ainput2);
    
    vbat1 = 3.3 * vdiv_bat_1* Ainput0/ 4096;
    vbat2 = 3.3* vdiv_bat_2* Ainput1/ 4096;
    
    Serial.println(vbat1);
    Serial.println(vbat2);
    
    voltage_battery= 3.3* (vdiv_bat_1) * (Ainput0-Ainput2) /4096;
    current_battery= (vbat1-vbat2) /(RshuntBatt);
  
    power_load = instant_power(voltage_load, current_load);
    energy_load = energy_load + energy(power_load, sampling_interval);

    power_battery = instant_power(voltage_battery, current_battery);
    energy_battery = energy_battery + energy(power_battery, sampling_interval);
    
    
    input_logfile();

    lastUpdate = millis();
  }
  UI_management();
}


/**********************************************************************/
/************************* Helper Functions ***************************/
/**********************************************************************/

// returns the instant power
double instant_power(double voltage, double current){
  return current*voltage;
}


// returns the energy of the current interval in Wh
double energy(double power, int sampling_interval){
  return power*(sampling_interval/1000)/3600;
}




// inits the logfile with correct columns
void init_logfile(){
  logfile.print("time \t");
  logfile.print("p_load \t");
  logfile.print("p_battery \t");
  logfile.print("v_load \t");
  logfile.print("i_load \t");
  logfile.print("i_battery \t");
  logfile.print("e_load \t");
  logfile.print("e_battery \t");
  logfile.print("v_battery \t");
  logfile.print("vbat \t");
  logfile.print("date \t");

  logfile.println();
}

// saves the values into file in SD card with a timestamp
void input_logfile(){

    DateTime now = rtc.now(); // rtc snapshot
    
    // rtc
    logfile.print(now.hour(),DEC);
    logfile.print(":");
    logfile.print(now.minute(),DEC);
    logfile.print(":");
    logfile.print(now.second(),DEC);
    logfile.print("\t");
  
    // values
    logfile.print(power_load);
    logfile.print("\t");
    logfile.print(power_battery);
    logfile.print("\t");
    logfile.print(voltage_load);
    logfile.print("\t");
    logfile.print(current_load);
    logfile.print("\t");
    logfile.print(current_battery);
    logfile.print("\t");
    logfile.print(energy_load);
    logfile.print("\t");
    logfile.print(energy_battery);
    logfile.print("\t");
    logfile.print(voltage_battery);
    logfile.print("\t");
    logfile.print(lipo_voltage);
    logfile.print("\t");

    logfile.print(now.day());
    logfile.print(".");
    logfile.print(now.month());
    logfile.print(".");
    logfile.print((now.year()-offset_year),DEC);
    logfile.print("\t");
    logfile.println("");
    
    logfile.flush(); // flushing often increases power consumption (30mA VS 10mA)
}

// displays information on OLED display and handles button inputs
void UI_management(){

  if(!digitalRead(BUTTON_B)) 
  {
    display.clearDisplay();
    display.setCursor(0,0);

    display.print("bat_P:  ");
    display.print(power_battery); display.println(" [W]");
    display.print("bat_V:  ");
    display.print(voltage_battery); display.println(" [V]");
    display.print("bat_I:  ");
    display.print(current_battery); display.println(" [A]");
    
    display.print("vbat: "); display.print(lipo_voltage); display.println("[V]");
    display.println();
    
    DateTime now = rtc.now(); // rtc snapshot
    display.print(now.day());display.print(".");display.print(now.month());display.print(".");display.println((now.year()-offset_year),DEC);
    display.print(now.hour(),DEC);display.print(":");display.print(now.minute(),DEC);display.print(":");display.println(now.second(),DEC);
  }
  else if(!digitalRead(BUTTON_C))
  {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("load_e:  ");
    display.print(energy_load); display.println(" [Wh]");
    display.print("battery_e:  ");
    display.print(energy_battery); display.println(" [Wh]");
    
  } else {
    // OLED INFO DISPLAY //
    display.clearDisplay();
    display.setCursor(0,0);
    
    display.println("LOAD  ");
    display.print("P: ");display.print(power_load); display.println(" [W]");
    display.print("V: ");display.print(voltage_load); display.println(" [V]  ");
    display.print("I: ");display.print(current_load); display.println(" [A]");
  }
  
  delay(10);
  yield();
  display.display();
}

// error function
void error(uint8_t errno) {

    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
}
