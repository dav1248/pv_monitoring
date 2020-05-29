/* 
 *  Code for the Adafruit M0 Feather microcontroller with logging, 
 *  OLED and external 16-bit ADC module ADS1115.  
 *  It allows the reading and logging of external ADC values, in the case of 
 *  voltage and current measurement of the PV module output.
 */

#include "config.h"

unsigned long lastUpdate=0;

/************************* System Parameters **************************/
double lipo_voltage=0;

int32_t vcount_pv=0;
int32_t icount_pv=0;
double voltage_pv=0;
double current_pv=0;

double power_pv=0;
double energy_pv=0;


/**********************************************************************/
/************************* Setup **************************************/
/**********************************************************************/
void setup() {
  
  Serial.begin(9600);
  delay(1000);
  //while (!Serial) ;
  pinMode(13, OUTPUT);

  analogReadResolution(12); // default analog read resolution is 10bits, but M0 supports up to 12

  // ADC INIT //
  ads1115_pv.begin();
  
  ads1115_pv.setGain(GAIN_TWO); // for an input range of +/-2.048V. See config.h for GAIN values

  
  // OLED INIT //
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println("OLED begun");
  
  display.clearDisplay();   // Clear the buffer.
  display.display();

  //pinMode(BUTTON_A, INPUT_PULLUP); pin of Button A already used
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
  // text display init
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("hiLyte PV");
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
  strcpy(filename, "/PV000.tsv");
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
    //rtc.adjust(DateTime(2020, 1, 1, 23, 59, 50));    //to do so, remove the small battery 
  }
  digitalWrite(13, HIGH);
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //can use this line to recalibrate date&time

}


/**********************************************************************/
/************************* Loop ***************************************/
/**********************************************************************/
void loop() {

  if(millis() > (lastUpdate + sampling_interval)){

    lipo_voltage = (3.3/4096.0)*(2*analogRead(vbat));

    // multiple sampling to take the average value due to MPP algorithm noise
    vcount_pv=0;
    icount_pv=0;
    for(int i=0;i<15;i++){

      vcount_pv += ads1115_pv.readADC_Differential_0_1(); 
      icount_pv -= ads1115_pv.readADC_Differential_2_3(); 
    }

    vcount_pv = vcount_pv/15.0;
    icount_pv = icount_pv/15.0;

    
    voltage_pv = max_voltage_count*voltage_divider*vcount_pv/32768; // current resolution: 1bit = 1mV
    current_pv = max_voltage_count*icount_pv/(RshuntPV*32768);
 
    
    power_pv = instant_power(voltage_pv, current_pv);
    energy_pv = energy_pv + energy(power_pv,sampling_interval);
   
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

// displays information on OLED display and handles button inputs
void UI_management(){
  
  if(!digitalRead(BUTTON_B)) 
  {
    display.clearDisplay();
    display.setCursor(0,0);
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
    display.print("PV_e:    ");
    display.print(energy_pv); display.println(" [Wh]");
    
  } else {
  
    // OLED INFO DISPLAY //
    display.clearDisplay();
    display.setCursor(0,0);
    
    display.println("PV  ");
    display.print("P: ");display.print(power_pv); display.println(" [W]");
    display.print("V: ");display.print(voltage_pv); display.println(" [V]  ");
    display.print("I: ");display.print(current_pv); display.println(" [A]");
  }
  delay(10);
  yield();
  display.display();
}

// inits the logfile with correct columns
void init_logfile(){
  logfile.print("date \t");
  logfile.print("time \t");
  logfile.print("p_pv \t");
  logfile.print("v_pv \t");
  logfile.print("i_pv \t");
  logfile.print("e_pv \t");
  logfile.print("vcount_pv \t");
  logfile.print("icount_pv \t");
  logfile.print("vbat \t");
  logfile.println();
}

// saves the values into file in SD card with a timestamp
void input_logfile(){

    DateTime now = rtc.now(); // rtc snapshot
    
    // rtc
    logfile.print(now.day());
    logfile.print(".");
    logfile.print(now.month());
    logfile.print(".");
    logfile.print((now.year()-offset_year),DEC);
    logfile.print("\t");
    logfile.print(now.hour(),DEC);
    logfile.print(":");
    logfile.print(now.minute(),DEC);
    logfile.print(":");
    logfile.print(now.second(),DEC);
    logfile.print("\t");
  
    // values
    logfile.print(power_pv);
    logfile.print("\t");
    logfile.print(voltage_pv);
    logfile.print("\t");
    logfile.print(current_pv);
    logfile.print("\t");
    logfile.print(energy_pv);
    logfile.print("\t");
    logfile.print(vcount_pv);
    logfile.print("\t");
    logfile.print(icount_pv);
    logfile.print("\t");
  
    logfile.print(lipo_voltage);
  
    logfile.println("");
    logfile.flush(); // flushing can increase power consumption (30mA VS 10mA)
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
