// Magic Wheelchair - Tie Silencer 
//
// reference: 
// IMPORTANT NOTE: 8.3 FILENAMES FOR WAV AUDIO FILES!
// IMPORTANT NOTE: WAV 44100 STEREO 16BIT

//you can use the serial input with keys 1-9 to toggle
//key 0 will turn them all off so we don't use 0 here

bool debugOptions[10] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0};   //change default here, helpful for startup debugging

                                                        
char* debugOptionsText[10] =  {"", "Input","Audio", "Action", "Peak Audio",
                                "Animation","Animation RGB Dump"};
                                
#define DEBUG_INPUT               1  //input functions 
#define DEBUG_AUDIO               2  //audio functions 
#define DEBUG_ACTION              3  //action functions 
#define DEBUG_PEAK                4  //Peak Audio functions 
#define DEBUG_ANIMATION           5  //Animation functions   //THIS MAY SLOW ANIMATION                            
#define DEBUG_ANIMATION_RGB       6  //Show Full Animation Frame RGB 
                                     //Does not require DEBUG_ANIMATION to be on
                                     //THIS MAY SLOW ANIMATION

 
 /*
 * Hardware Buttons
 */
 
#include <Bounce.h>

#define NUM_BUTTONS 4
uint8_t buttonPins[NUM_BUTTONS] = { 0, 1, 2, 3 };
Bounce buttons[NUM_BUTTONS] = { {buttonPins[0], 250}, {buttonPins[1], 250}, {buttonPins[2], 1000},{buttonPins[3], 1000} };
unsigned long buttonDuration[NUM_BUTTONS];    //holds the last millis() for a falling edge
#define BUTTON_HOLD_DURATION 2000


    
#define BUTTON_LIGHT_TORPEDO  38
#define BUTTON_LIGHT_LASER    37
#define BUTTON_LIGHT_SPEECH   36
#define BUTTON_LIGHT_ENGINE   35

uint8_t buttonLightPins[NUM_BUTTONS] = { BUTTON_LIGHT_TORPEDO, BUTTON_LIGHT_LASER,
                                         BUTTON_LIGHT_SPEECH, BUTTON_LIGHT_ENGINE };

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
AudioMixer4              mixer3;         //xy=347,337
AudioMixer4              mixer2;         //xy=348.16668701171875,238.3333282470703
AudioMixer4              mixer1;         //xy=350.0000305175781,146.1666717529297
AudioMixer4              mixer4;         //xy=351,422
AudioMixer4              mixer5;         //xy=351,503
AudioOutputI2S           i2s1;           //xy=578.1666564941406,152.6666717529297
AudioAnalyzePeak         peak1;          //xy=594,337
AudioAnalyzePeak         peak3;          //xy=595,502
AudioAnalyzePeak         peak2;          //xy=596,421
AudioConnection          patchCord1(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord2(playSdWav2, 0, mixer4, 0);
AudioConnection          patchCord3(playSdWav2, 1, mixer2, 1);
AudioConnection          patchCord4(playSdWav2, 1, mixer4, 1);
AudioConnection          patchCord5(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord6(playSdWav1, 0, mixer3, 0);
AudioConnection          patchCord7(playSdWav1, 1, mixer2, 0);
AudioConnection          patchCord8(playSdWav1, 1, mixer3, 1);
AudioConnection          patchCord9(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord10(playSdWav3, 0, mixer5, 0);
AudioConnection          patchCord11(playSdWav3, 1, mixer2, 2);
AudioConnection          patchCord12(playSdWav3, 1, mixer5, 1);
AudioConnection          patchCord13(mixer3, peak1);
AudioConnection          patchCord14(mixer2, 0, i2s1, 1);
AudioConnection          patchCord15(mixer1, 0, i2s1, 0);
AudioConnection          patchCord16(mixer4, peak2);
AudioConnection          patchCord17(mixer5, peak3);
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



AudioAnalyzePeak  *peakAnalyzers[NUM_CHANNELS] = { &peak1, &peak2, &peak3 };

#define NUM_BGM_WAVS        6  //plays random file //set to 3 for KYLO specific
#define NUM_LASER_WAVS      7  //plays random file
#define NUM_TORPEDO_WAVS    4  //plays random file
#define NUM_ENGINE_WAVS     2  //plays 0 all the time, then 1 when button pressed
#define NUM_KYLO_WAVS       9  //plays random

//LED ALL THE THINGS!
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

#define LASER_NUM_LEDS 16         //will be 16 
#define LASER_DATA_PIN 32
CRGB laserLEDS[LASER_NUM_LEDS];
int laserFrame = 9999;
int torpedoFrame = 9999;

#define LASER_ANIMATION_HEIGHT 60
#define LASER_ANIMATION_WIDTH  16
byte laserAnimation[LASER_ANIMATION_HEIGHT * LASER_ANIMATION_WIDTH * 3];

#define TORPEDO_ANIMATION_HEIGHT 72
#define TORPEDO_ANIMATION_WIDTH  16
byte torpedoAnimation[TORPEDO_ANIMATION_HEIGHT * TORPEDO_ANIMATION_WIDTH * 3];

#define ENGINE_NUM_LEDS 145 
#define ENGINE_DATA_PIN 33
CRGB engineLEDS[ENGINE_NUM_LEDS]; 

//WARNING - ADJUSTING THIS SETTING COULD LEAD TO 
//EXCESS CURRENT DRAW AND POSSIBLE SYSTEM DAMAGE
#define DEFAULT_BRIGHTNESS 96 //WARNING!!!!!!!!!
//DON'T DO IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//Show & Mode Globals

#include <Metro.h> //Include Metro library 

Metro bgmMetro = Metro(100);
Metro playQueueMetro = Metro(50);
Metro animationMetro = Metro(17); //approx 60 frames per second


bool bgmStatus = 1;           //0 = BGM off; 1 = BGM on
bool engineStatus = 1;        //0 = Engine off; 1 = Engine on

//for USB host functions
#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
USBHIDParser hid1(myusb);
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

#define MAX_ACTION_ID                11 //change this if you add actions!
unsigned long lastActionTime[MAX_ACTION_ID]; 


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
  delay(100);

  Serial.println("Setup Started.");
  

  //setup audio system
  AudioMemory(128);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4);

  //set relative volumes by channel
  mixer1.gain(0, LEVEL_CHANNEL0);
  mixer2.gain(0, LEVEL_CHANNEL0);
  mixer1.gain(1, LEVEL_CHANNEL1);
  mixer2.gain(1, LEVEL_CHANNEL1);
  mixer1.gain(2, LEVEL_CHANNEL2);
  mixer2.gain(2, LEVEL_CHANNEL2);

  //these mixers feed the peak analysis
  mixer3.gain(0, 0); //peak1 (bgm)
  mixer3.gain(1, 0);
  mixer4.gain(1,.3); //peak2 (engine)
  mixer4.gain(1,.3);
  mixer5.gain(1, 0); //peak3
  mixer5.gain(1, 0);

  //setup SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  //load laser animation BMP
  loadAnimationBMP("LASER.BMP", laserAnimation, LASER_ANIMATION_HEIGHT, LASER_ANIMATION_WIDTH);
  if (debugOptions[DEBUG_ANIMATION]) {
    printAnimation(laserAnimation, LASER_ANIMATION_HEIGHT, LASER_ANIMATION_WIDTH);
  }
  //load torpedo animation BMP
  loadAnimationBMP("TORPEDO.BMP", torpedoAnimation, TORPEDO_ANIMATION_HEIGHT, TORPEDO_ANIMATION_WIDTH);
  if (debugOptions[DEBUG_ANIMATION]) { 
    printAnimation(torpedoAnimation, TORPEDO_ANIMATION_HEIGHT, TORPEDO_ANIMATION_WIDTH);
  }

  //seed random function
  randomSeed(analogRead(0));

  //setup buttons & button light
  for (int btn = 0; btn< NUM_BUTTONS; btn++) {
    pinMode(buttonPins[btn], INPUT_PULLUP);
    pinMode(buttonLightPins[btn], OUTPUT);  
    buttonDuration[btn] = 0; 
    buttons[btn].update();
  }

  //startup button light animation
  int btnDelay = 50;
  int absBtn = 0;
  int numCycles = 10;
  for (int cycle = 0; cycle < numCycles; cycle++) { 
    for (int btn = 0; btn< NUM_BUTTONS; btn++) { 
      btnDelay = btnDelay - absBtn;
      if (btnDelay <0) btnDelay=0;
      digitalWrite(buttonLightPins[btn], HIGH);
      delay(btnDelay);  
      digitalWrite(buttonLightPins[btn], LOW);
      delay(btnDelay); 
      absBtn++;
    }
  }


  //Setup LEDS
  
  //Validate the LED color order!
  LEDS.addLeds<WS2812SERIAL,  LASER_DATA_PIN,   BRG>(laserLEDS,   LASER_NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL,  ENGINE_DATA_PIN,  BRG>(engineLEDS,  ENGINE_NUM_LEDS);
    
  LEDS.setBrightness(DEFAULT_BRIGHTNESS);

  //set first LED to white at startup to show proper operation
  FastLED.clear();
  laserLEDS[0] = CRGB(255,255,255);
  engineLEDS[0] = CRGB(255,255,255);
  FastLED.show();

  //Setup USB Host
  Serial.println("USB Host Setup");
  myusb.begin();
  keyboard1.attachPress(OnPress);
  keyboard1.attachRelease(OnRelease);
  keyboard1.attachExtrasPress(OnHIDExtrasPress);
  keyboard1.attachExtrasRelease(OnHIDExtrasRelease);
  
  delay(1000);
  Serial.println("Setup Complete.");
  printDebugOptions();
  actionKylo(0);
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
        if (debugOptions[DEBUG_AUDIO]){
          Serial.println("Starting Background Music from bgmMetro");
        }
        playWAV( CHANNEL_MUSIC, fn);

      }
      //check on engine
      if (!channels[CHANNEL_ENGINE]->isPlaying()) {
        //We want ENGINE1.WAV (baseline engine) playing whenever ENGINE2.WAV (thrust) is NOT playing
      if (debugOptions[DEBUG_AUDIO]) {
        Serial.println("Starting Background Engine Sound from bgmMetro");
      }
        playWAV(CHANNEL_ENGINE, "ENGINE1.WAV");
      }
      
   } //end bgmMetro check

  if (playQueueMetro.check() == 1) { // check if theb metro has passed its interval
    for (int q = 0; q < NUM_CHANNELS; q++) {
      String fn = playQueue[q];
      if (fn.length() >0) {
        playQueue[q] = "";
        playWAV(q,fn); 
      }
    }
  } //end playQueueMetro check

  if (animationMetro.check() == 1) { // check if the metro has passed its interval
    //engine animation
    
    if (peakAnalyzers[CHANNEL_ENGINE]->available()) {
      float peak = peakAnalyzers[CHANNEL_ENGINE]->read();
      if (debugOptions[DEBUG_PEAK]) Serial.printf("Engine Peak: %f \n", peak);
      int peakbrt = map(peak,0,1,0,255);
      fill_solid(engineLEDS, ENGINE_NUM_LEDS, CRGB(peakbrt,0,0));
      analogWrite(BUTTON_LIGHT_ENGINE, peakbrt);
    }
    else fill_solid(engineLEDS, ENGINE_NUM_LEDS, CRGB(0,0,0));

    //if weapons sounds are playing, then peak the appropriate button
    if (channels[CHANNEL_WEAPON]->isPlaying() ) {
      if (peakAnalyzers[CHANNEL_WEAPON]->available()) {
        float peak = peakAnalyzers[CHANNEL_WEAPON]->read();
        if (debugOptions[DEBUG_PEAK]) Serial.printf("Weapon Peak: %f \n", peak);
        int peakbrt = map(peak,0,1,0,255);
        unsigned long k = lastActionTime[ACTION_KYLO];
        unsigned long l = lastActionTime[ACTION_LASER];
        unsigned long t = lastActionTime[ACTION_TORPEDO];

        if ( k > l && k > t) {
          //kylo was most recent
          analogWrite(BUTTON_LIGHT_SPEECH, peakbrt);
          analogWrite(BUTTON_LIGHT_LASER, LOW); 
          analogWrite(BUTTON_LIGHT_TORPEDO, LOW);
        }
        else if ( l > k && l > t) {
          analogWrite(BUTTON_LIGHT_LASER, peakbrt);
          analogWrite(BUTTON_LIGHT_TORPEDO, LOW);   
        }
        else if ( t > k && t > l) {
          analogWrite(BUTTON_LIGHT_TORPEDO, peakbrt);
          analogWrite(BUTTON_LIGHT_LASER, LOW);
        }
      }   //end if peak available()
    } //end if isPlaying
    else {
      analogWrite(BUTTON_LIGHT_LASER, LOW); 
      analogWrite(BUTTON_LIGHT_TORPEDO, LOW); 
      analogWrite(BUTTON_LIGHT_SPEECH, LOW);
    }
    
  bool rLaser = animate(&laserFrame, LASER_ANIMATION_HEIGHT, laserAnimation, LASER_NUM_LEDS, laserLEDS);
  bool rTorpedo = animate(&torpedoFrame, TORPEDO_ANIMATION_HEIGHT, torpedoAnimation, LASER_NUM_LEDS, laserLEDS);
  if (!rLaser && !rTorpedo) { 
    fill_solid(laserLEDS, LASER_NUM_LEDS, CRGB(0,0,0)); 
  }
  
    FastLED.show();
  } //end animationMetro Check

  debugOptionsCheck();   
}

void processAction (int action, int src, int key, int data) {
  
  switch (action) {
   
      case ACTION_TORPEDO:            actionTorpedo(data); break;
      case ACTION_LASER:              if (data >= BUTTON_HOLD_DURATION) actionBGMToggle(data);
                                      else actionLaser(data);   
                                      break;
                                                                         
      case ACTION_KYLO:               if (data >= BUTTON_HOLD_DURATION) actionJarJar(data); 
                                      else actionKylo(data);
                                      break;
                                      
      case ACTION_ENGINE:             if (data >= BUTTON_HOLD_DURATION) actionEngineToggle(data);  
                                      else actionEngine(data);
                                      break;
                                      
      case ACTION_BGM_TOGGLE:         actionBGMToggle(data); break;
   
  }
  
}

void actionTorpedo(int holdDuration) {
  if (debugOptions[DEBUG_ACTION]) Serial.println("Torpedo away!");

  //play random TORPEDO#.WAV
  String fn = "TORPEDO";
  fn = fn + random (1, NUM_TORPEDO_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);

  //torpedo LED animation
  laserFrame=9999;
  torpedoFrame=0;
}

void actionLaser(int holdDuration) {
  if (debugOptions[DEBUG_ACTION]) Serial.println("Fire the LAZORS!");
  //laser sound
  //laser LED animation

  //play random LASER#.WAV
  String fn = "LASER";
  fn = fn + random (1, NUM_LASER_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);
  torpedoFrame=9999;
  laserFrame=0; 
}

void actionKylo(int holdDuration) {
  if (debugOptions[DEBUG_ACTION]) Serial.println("Kylo was here!");

    //play random KYLO#.WAV
    String fn = "KYLO";
    fn = fn + random (1, NUM_KYLO_WAVS + 1) + ".WAV";
    queueWAV( CHANNEL_SPEECH, fn);
}

void actionJarJar (int holdDuration) {
  if (debugOptions[DEBUG_ACTION]) Serial.println("EXQUEEEZE ME!");
  queueWAV( CHANNEL_SPEECH, "JARJAR.WAV");
}

void actionEngine(int holdDuration) {
  if (debugOptions[DEBUG_ACTION]) Serial.println("There is no sound in space, but let's make some anyway!");
  
  mixer1.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
  mixer2.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
  
  queueWAV(CHANNEL_ENGINE, "ENGINE2.WAV");

}

/*
 * actionEngineToggle() is used to toggle the status of Engine constant sound
 * and will stop if the status is now off
 * Note that there is a metro that watches for Engine sound to finish and restarts, 
 * so we will let that metro start the sound once we change the status
 * 
 */
void actionEngineToggle (int holdDuration) {
    engineStatus = !engineStatus;

  if (debugOptions[DEBUG_ACTION]) Serial.printf("Background Engine status = %s\n", engineStatus?"HIGH":"LOW");

  if (engineStatus) {
      mixer1.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
      mixer2.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
  }
  else {
      mixer1.gain(CHANNEL_ENGINE, .1);
      mixer2.gain(CHANNEL_ENGINE, .1);
  }
}


/*
 * actionBGMToggle() is used to toggle the status of Background Music
 * and will stop if the status is now off
 * Note that there is a metro that watches for BGM to finish and restarts, 
 * so we will let that metro start the music once we change the status
 * 
 */
void actionBGMToggle(int holdDuration) {
  bgmStatus = !bgmStatus;
  if (debugOptions[DEBUG_ACTION]) Serial.printf("Background music status = %s\n", bgmStatus?"ON":"OFF");
  if (!bgmStatus) channels[CHANNEL_MUSIC]->stop(); 
}

/*
 * HELPER FUNCTIONS
 * There should not be any code specific to this build below
 * 
 */

/*
 * Audio Playback 
 * queueWAV() - this is needed because we can't start audio files
 *              during the USB interrupt that generates key messages
 *              We add the file to a channel queue (currently only one deep)
 *              and there is a queue metro that will check for it and play it
 * 
 */

void queueWAV (int channel, String fn) {
  if (channel < NUM_CHANNELS) {
    playQueue[channel] = fn;
  if (debugOptions[DEBUG_AUDIO]) Serial.printf ("queueWAV(%i, %s)\n", channel, fn.c_str());
  
  }
}

/*
 * Audio Playback
 * playWAV() - this plays a specific wav file from SD card
 *             DO NOT CALL THIS FROM WITHIN A USB EVENT HANDLER, 
 *             USE queueWAV instead!
 */

void playWAV (int channel, String fn) {

  if (debugOptions[DEBUG_AUDIO]) Serial.printf("playWAV(%i, %s)\n", channel, fn.c_str());

  channels[channel]->play(fn.c_str());
  //while this delay is recommended, we are operating properly without it.
  //likely due to debounce and play queuing
  //delay(10);

} //end playWAV


/*
 * Animation
 * animate() - this function places an animation frame into an LED array
 *           note that we pass a pointer to the frame variable
 *           so that we can increment it inside the function if
 *           we are still within the animation
 *  
 */

bool animate(int *frame, int lastFrame, byte ani[], int numLEDS, CRGB leds[]) {
  if (*frame < lastFrame) {

  if (debugOptions[DEBUG_ANIMATION_RGB]) {
  Serial.print("Animation Row:");
  Serial.print(*frame);
  Serial.print("-");  
  Serial.print(millis());
  }
     
    for (int col=0; col < numLEDS; col++) {
      int base = (*frame*numLEDS*3) + (col*3);
      int b = ani[base];
      int g = ani[base + 1 ];
      int r = ani[base + 2 ];

      if (debugOptions[DEBUG_ANIMATION_RGB]) {
        Serial.printf("{%i:%i:%i,%i,%i}", col, base, r, g, b);
      }
  
      if (debugOptions[DEBUG_ANIMATION]) {
        Serial.printf("%s ", (r || g || b)?"*":" ");
      }  

      leds[col] = CRGB(r,g,b);    
    }
    
    if (debugOptions[DEBUG_ANIMATION_RGB] || debugOptions[DEBUG_ANIMATION]) {
        Serial.println("");
    }
    (*frame)++;
    return true;     //we updated the LED array
  } 
  else return false; //we did not update the LED array 
}

/*
 * Animation
 * loadAnimationBMP() - this function loads a BMP animation file from SD Card 
 *                      into an animation array 
 * 
 */
void loadAnimationBMP (const char *fn, byte ani[], int aniHeight, int aniWidth) { 
    Serial.printf("Loading BMP: %s \n", fn);
    
    // Open
    File bmpImage = SD.open(fn, FILE_READ);
    int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);

    // Change their types to int32_t (4byte)
    int32_t width = readNbytesInt(&bmpImage, 0x12, 4);
    int32_t height = readNbytesInt(&bmpImage, 0x16, 4);
    Serial.printf("Animation width: %i, height: %i \n", width, height); 

    if (width > aniWidth) Serial.println ("WARNING: BMP is wider than Array!");
    if (height > aniHeight) Serial.println ("WARNING: BMP is taller than Array!");
    if (width < aniWidth) Serial.println ("WARNING: BMP is narrower than Array!");
    if (height < aniHeight) Serial.println ("WARNING: BMP is shorter than Array!");
    
    int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

    if (pixelsize != 24)
    {
        Serial.println("Image is not 24 bpp");
        while (1);
    }

    bmpImage.seek(dataStartingOffset);//skip bitmap header

    // 24bpp means you have three bytes per pixel, usually B G R

    byte R, G, B;

    for(int32_t i = 0; i < height; i ++) {
        for (int32_t j = 0; j < width; j ++) {
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();

            int base = (aniHeight-1-i) * aniWidth * 3 + (j*3); //invert
            
            if ((i < aniHeight) && (j < aniWidth)) {
              ani[base] = B;
              ani[base+1] = G;
              ani[base+2] = R;
            }
            
            if (debugOptions[DEBUG_ANIMATION]) {
              if ( R || G || B ) Serial.print("*");
                else Serial.print(" ");
            }
        }
        if (debugOptions[DEBUG_ANIMATION]) Serial.print("\n");
    }

    bmpImage.close();

    Serial.printf("Finished loading BMP: %s \n", fn);     
}

/*
 * Animation
 * readNbytesInt - this function is used by loadAnimationBMP
 * 
 */
int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
    if (nBytes > 4)
        return 0;

    p_file->seek(position);

    int32_t weight = 1;
    int32_t result = 0;
    for (; nBytes; nBytes--)
    {
        result += weight * p_file->read();
        weight <<= 8;
    }
    return result;
}

/*
 * Animation
 * printAnimation() - this function prints a representation of an animation
 *                    array to the serial output
 *                    
 */

void printAnimation (byte ani[], int aniHeight, int aniWidth) {
    for(int32_t i = 0; i < aniHeight; i ++) {
        for (int32_t j = 0; j < aniWidth; j ++) {
            int base = (i * aniWidth * 3) + (j*3);
            
            int B = ani[base];
            int G = ani[base + 1];
            int R = ani[base + 2];

            if ( R || G || B ) Serial.print("*");
              else Serial.print(" ");
 
            }
        Serial.print("\n");
    }
}

/*
 * Input
 * updateButtons() - This function reads each of the buttons 
 *                   and calls mapAction with the button info
 *                   
 */
void updateButtons() {
    for (int btn = 0; btn< NUM_BUTTONS; btn++) {
      buttons[btn].update();
      if (buttons[btn].fallingEdge()){
        if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonDown: %i \n", btn);

        buttonDuration[btn] = millis();
        //moved the action to button release 
        //mapAction(SOURCE_BUTTON, btn, 0);
      }

      //if buttonDuration is zero, ignore since this is a button read on startup
      if (buttons[btn].risingEdge() && buttonDuration[btn]){

        unsigned long duration = millis() - buttonDuration[btn];

        if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonUp: %i; Duration = %i \n", btn, duration); 

        mapAction(SOURCE_BUTTON, btn, duration);
        }
        
    }//end for
}

/*
 * Input
 * onPress() - this function is called by the USB Host HID event when a key is pressed
 * 
 */
void OnPress(int key)
{
  if (debugOptions[DEBUG_INPUT]) Serial.printf("Pressed key: %i \n", key);

  //mapAction(SOURCE_KEY, key, 0);
  keyHeld = key;
  keyHeldDuration = millis();   
}

/*
 * Input
 * onRelease() - this function is called by the USB Host HID event when a key is released
 * 
 */
void OnRelease(int key)
{
  if (debugOptions[DEBUG_INPUT]) Serial.printf("Released key: %i \n", key);

  unsigned long duration = 0;
  
  if (keyHeld == key) {
    duration = millis() - keyHeldDuration;
  }
  
  if (debugOptions[DEBUG_INPUT]) Serial.printf("KeyUp: %i; Duration = %i \n", key, duration); 

  mapAction(SOURCE_KEY, key, duration);   
}

/*
 * Input
 * OnHIDExtrasPress() - this function is called by the USB Host HID event when a MULTIMEDIA key is pressed
 * 
 */
void OnHIDExtrasPress(uint32_t top, uint16_t key) 
{
   OnPress(key);
}

/*
 * Input
 * OnHIDExtrasReleased() - this function is called by the USB Host HID event when a MULTIMEDIA key is released
 * 
 */
void OnHIDExtrasRelease(uint32_t top, uint16_t key) 
{
  OnRelease(key);
}


/* 
 *  Action
 *  mapAction() - this function maps a key, note, button, etc to an action
 *                and then calls the function for that action
 *  
 */

void mapAction(int src, int key, int data) {
  for (int s = 0; s< ACTION_MAP_SIZE; s++) {
    if (ActionMap[s][0] == src && ActionMap[s][1] == key) {
      int action = ActionMap[s][2];
      lastActionTime[action] = millis();
      processAction(action, src, key, data);     
    } //end if
  } //end for
}

/*
 * debugOptionsCheck() - this function checks the Serial input and
 *                       changes debug options as well as emulates some 
 *                       button presses
 *                       
 */
void debugOptionsCheck() {
  int incomingByte = 0;
  
  if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      int option;
      
      switch (incomingByte) {
          case '0':                         //turn all options off
            for (int o = 1; o<10; o++) {    //we dont use zero
              debugOptions[o] = 0;
            }
            Serial.println ("All debug options turned OFF");
            break;
            
          case '1' ... '9': 
            option = incomingByte - '0';
            debugOptions[option] = !debugOptions[option]; 
            //Serial.printf("Debug option %d is now %s\n", option, debugOptions[option]?"ON":"OFF");
            break;

          case 'q': mapAction(SOURCE_BUTTON, 1, 0); break;
          case 'w': mapAction(SOURCE_BUTTON, 2, 0); break;
          case 'e': mapAction(SOURCE_BUTTON, 3, 0); break;
          case 'r': mapAction(SOURCE_BUTTON, 4, 0); break;
          }
         
         printDebugOptions();
          
      }
       
}

/*
 * printDebugOptions() - this function outputs a debug option list with
 *                       with current status, and provides some input 
 *                       instructions
 *  
 */
void printDebugOptions() {
  Serial.println("\nDebug Options Status");
  Serial.println("Use serial input keys 1 through 9 to change debug options");
  Serial.println("Use serial input keys QWER to emulate buttons 1 through 4");
  
  for (int o=1; o<10; o++) {    //we don't use zero
    {
      if (debugOptionsText[o]) //don't print undefined options
        Serial.printf("   Option %d = %s %s\n", o, debugOptions[o]?"ON: ":"OFF:", debugOptionsText[o]);
    }
  }
  Serial.println("\n");       //a little extra padding in the output
}
 
 
