// Magic Wheelchair - Tie Silencer 
//
// reference: Using USB MIDI - https://www.pjrc.com/teensy/td_midi.html
// reference: 
// IMPORTANT NOTE: 8.3 FILENAMES FOR WAV AUDIO FILES!

/*
 * Todo:
 * Redo the audio includes with the audio design tool
 * 
 * 
 */



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

AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

// GUItool: end automatically generated code


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

#define DEFAULT_BRIGHTNESS 255

//Show & Mode Globals

#include <Metro.h> //Include Metro library
Metro showMetro = Metro(100); 
Metro fftMetro = Metro(50); 
Metro chaseMetro = Metro(20); 


#define SHOW_DEFAULT  0
#define SHOW_FLARE    1
#define SHOW_SPEAKING 2
#define SHOW_PLAYING  3
#define SHOW_HONKING  4

#define NUM_MODES       8
#define DEFAULT_MODE    3

#define MODE_OFF            0
#define MODE_FFT_SPLIT      1
#define MODE_FFT_STRAIGHT   2
#define MODE_PEAK_SPLIT     3
#define MODE_PEAK_STRAIGHT  4
#define MODE_CHASE1         5
#define MODE_CHASE2         6
#define MODE_CHASE3         7


int currentShow = 0; //shows = speaking / playing, honking, default, off
int currentMode = DEFAULT_MODE; //modes are what is happening during the default show; fft, chase, etc.

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

#define NUM_VOICE_FILES 26
#define NUM_SOUND_FILES 7


#define SOURCE_KEY                    0
#define SOURCE_BUTTON                 1


#define ACTION_DO_NOT_USE             0 //just putting this here as a reminder to not use it :)

#define ACTION_TORPEDO               1
#define ACTION_LASER                 2
#define ACTION_KYLO                  3
#define ACTION_ENGINE                4
#define ACTION_BACKGROUND_TOGGLE     5


#define ACTION_PLAY_WAV               10
#define ACTION_PLAY_WAV_RND           11




#define ACTION_MAP_SIZE 8

/*
 * The ActionMap allows us to have multiple types of input events
 * that get mapped to an action. We can have a button press, or an HID keyboard
 * key, etc. that all go to the same action
 * 
 */
int ActionMap[][3] = {
  //src, key, action
  {SOURCE_KEY, 214, ACTION_TORPEDO},            //remote right
  {SOURCE_KEY, 211, ACTION_LASER},              //remote left
  {SOURCE_KEY,  27, ACTION_KYLO},               //remote up
  {SOURCE_KEY,  98, ACTION_ENGINE},             //remote down
  {SOURCE_KEY, 198, ACTION_BACKGROUND_TOGGLE},  //remote play
   
};



void setup() {
  Serial.begin(9600);
  AudioMemory(128);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.9);
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

  //USB Host Setup
  Serial.println("USB Host Setup");
  myusb.begin();
  keyboard1.attachPress(OnPress);
  
 
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
  
  delay(1000);
}

void loop() {

  myusb.Task();
 
  if (showMetro.check() == 1) { // check if the metro has passed its interval .
      updateModeShow(); //update the show 10 times a second 
      } //end showMetro check
 
  if (fftMetro.check() == 1) { // check if the metro has passed its interval .
      if (currentShow == SHOW_DEFAULT) {
        switch (currentMode) {
          //do stuff;
         
        } //end switch
      } //end if
    } //end fftMetro check
  
  if (chaseMetro.check() == 1) {
      if (currentShow == SHOW_DEFAULT) {
        switch (currentMode) {
          //do stuff
        } //end switch
      } //end if
  } //end chaseMetro      
}


void updateModeShow() {

 switch (currentShow) {
  
  case SHOW_DEFAULT:  
    break;  
 }

  FastLED.show();
}


void OnPress(int key)
{
  Serial.print("key: "); Serial.println(key);
  mapAction(SOURCE_KEY, key, 0);   
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
    //case ACTION_PLAY_WAV:           actionPlayWAV(data); break;
    //case ACTION_PLAY_WAV_RND:       actionPlayRandomWAV(); break;
 
      case ACTION_TORPEDO:            actionTorpedo(); break;
      case ACTION_LASER:              actionLaser();   break;
      case ACTION_KYLO:               actionKylo();    break;
      case ACTION_ENGINE:             actionEngine();  break;
      case ACTION_BACKGROUND_TOGGLE:  actionBackgroundMusicToggle(); break;
   
  }
}

void actionTorpedo() {
  Serial.println("Torpedo away!");
  //torpedo sound
  //torpedo LED animation

  actionPlayWAV("SDTEST1.WAV");
}

void actionLaser() {
  Serial.println("Fire the LAZORS!");
  //laser sound
  //laser LED animation
  actionPlayWAV("SDTEST2.WAV");
}

void actionKylo() {
  Serial.println("Kylo was here!");
  //random Kylo speech
  //cockpit lighting animation?
  actionPlayWAV("SDTEST3.WAV");
}

void actionEngine() {
  Serial.println("There is no sound in space, but let's make some anyway!");
  //play spaceship sound
  //engine lighting animation
  actionPlayWAV("SDTEST4.WAV");
}

void actionBackgroundMusicToggle() {
  Serial.println("Toggle the background music");
  //if on, then off
  //if off, then on
  //need to write something to watch for file playing and then restart it
  
}


void actionPlayRandomWAV() {
 Serial.println("actionPlayRandomWAV");
 playRandomVoiceFile();
}

void actionPlayWAV (char* filename) {
 //note: this function does not have the rate protection code to prevent button spamming locks
 Serial.print("actionPlayWAV - ");
 Serial.println(filename);
 playFile(filename);
 // playSdWav1.play("HORN.WAV");
 //playFile("SDTEST2.WAV");

}

//needed for string constants in the processAction function
void actionPlayWAV (char const* filename) {
  actionPlayWAV((char*) filename); //cast to char* to call the other function
}


/*
 * Audio Playback
 * playFile - this plays a specific wav file
 * there is a "debounce" on this to prevent locking behavior
 * 
 */

 bool playFile (String fn) {
  unsigned long curMillis = millis();
  unsigned long playDelay = 200;
  unsigned long testVal = (playDelay + lastPlayStart);

  Serial.print ("playFile-");
  Serial.println (fn);
  //Serial.println  (millis());
  //Serial.println (lastPlayStart);
  //Serial.println (testVal);

  //IF NOT PLAYING OR ENOUGH TIME HAS ELAPSED
  if (( playSdWav1.isPlaying() == false) || (curMillis > testVal) ) {
        lastPlayStart = millis();
        playSdWav1.play(fn.c_str());
        delay(10); // wait for library to parse WAV info
          //NOT SURE THIS DELAY WORKS SINCE THREADED CALLS...
        return true;
  } //end if
  else {
    //Serial.println("Ignoring Action due to Play Delay.");
    return false;
  }
} //end playFile


/*
 * Audio Playback
 * playRandomVoiceFile - this plays a random wav file from the voice files
 * there is NO "debounce" on this to prevent locking behavior
 */
int playRandomVoiceFile() {
    
    //pick random number
    int i = random (1, NUM_VOICE_FILES + 1); //random returns up to (max-1)

    //generate filename
    String fn = "/bb-wavs/bb";
    fn = fn + i + ".wav";

     if (playFile(fn)) {
      Serial.print("playRandomVoiceFile: ");
      Serial.println(fn);
     }
    
    return i;
}


/*
 * Audio Playback
 * playRandomSoundFile - this plays a random wav file from the voice files
 * there is NO "debounce" on this to prevent locking behavior
 */
int playRandomSoundFile() {
    
    //pick random number
    int i = random (1, NUM_SOUND_FILES);

    //generate filename
    String fn = "/bb-wavs/bbs";
    fn = fn + i + ".wav";

    if (playFile(fn)) {
      Serial.print("playRandomSoundFile: ");
      Serial.println(fn);
     }
    
    return i;
}
