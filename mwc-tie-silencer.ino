// Magic Wheelchair - Tie Silencer 
//
// reference: 
// IMPORTANT NOTE: 8.3 FILENAMES FOR WAV AUDIO FILES!
// IMPORTANT NOTE: WAV 44100 STEREO 16BIT

#define DEBUG_INPUT  1  //input functions will Serial.print if 1
#define DEBUG_AUDIO  1  //audio functions will Serial.print if 1
#define DEBUG_ACTION 1  //action functions will Serial.print if 1

/*
 * Hardware Buttons
 */
 
#include <Bounce.h>

#define NUM_BUTTONS 4
int buttonPins[NUM_BUTTONS] = { 0, 1, 2, 3 };
Bounce buttons[NUM_BUTTONS] = { {buttonPins[0], 250}, {buttonPins[1], 250}, {buttonPins[2], 1000},{buttonPins[3], 1000} };
unsigned long buttonDuration[NUM_BUTTONS];    //holds the last millis() for a falling edge
#define BUTTON_HOLD_DURATION 2000

int keyHeld = 0;
unsigned long keyHeldDuration = 0;
#define KEY_HOLD_DURATION 2000


/*
 * Audio System Includes & Globals
 * Reference the Audio Design Tool - https://www.pjrc.com/teensy/gui/index.html
 * 
 */


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav2;     //xy=163,192
AudioPlaySdWav           playSdWav1;     //xy=165,133
AudioPlaySdWav           playSdWav3;     //xy=165,251
AudioMixer4              mixer2;         //xy=348.16668701171875,238.3333282470703
AudioMixer4              mixer1;         //xy=350.0000305175781,146.1666717529297
AudioMixer4              mixer3;         //xy=569,306
AudioOutputI2S           i2s1;           //xy=578.1666564941406,152.6666717529297
AudioAnalyzePeak         peak1;          //xy=709,306
AudioConnection          patchCord1(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord2(playSdWav2, 1, mixer2, 1);
AudioConnection          patchCord3(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord4(playSdWav1, 1, mixer2, 0);
AudioConnection          patchCord5(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord6(playSdWav3, 1, mixer2, 2);
AudioConnection          patchCord7(mixer2, 0, i2s1, 1);
AudioConnection          patchCord8(mixer2, 0, mixer3, 1);
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer1, 0, mixer3, 0);
AudioConnection          patchCord11(mixer3, peak1);
AudioControlSGTL5000     sgtl5000_1;     //xy=106,55
// GUItool: end automatically generated code


#define NUM_CHANNELS       3     //SD Card playback is best to not exceed 3 simultaneous WAVs
                                 //There is an SD card test in the Audio examples

#define CHANNEL_MUSIC      0     //Only one item can play per channel
#define CHANNEL_ENGINE     1     //Use these to map sound types to a channel
#define CHANNEL_SPEECH     2     
#define CHANNEL_WEAPON     2

#define LEVEL_CHANNEL0    .3    //change these for relative channel levels
#define LEVEL_CHANNEL1    .3
#define LEVEL_CHANNEL2    .8  

#define MAIN_VOLUME       .8    //change this to reduce clipping in the main amp

//I use this syntax so that I can leave the declarations above which come from the Audio Design tool
AudioPlaySdWav *channels[NUM_CHANNELS] = { &playSdWav1, &playSdWav2, &playSdWav3 };
String playQueue[NUM_CHANNELS];

#define NUM_BGM_WAVS        6  //plays random file //set to 3 for KYLO specific
#define NUM_LASER_WAVS      7  //plays random file
#define NUM_TORPEDO_WAVS    4  //plays random file
#define NUM_ENGINE_WAVS     2  //plays 0 all the time, then 1 when button pressed
#define NUM_KYLO_WAVS       9  //plays random


//LED ALL THE THINGS!
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>


#define LASER_NUM_LEDS 20 
#define LASER_DATA_PIN 33
CRGB laserLEDS[LASER_NUM_LEDS];

#define ENGINE_NUM_LEDS 52 
#define ENGINE_DATA_PIN 26
CRGB engineLEDS[ENGINE_NUM_LEDS]; 

#define COCKPIT_NUM_LEDS 10
#define COCKPIT_DATA_PIN 32
CRGB cockpitLEDS[COCKPIT_NUM_LEDS]; 

#define DEFAULT_BRIGHTNESS 32

//Show & Mode Globals

#include <Metro.h> //Include Metro library 

Metro bgmMetro = Metro(100);
Metro playQueueMetro = Metro(50);

bool bgmStatus = 1;           //0 = BGM off; 1 = BGM on
bool engineStatus = 1;        //0 = Engine off; 1 = Engine on

//for USB host functions
#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
KeyboardController keyboard1(myusb);


// this is for wav file playback to reduce locking
unsigned long lastPlayStart = 0;

// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

#define SOURCE_KEY                    0
#define SOURCE_BUTTON                 1


#define ACTION_DO_NOT_USE             0 //just putting this here as a reminder to not use it :)

#define ACTION_TORPEDO               1
#define ACTION_LASER                 2
#define ACTION_KYLO                  3
#define ACTION_ENGINE                4
#define ACTION_BGM_TOGGLE            5 //BGM = BACKGROUND MUSIC


#define ACTION_PLAY_WAV               10
#define ACTION_PLAY_WAV_RND           11




/*
 * The ActionMap allows us to have multiple types of input events
 * that get mapped to an action. We can have a button press, or an HID keyboard
 * key, etc. that all go to the same action
 * 
 */

#define ACTION_MAP_SIZE 9

int ActionMap[][3] = {
  //src, key, action
  {SOURCE_KEY, 214, ACTION_TORPEDO},            //remote right
  {SOURCE_KEY, 211, ACTION_LASER},              //remote left
  {SOURCE_KEY,  27, ACTION_KYLO},               //remote up
  {SOURCE_KEY,  98, ACTION_ENGINE},             //remote down
  {SOURCE_KEY, 198, ACTION_BGM_TOGGLE},         //remote play
  {SOURCE_BUTTON, 0, ACTION_TORPEDO},            //blue button
  {SOURCE_BUTTON, 1, ACTION_LASER},              //green button
  {SOURCE_BUTTON, 2, ACTION_KYLO},               //white button
  {SOURCE_BUTTON, 3, ACTION_ENGINE},             //red button
   
}; //if you change this, don't forget to update the ACTION_MAP_SIZE



void setup() {
  Serial.begin(115200);
  AudioMemory(128);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

 
  //seed random function
  randomSeed(analogRead(0));

  for (int btn = 0; btn< NUM_BUTTONS; btn++) {
    pinMode(buttonPins[btn], INPUT_PULLUP);
    buttonDuration[btn] = 0; 
    buttons[btn].update();
  }

  //Rings like BRG space to be the same as the RGB strip
  LEDS.addLeds<WS2812SERIAL,  LASER_DATA_PIN,   BRG>(laserLEDS,   LASER_NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL,  COCKPIT_DATA_PIN, BRG>(cockpitLEDS, COCKPIT_NUM_LEDS); 
  LEDS.addLeds<WS2812SERIAL,  ENGINE_DATA_PIN,  BRG>(engineLEDS,  ENGINE_NUM_LEDS);
    
  LEDS.setBrightness(DEFAULT_BRIGHTNESS);


  //set first LED to white at startup to show proper operation
  FastLED.clear();
  laserLEDS[0] = CRGB(255,255,255);
  cockpitLEDS[0] = CRGB(255,255,255);
  engineLEDS[0] = CRGB(255,255,255);
  FastLED.show();

  Serial.print("DEBUG_INPUT = ");
  Serial.println(DEBUG_INPUT);
  Serial.print("DEBUG_ACTION = ");
  Serial.println(DEBUG_ACTION);
  Serial.print("DEBUG_AUDIO = ");
  Serial.println(DEBUG_AUDIO);

  //set relative volumes by channel
  mixer1.gain(0, LEVEL_CHANNEL0);
  mixer2.gain(0, LEVEL_CHANNEL0);
  mixer1.gain(1, LEVEL_CHANNEL1);
  mixer2.gain(1, LEVEL_CHANNEL1);
  mixer1.gain(2, LEVEL_CHANNEL2);
  mixer2.gain(2, LEVEL_CHANNEL2);

  //USB Host Setup
  Serial.println("USB Host Setup");
  myusb.begin();
  keyboard1.attachPress(OnPress);
  keyboard1.attachRelease(OnRelease);
  
  delay(1000);
  Serial.println("Setup Complete.");
  
}

void loop() {

  updateButtons();
  myusb.Task();

   if (bgmMetro.check() == 1) { // check if the metro has passed its interval 
      //check on background music
      if (bgmStatus && !channels[CHANNEL_MUSIC]->isPlaying()) { 
        
        //play random BACKGND#.WAV
        String fn = "BACKGND";
        fn = fn + random (1, NUM_BGM_WAVS + 1) + ".WAV";
#if DEBUG_AUDIO
        Serial.println("Starting Background Music from bgmMetro");
#endif
        playWAV( CHANNEL_MUSIC, fn);

      }
      //check on engine
      if (engineStatus && !channels[CHANNEL_ENGINE]->isPlaying()) {
        //We want ENGINE1.WAV (baseline engine) playing whenever ENGINE2.WAV (thrust) is NOT playing
#if DEBUG_AUDIO
        Serial.println("Starting Background Engine Sound from bgmMetro");
#endif
        playWAV(CHANNEL_ENGINE, "ENGINE1.WAV");
      }
      
   } //end bgmMetro check

  if (playQueueMetro.check() == 1) { // check if the metro has passed its interval
    for (int q = 0; q < NUM_CHANNELS; q++) {
      String fn = playQueue[q];
      if (fn.length() >0) {
        playQueue[q] = "";
        playWAV(q,fn); 
      }
    }
  }
   
}

void updateButtons() {
    for (int btn = 0; btn< NUM_BUTTONS; btn++) {
      buttons[btn].update();
      if (buttons[btn].fallingEdge()){
#if DEBUG_INPUT
        Serial.print("buttonDown: "); Serial.println(btn);
#endif
        buttonDuration[btn] = millis();
        //moved the action to button release 
        //mapAction(SOURCE_BUTTON, btn, 0);
      }

      //if buttonDuration is zero, ignore since this is a button read on startup
      if (buttons[btn].risingEdge() && buttonDuration[btn]){

        unsigned long duration = millis() - buttonDuration[btn];
#if DEBUG_INPUT
        Serial.print("buttonUp: "); 
        Serial.print(btn);
        Serial.print("; Duration = "); 
        Serial.println(duration);
#endif
        mapAction(SOURCE_BUTTON, btn, duration);
        }
        
    }//end for
}

void OnPress(int key)
{
#if DEBUG_INPUT
  Serial.print("Pressed key: "); Serial.println(key);
#endif
  //mapAction(SOURCE_KEY, key, 0);
  keyHeld = key;
  keyHeldDuration = millis();   
}

void OnRelease(int key)
{
#if DEBUG_INPUT
  Serial.print("Released key: "); Serial.println(key);
#endif

  unsigned long duration = 0;
  
  if (keyHeld == key) {
    duration = millis() - keyHeldDuration;
  }
  
#if DEBUG_INPUT
        Serial.print("KeyUp: "); 
        Serial.print(key);
        Serial.print("; Duration = "); 
        Serial.println(duration);
#endif
  mapAction(SOURCE_KEY, key, duration);   
}

/* 
 *  ACTION CODE
 *  
 */

void mapAction(int src, int key, int data) {
  for (int s = 0; s< ACTION_MAP_SIZE; s++) {
    if (ActionMap[s][0] == src && ActionMap[s][1] == key) {
      processAction(ActionMap[s][2], src, key, data);     
    } //end if
  } //end for
}

void processAction (int action, int src, int key, int data) {
  switch (action) {
   
      case ACTION_TORPEDO:            actionTorpedo(); break;
      case ACTION_LASER:              actionLaser();   break;
      case ACTION_KYLO:               actionKylo(data);    break;
      case ACTION_ENGINE:             actionEngine(data);  break;
      case ACTION_BGM_TOGGLE:         actionBGMToggle(); break;
   
  }
}

void actionTorpedo() {
#if DEBUG_ACTION
  Serial.println("Torpedo away!");
#endif

  //play random TORPEDO#.WAV
  String fn = "TORPEDO";
  fn = fn + random (1, NUM_TORPEDO_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);

  //torpedo LED animation

}

void actionLaser() {
#if DEBUG_ACTION
  Serial.println("Fire the LAZORS!");
#endif

  //laser sound
  //laser LED animation

  //play random LASER#.WAV
  String fn = "LASER";
  fn = fn + random (1, NUM_LASER_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);
    
}

void actionKylo(int data) {
#if DEBUG_ACTION
  Serial.println("Kylo was here!");
#endif
  //cockpit lighting animation?
  if (data > BUTTON_HOLD_DURATION) {
    queueWAV( CHANNEL_SPEECH, "JARJAR.WAV");
    }
  else {
    //play random KYLO#.WAV
    String fn = "KYLO";
    fn = fn + random (1, NUM_KYLO_WAVS + 1) + ".WAV";
    queueWAV( CHANNEL_SPEECH, fn);
    }  
  
}

void actionEngine(int data) {
#if DEBUG_ACTION
  Serial.println("There is no sound in space, but let's make some anyway!");
#endif

  if (data > BUTTON_HOLD_DURATION) {
    engineStatus = !engineStatus;
#if DEBUG_AUDIO
    Serial.print("Engine  status = ");
    Serial.println(engineStatus);
#endif

    if (!engineStatus) channels[CHANNEL_ENGINE]->stop();
    }
  else {
  //engine lighting animation
  queueWAV(CHANNEL_ENGINE, "ENGINE2.WAV");
  }
}

/*
 * actionBGMToggle() is used to toggle the status of Background Music
 * and will stop if the status is now off
 * Note that there is a metro that watches for BGM to finish and restarts, 
 * so we will let that metro start the music once we change the status
 * 
 */
void actionBGMToggle() {

  bgmStatus = !bgmStatus;

#if DEBUG_AUDIO
  Serial.print("Background music status = ");
  Serial.println(bgmStatus);
#endif

  if (!bgmStatus) channels[CHANNEL_MUSIC]->stop();
 
  
}

/*
 * Audio Playback 
 * queueWAV() - this is needed because we can't start audio files
 * during the USB interrupt that generates key messages
 * We add the file to a channel queue (currently only one deep)
 * and there is a queue metro that will check for it and play it
 * 
 */

void queueWAV (int channel, String fn) {
  if (channel < NUM_CHANNELS) {
    playQueue[channel] = fn;
#if DEBUG_AUDIO
    Serial.print ("queueWAV(");
    Serial.print (channel);
    Serial.print (",");
    Serial.print (fn);
    Serial.println (");");   
#endif 
  
  }
}


/*
 * Audio Playback
 * playFile - this plays a specific wav file
 * there is a "debounce" on this to prevent locking behavior
 * DO NOT CALL THIS FROM WITHIN A USB EVENT HANDLER, USE queueWAV instead!
 */

void playWAV (int channel, String fn) {

#if DEBUG_AUDIO
  Serial.print ("playWAV(");
  Serial.print (channel);
  Serial.print (",");
  Serial.print (fn);
  Serial.println (")");  
#endif

  channels[channel]->play(fn.c_str());
  //while this delay is recommended, we are operating properly without it.
  //likely due to debounce and play queuing
  //delay(10);

} //end playWAV
