/*   DED v1.15 Implemented interrupt method to switch mode.
 *   DED v1.1 Added antenna symbols that replace the radio channels during radio operation in F-18 mode.
 *   Changed LIST and MISC detection method as just checking the first row wasn't robust enough.
 *  DED v1.0 Created by James Storey in May 2022, github.com/jg-storey/ded
 *  Arduino code to display F-16 Data Entry Display and F-18 Upfront Controls information from DCS-BIOS
 *  on a ER-OLEDM028-1 2.8" 256x64 OLED display from BuyDisplay.com
 *  https://www.buydisplay.com/yellow-2-8-inch-arduino-raspberry-pi-oled-display-module-256x64-spi
 *  Tested with DCS-BIOS v0.10.0
 *  https://github.com/dcs-bios/dcs-bios/releases
 *  
 *  A toggle switch (i.e. not a pushbutton) wired between pin D2 and GND allows the user to select between the F-16 and F-18 modes.
 *  If a switch is not connected, the mode defaults to the F-16.
 *  The selected mode activates after disconnecting and connecting to the Arduino in the DCS-BIOS Hub, or by resetting the Arduino.
 *  There is probably a better way to change modes using an interrupt.
 *  
 *  The F-18 display also includes a fuel reading and a master arm indicator. 
 *  There is still some space to add other items in both modes.
 *  
 *  Known issues. The DEST page in the LIST menu comes up blank. This appears to be an issue with DCS-BIOS.
 *  When entering data between the highlighted asterisks the field does not highlight. I haven't figured out an efficient way to do this yet.
 */

/*
  Tell DCS-BIOS to use a serial connection and use interrupt-driven
  communication. The main program will be interrupted to prioritize
  processing incoming data.
  
  This should work on any Arduino that has an ATMega328 controller
  (Uno, Pro Mini, many others).
 */
#define DCSBIOS_IRQ_SERIAL


#include "DcsBios.h"

/***************************************************
//Web: http://www.buydisplay.com
EastRising Technology Co.,LTD
Examples for ER-OLEDM28-1
Display is Hardward SPI 4-Wire SPI Interface 
Tested and worked with:
Works with Arduino 1.6.0 IDE  
Test OK : Arduino DUE,Arduino mega2560,Arduino UNO Board 
****************************************************/

/*
 Note:The module needs to be jumpered to an SPI interface. R19,R23 Short and R18,R20 Open  
 Unused signal pin Recommended to connect to GND
  == Hardware connection ==
    OLED   =>    Arduino
  *1. GND    ->    GND
  *2. VCC    ->    3.3
  *4. SCL    ->    SCK (D13)
  *5. SDI    ->    MOSI (D11)
  *14. DC     ->    D9
  *15. RES    ->    D8  
  *16. CS     ->    D10
*/



#include "er_oled.h"


volatile boolean f16 = true; // flag to display F-16 DED
volatile boolean f18 = false; // flag to display F-18 UFC
//boolean isList = false; //flag to indicate if we are on the LIST or MISC DED pages

const int buttonPin = 2; //Toggle switch to change modes
//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

//F-16 DCS BIOS Functions

void onDedLine1Change(char* newValue) {
    /* your code here */
    if(f16){
      
      er_oled_dedstring(0,0,newValue,0); 
    }
}
DcsBios::StringBuffer<29> dedLine1Buffer(0x4500, onDedLine1Change);

void onDedLine2Change(char* newValue) {
    /* your code here */
    if(f16){
      er_oled_dedstring(0,13,newValue,0); 
      
      //Highlight numbers if we are on the LIST or MISC pages
      if(newValue[0]=='1' && (newValue[1]=='D' || newValue[1]=='C')){
        //Highlight the menu numbers
        er_oled_string(0,13,"1",1);
        er_oled_string(6*8,13,"2",1);
        er_oled_string(12*8,13,"3",1);
        er_oled_string(18*8,13,"R",1);
      }
    }  
}
DcsBios::StringBuffer<29> dedLine2Buffer(0x451e, onDedLine2Change);


void onDedLine3Change(char* newValue) {
    /* your code here */
    if(f16){
      er_oled_dedstring(0,26,newValue,0); 
      
      if(newValue[0]=='4' && (newValue[1]=='N' || newValue[1]=='I')){
        er_oled_string(0,26,"4",1);
        er_oled_string(6*8,26,"5",1);
        er_oled_string(12*8,26,"6",1);
        er_oled_string(18*8,26,"E",1);
      }
    }  
}
DcsBios::StringBuffer<29> dedLine3Buffer(0x453c, onDedLine3Change);

void onDedLine4Change(char* newValue) {
    /* your code here */
    if(f16){
      er_oled_dedstring(0,39,newValue,0);  
      
      if(newValue[0]=='7' && (newValue[1]=='C' || newValue[1]=='D')){
        er_oled_string(0,39,"7",1);
        er_oled_string(6*8,39,"8",1);
        er_oled_string(12*8,39,"9",1);
        er_oled_string(18*8,39,"0",1);
      }
    }
}
DcsBios::StringBuffer<29> dedLine4Buffer(0x455a, onDedLine4Change);

void onDedLine5Change(char* newValue) {
    /* your code here */
    if(f16){
      er_oled_dedstring(0,52,newValue,0); 
    }  
}
DcsBios::StringBuffer<29> dedLine5Buffer(0x4578, onDedLine5Change);




// F-18 DCS BIOS Functions
void onUfcComm1DisplayChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(0*8,52,newValue,0);
    }
}
DcsBios::StringBuffer<2> ufcComm1DisplayBuffer(0x7424, onUfcComm1DisplayChange);


void onUfcComm2DisplayChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(5*8,52,newValue,0);
    }
}
DcsBios::StringBuffer<2> ufcComm2DisplayBuffer(0x7426, onUfcComm2DisplayChange);


void onUfcOptionCueing1Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(12*8,0,newValue,0);
    }  
}
DcsBios::StringBuffer<1> ufcOptionCueing1Buffer(0x7428, onUfcOptionCueing1Change);


void onUfcOptionCueing2Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(12*8,13,newValue,0);
    }
}
DcsBios::StringBuffer<1> ufcOptionCueing2Buffer(0x742a, onUfcOptionCueing2Change);


void onUfcOptionCueing3Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(12*8,26,newValue,0);
    }  
}
DcsBios::StringBuffer<1> ufcOptionCueing3Buffer(0x742c, onUfcOptionCueing3Change);


void onUfcOptionCueing4Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(12*8,39,newValue,0);
    }
}
DcsBios::StringBuffer<1> ufcOptionCueing4Buffer(0x742e, onUfcOptionCueing4Change);

void onUfcOptionCueing5Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(12*8,52,newValue,0);
    }
}
DcsBios::StringBuffer<1> ufcOptionCueing5Buffer(0x7430, onUfcOptionCueing5Change);


void onUfcOptionDisplay1Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(13*8,0,newValue,0);
    }
}
DcsBios::StringBuffer<4> ufcOptionDisplay1Buffer(0x7432, onUfcOptionDisplay1Change);


void onUfcOptionDisplay2Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(13*8,13,newValue,0);
    }
}
DcsBios::StringBuffer<4> ufcOptionDisplay2Buffer(0x7436, onUfcOptionDisplay2Change);

void onUfcOptionDisplay3Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(13*8,26,newValue,0);
    }
}
DcsBios::StringBuffer<4> ufcOptionDisplay3Buffer(0x743a, onUfcOptionDisplay3Change);

void onUfcOptionDisplay4Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(13*8,39,newValue,0);
    }
}
DcsBios::StringBuffer<4> ufcOptionDisplay4Buffer(0x743e, onUfcOptionDisplay4Change);

void onUfcOptionDisplay5Change(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(13*8,52,newValue,0);
    }
}
DcsBios::StringBuffer<4> ufcOptionDisplay5Buffer(0x7442, onUfcOptionDisplay5Change);


void onUfcScratchpadNumberDisplayChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(3*8,0,newValue,0);
    }
}
DcsBios::StringBuffer<8> ufcScratchpadNumberDisplayBuffer(0x7446, onUfcScratchpadNumberDisplayChange);



void onUfcScratchpadString1DisplayChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(0*8,0,newValue,0);
    }
}
DcsBios::StringBuffer<2> ufcScratchpadString1DisplayBuffer(0x744e, onUfcScratchpadString1DisplayChange);

void onUfcScratchpadString2DisplayChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(2*8,0,newValue,0);
    }
}
DcsBios::StringBuffer<2> ufcScratchpadString2DisplayBuffer(0x7450, onUfcScratchpadString2DisplayChange);


void onIfeiFuelUpChange(char* newValue) {
    /* your code here */
    if(f18){
      er_oled_string(20*8,26,newValue,0);
    }
}
DcsBios::StringBuffer<6> ifeiFuelUpBuffer(0x748a, onIfeiFuelUpChange);

void onMasterModeAaLtChange(unsigned int newValue) {
    /* your code here */
}
DcsBios::IntegerBuffer masterModeAaLtBuffer(0x740c, 0x0200, 9, onMasterModeAaLtChange);

void onMasterArmSwChange(unsigned int newValue) {
    /* your code here */
    if(f18){
      if(newValue){
      er_oled_string(20*8,0," ARM ",1);
    }else{
      er_oled_string(20*8,0," SAFE ",0);
    }
  }
}
DcsBios::IntegerBuffer masterArmSwBuffer(0x740c, 0x2000, 13, onMasterArmSwChange);

void splashScreen(){
  er_oled_clear();
  er_oled_string(12*8,13,"DED V1.15",1);
  er_oled_string(4*8,26,"GITHUB.COM/JG-STOREY/DED",0);
  if(f16){
    er_oled_string(11*8,52,"F-16 MODE",0);
  }
  else if(f18){
    er_oled_string(11*8,52,"F/A-18 MODE",0);
  }
  
  
}

void button_ISR(){
  button_time = millis();
  //check to see if increment() was called in the last 250 milliseconds
  if (button_time - last_button_time > 250){
    f16 = digitalRead(buttonPin);
    f18 = !f16;
    splashScreen();
    er_oled_clear();
    last_button_time = button_time;
  }
}

void setup() {

  //set D2 as a switch to select F-16 or F-18 displays. If unconnected it defaults to F-16.
  pinMode(buttonPin, INPUT_PULLUP);
  f16 = digitalRead(buttonPin);
  f18 = !f16;
  /* initialize the display */
  er_oled_begin();
  splashScreen();
  delay(2000);
  er_oled_clear();
  attachInterrupt(digitalPinToInterrupt(buttonPin), button_ISR, CHANGE);  
  
  DcsBios::setup();
  
}


void loop() {

DcsBios::loop();
 
}
