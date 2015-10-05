#include "Tlc5940.h"
#include <Bounce.h>
#include <MIDI.h>
#define MIDI_CHAN 3


#define HIGHTHRESH 740                                          //TODO ACCEL CONR SEND WITH BUTTONLOW TO SWITCH IT ON AND OFF, probs sec var needed
#define LOWTHRESH 770                                           //MIDI DEBOUNCE
#define FSR_N 9                                                 //BANK CHANGE SWITCH ALL MIDI NOTES OFF, MAYBE AS START RESET FUNCTION, TOO
int fsrPins[] = {15, 17, 40, 20, 21, 23, 22, 16, 14};
//int fsrPins[] = {A14, A0, A1, A2, A3, A6, A7, A8, A9};
int fsrReadings[FSR_N];
bool fsrPress[FSR_N];
int fsrmidiVol[FSR_N];

int scale[] = {37, 38, 39, 40, 41, 42, 43, 44, 36};     //a chromatic scale, use this only for drum pads
                                                        // the last pin is the first note, to hav ethe big pad as the start of the standard ableton drumrack setup

#define BUTTON1 0                                        //BUTTONS AND BOUNCE
#define BUTTON2 1  
#define BUTTON3 2
Bounce ButtonUp = Bounce(BUTTON1, 10);
Bounce ButtonDown = Bounce(BUTTON2, 10);
Bounce ButtonLow = Bounce(BUTTON3, 10);

int tlcbluePins[] = {3,6,9,12,16,19,22,25,28,31};
int tlcgreenPins[] = {1,4,7,10,13,17,20,23,26,29};
int tlcredPins[] = {2,5,8,11,14,18,21,24,27,30};
int tlcallPins[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
#define TLCBLUEPIN_N 10
#define TLCGREENPIN_N 10
#define TLCREDPIN_N 10
#define TLCALLPIN_N 30

#define NUM_BANKS 3                       //number of banks
int bank = 0;

//ADD ACCEL HERE

void setup(void) {
 
  Serial.begin(9600);                     //serial monitor start

  Tlc.init();                             //inits fo all sensors and led chips
  initFsrs();
  initButtons();

  for(int i=0; i<FSR_N; i++){             //reset all press booleans
    fsrPress[i]=false;
  }

  clearLeds();                            //little led boot-up play
  for (int i=0; i<TLCBLUEPIN_N; i++)   {
    Tlc.set(tlcbluePins[i], 3000); 
    Tlc.update();
    delay(50);  
  }
  clearLeds();
  for (int i=0; i<TLCGREENPIN_N; i++)   {
    Tlc.set(tlcgreenPins[i], 3000); 
    Tlc.update();
    delay(50);  
  }
  clearLeds();
  for (int i=0; i<TLCREDPIN_N; i++)   {
    Tlc.set(tlcredPins[i], 3000); 
    Tlc.update();
    delay(50);  
  }
  clearLeds();
  for (int i=0; i<TLCALLPIN_N; i++)   {
    Tlc.set(tlcallPins[i], 3000); 
    Tlc.update();
    delay(50/3);  
  }

  clearLeds();
 
}

void loop(void) {       

  while (usbMIDI.read()) ;                // read and discard any incoming MIDI messages

  readFsrs();
  readButtons();

  if(ButtonUp.fallingEdge()) {              //3 banks and the buttons switch through them BLUE==MODE0, TURKOISE==MODE1, GREEN=MODE2
    bank++;                                 //Leds show bank on button switch when control or midi notes are being sent                                
    if (bank>=3) bank=NUM_BANKS-3;;
    bankLeds();   
    resetPress();                           //resets the press booleans on every bank change  
  }
  else if(ButtonDown.fallingEdge()) {
    bank=bank-1;
    if (bank<=-1) bank=NUM_BANKS-1;
    bankLeds();
    resetPress();                           
  } 
  
  Serial.println(bank); 

  for(int i=0; i<FSR_N; i++){                                        //FSR Code 
    if (fsrReadings[i]<=HIGHTHRESH){                          //PROBS ADD MILIS BOUNCE??
      if(!fsrPress[i]) {
        fsrmidiVol[i] = map (fsrReadings[i], 1024, 0, 0, 127);
        usbMIDI.sendNoteOn(scale[i], fsrmidiVol[i], MIDI_CHAN);
        clearLeds();
      } 
      fsrPress[i]=true;
      Tlc.set(tlcbluePins[i], 3000); 
      if (fsrReadings[8]<=HIGHTHRESH) {
        Tlc.set(tlcbluePins[9],3000);
        }
      Tlc.update();
      delay(1);                                           //tlc chip needs some extra time, without the delay msgs often dont arrive especially in forloops...
      Serial.println(fsrPins[i]);
    }
    else if (fsrReadings[i] > HIGHTHRESH && fsrPress[i]==true) {
      fsrPress[i] = false;
      usbMIDI.sendNoteOff(scale[i], 127, MIDI_CHAN);
      clearLeds();
      Serial.println(String(" Note ")+(scale[i])+(" LedOff "));
    }
  }
}


void initFsrs(){
  pinMode(fsrPins[0], INPUT_PULLUP); 
  pinMode(fsrPins[1], INPUT_PULLUP);
  pinMode(fsrPins[2], INPUT_PULLUP);
  pinMode(fsrPins[3], INPUT_PULLUP);
  pinMode(fsrPins[4], INPUT_PULLUP);
  pinMode(fsrPins[5], INPUT_PULLUP);
  pinMode(fsrPins[6], INPUT_PULLUP);
  pinMode(fsrPins[7], INPUT_PULLUP);
  pinMode(fsrPins[8], INPUT_PULLUP);
}

void initButtons() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
} 

void readFsrs(){                          //reads and prints fsr values
  for(int i=0; i<FSR_N; i++){
    fsrReadings[i] = analogRead(fsrPins[i]);
  }
  for(int i=0;i<FSR_N; i++){
    Serial.print (fsrReadings[i]);
    Serial.print (" ");
  }
  Serial.println (" ");
}

void readButtons(){                       //reads buttons
  ButtonUp.update();
  ButtonDown.update();
  ButtonLow.update();
}

void bankLeds() {                       //the led functions to show the different banks
    if (bank==0) {                      //maybe change color for green and red later...
      clearLeds();
      for (int i=0; i<TLCBLUEPIN_N; i++)   {
        Tlc.set(tlcbluePins[i], 400); 
        Tlc.update(); 
        delay (1);                       //otherwise the chip cant handle the speed 
      }
    }
    else if (bank==1) {
      clearLeds();
      for (int i=0; i<TLCGREENPIN_N; i++)   {
        Tlc.set(tlcgreenPins[i], 300); 
        Tlc.update(); 
        delay (1);                      
      }
    }
    else if (bank==2) {
      clearLeds();
      for (int i=0; i<TLCREDPIN_N; i++)   {
        Tlc.set(tlcredPins[i], 300); 
        Tlc.update(); 
        delay (1);                      
      }
    }
}

void clearLeds() {                      
  Tlc.clear();
  Tlc.update();
}

void resetPress(){
  for(int i=0; i<FSR_N; i++){ 
    fsrPress[i]=false;
  }
}


