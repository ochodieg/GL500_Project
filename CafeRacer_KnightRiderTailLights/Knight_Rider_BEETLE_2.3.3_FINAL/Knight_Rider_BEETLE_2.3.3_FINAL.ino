#include <Adafruit_NeoPixel.h>
#define LCOUNT  57              // strip size
#define DOUT    9             // data out
Adafruit_NeoPixel strip(LCOUNT, DOUT, NEO_GRB + NEO_KHZ800);

//------------------------------------------------------------------------
// ------ using HSV ------

uint32_t HSVdrivingLights = strip.ColorHSV(0,255,100);  // slightly darker than hex
uint32_t HSVheadLightIsOn = strip.ColorHSV(0,255,50);   // slightly darker than hex
uint32_t HSVbrake         = strip.ColorHSV(0,255,255);  // same as hex
uint32_t HSVamber         = strip.ColorHSV(65536/6/2);  // slightly darker than hex
uint32_t HSVdarkAmber     = strip.ColorHSV(65536/6/5);  // slightly less reddish than hex
uint32_t HSVreverse       = 0xffffff;


// ------ using HEX ------

uint32_t drivingLights = 0x800000;
uint32_t headLightIsOn = 0x500000;
uint32_t brake         = 0xff0000;
uint32_t amber         = 0xff8800;
uint32_t darkAmber     = 0xff3000;
uint32_t reverse       = 0xffffff;

uint32_t magenta        = strip.Color(255, 0, 255);
uint32_t rgbcolor       = strip.ColorHSV(65536/6/2);
//------------------------------------------------------------------------

// ------ Headlight detection ------
int headLightDetect   = 100;      // input to detect if hL is on
int headLightFlag;              // flag if hL is on

// -------- Brake Detection --------
int   brakeDetect     = A2;      // pin to detect if brake has been activated
bool  brakeFlag;                // variable to to signal if 
bool  maintainedBrake = false;  // variable used to detect if brake is consistent
// ------ knight eye variables -----
int eyeCenter = 0, dIrection = 1;

// ------ Turn Signal Variables ----
int   rightTurn = 11; // input to detect if rT is active
bool  rightTurnFlag; // flag if right turn is active
int   firstPixelRight = 0, rDirection = 1;   // variables used for Right turn() signals

int   leftTurnCount, rightTurnCount;

int   leftTurn = 10;
bool  leftTurnFlag; // flag if right turn is active
int   firstPixelLeft=strip.numPixels(),  lDirection = -1;  // variables used for left turn() signals

// ------ Aux animation variables ---
int rainBow = 100;
bool rainFlag;

char  directions = 'R';  // variable used to test this function


// ------------------------ FUNCTIONS ------------------------


// smooth color tessalation from left to right
void colorWipe(uint32_t color, int wait){       // recall colors are 24 bit and thus need 32 bit word          
  for(int i = 0; i <= strip.numPixels(); i++){  // for every pixel in strip
    strip.setPixelColor(i, color);              // set the pixel a color
    strip.show();                               // update strip
    delay(wait);
  }
}

// colorWipe on opp. direction
void colorWipeR(uint32_t color, int wait){
  for(int i = strip.numPixels(); i >= 0; i--){ // for every pixel starting at end, count back
    strip.setPixelColor(i,color);
    strip.show();
    delay(wait);
  }
}

// slowly brightens tail light to selected color
void brighten(uint32_t HSVcolor, int wait) {
  /*
   * @param: color
   * @param: speed of transition
   * args accept both hex and HSV values
   * examples:
   * brighten(0XFF0000,20);
   * brighten(0,20);
   */
   uint16_t i, j;

  for (j = 0; j < 255; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.ColorHSV(HSVcolor,255,j)); 
    }
    strip.show();
    delay(wait);
  }
}

// slowly brightens tail light to *default brightness*, after startup sequence
void brightenToRun() {
  uint16_t i, j;
  for (j = 0; j < 20; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, j, 0, 0); 
    }
    strip.show();
    delay(20);
  }
}

// brake animation 
void engagedBrake(){  
  /*
   * animation starts at center of strip, slowly 
   * tessalating outwards on both sides at full brightness
   */
  int center = strip.numPixels()/2+1;;                             // center eye of strip
  strip.clear();                               // clear before setting brightness to avoid problems
  strip.setBrightness(255);

  // for loop to expand animation
  for(int i = 0; i<33; i++){                  // length of strip following center
    for(int j = 0;j<255; j+=51){              // brighten by intervals of 51         
      strip.setPixelColor(center+i, j, 0, 0); // pixels right of center
      strip.setPixelColor(center-i, j, 0, 0); // pixel left of center
      strip.show();
      delay(1);
    }
    strip.show();
    delay(1);
  }
}

// turn signal animation
void turn(int brake, char directions){

  int color = 4600;     // variable used to smoothly change color
  int fade  = 255;      // variable used to smoothly desaturate color 

  /*-----------------------------------------------------------------------------
   * For Loop used to draw the turn signal 'eye' every sequence.
   * every main loop() will redraw the eye in its subsequent position  
   * - WHILE turn() IS INVOKED. This is all the function will do until 
   * the end side of the strip is reached. Once the end is reached, the rest  
   * of the code will execute.
   */
  for(int i = 0; i<18; i++){
    switch(directions){
      case 'R':
        strip.setPixelColor(firstPixelRight - i, strip.ColorHSV(color,255,fade));
        break;
      case 'L':
        strip.setPixelColor(firstPixelLeft + i, strip.ColorHSV(color,255,fade));
        break;
    }
    color -= 183;
    fade  -=  10;
  }
//------------------
  strip.show();
  delay(8);
//------------------

//-----------------------------------------------------------------------------------
/*
 * FOR LOOP below redraws trail folowing the "eye", every sequence
 * effectively "destroying" the eye in the process (eye is displayed on strip beforehand)
 */
  for(int j=0; j<= 18; j++) {
    switch(directions){
      case 'R':
        // trail animation will redraw towards the right
        strip.setPixelColor(firstPixelRight-j, strip.ColorHSV(800,255,60));
        break;

      case 'L':
        // trail animation will redraw towards the left
        strip.setPixelColor(firstPixelLeft+j, strip.ColorHSV(800,255,60));
        break;
    } 
  }
//-----------------------------------------------------------------------------------
  /* 
   *  after knight is is destroyed by loop, firstPixelRightition is increased by one pixel
   *  this slowly moves the eye in given direction, 1 'frame' per sequence
   *  switch ase determines if left or right
   */
  switch(directions){
    case 'R':
      firstPixelRight += rDirection; 
      // moving leading pixel to the right direction    
      break;
    case 'L':
      firstPixelLeft -= rDirection;
      // moving leading pixel to the left direction 
      break;
    } 
//----------------------------------------------------------------------------------
  /*
   * if-statement resets 'eye' and clears strip 
   * depending on if right or left turn
   */

  if(directions == 'L' && firstPixelLeft +20 < 0) { // if left edge hit
    firstPixelLeft = strip.numPixels()+1;           // reset firstPixelLeft to lefthand starting position
    // determine how animation will be wiped
    // if brake is on, colorwipe red (0xff0000) with full brightness at full intensity
    if(leftTurnCount>0){
      leftTurnCount --;
    }
    switch(brake){
      case 0:
        colorWipeR(0x000000, 2);
      break;
      case 1:    
        strip.setBrightness(255);
        colorWipeR(0xff0000, 2);
      break;
     }
    delay(240);
  }else if(directions == 'R' && firstPixelRight >= strip.numPixels()+20) {  // if right edge hit
    firstPixelRight = 1;    // reset firstPixelRight to righthand starting position
    if(rightTurnCount>0){
      rightTurnCount --;
    }
      
     //QUARENTINED:
     //rightTurnCount -1;  // USE TO LOOP TURN SEQUENCE 3x UNINTERRUPTED
     
     /* 
      *  switch case wipes animation depending on if brake
      *  has been engaged. if brake is on, colorwipe with full bright at full intensity
      */
     switch(brake){
      case 0:
        colorWipe(0x000000, 2);
      break;
      case 1:    
        strip.setBrightness(255);
        colorWipe(0xff0000, 2);
      break;
     }
     delay(240);
    }
//----------------------------------------------------------------------------------
}

// emergency signals function
void emergencies(){
  /*
   * animation starts at center of strip, slowly 
   * tessalating outwards on both sides at full brightness
   */
  int center = strip.numPixels()/2+1;                             // center eye of strip
  strip.clear();                               // clear before setting brightness to avoid problems
  strip.setBrightness(255);

  // for loop to expand animation
  for(int i = 0; i<33; i++){
    for(int j = 0;j<255; j+=51){                // brighten by intervals of 51
      strip.setPixelColor(center+i, j, 30, 0);  // pixels right of center
      strip.setPixelColor(center-i, j, 30, 0);  // pixel left of center
      strip.show();
      
      strip.setPixelColor(center+i, j, 30, 0);  // pixels right of center
      strip.setPixelColor(center-i, j, 30, 0);  // pixel left of center
      strip.show();
      delay(1);
    }
    strip.show();
    delay(1);
  }
}

// knight rider animation sequence. Updates per loop
void knightSequence(){
  strip.setPixelColor(eyeCenter - 5, 0x800000);          // outerpixels are darkest
  strip.setPixelColor(eyeCenter - 4, strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter - 3, strip.ColorHSV(0)); // red pixels are brightest
  
  strip.setPixelColor(eyeCenter - 2, strip.ColorHSV(65536/6/5)); // Dark Amber
  strip.setPixelColor(eyeCenter - 1, strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter    , strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter + 1, strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter + 2, strip.ColorHSV(65536/6/5)); // Dark Amber

  strip.setPixelColor(eyeCenter + 3, strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter + 4, strip.ColorHSV(0)); // red pixels are brightest
  strip.setPixelColor(eyeCenter + 5, 0x800000);          // outerpixels are darkest
  strip.show();
  delay(7);

  for(int j=-5; j<= 5; j++) {
    switch(headLightFlag){
      case 0: // headlight is off: default brightness increased
        strip.setPixelColor(eyeCenter+j, strip.ColorHSV(0,255,100));  // red with higher intensity
        break;
      case 1: // headlight is on: default brightness is unchanged
       strip.setPixelColor(eyeCenter+j, 0x100000);
       break;
    }  
  }
  eyeCenter += dIrection;
  if(eyeCenter < 0) {                         // if left edge hit
    eyeCenter = 1;                            // reset eyeCenterition to start
    dIrection = -dIrection;                   // change direction from what it was
  }else if(eyeCenter >= strip.numPixels()){   // if right edge hit
    eyeCenter = strip.numPixels() - 2;        // reset eyeCenterition to just before right edge
    dIrection = -dIrection;                   // change direction from what it was
    }
}

// startup animation
void start() {                 
  // ------------------ ignition sequence ------------------
  colorWipe (HSVheadLightIsOn,      5);  
  delay(100);
  colorWipeR(HSVbrake,              5); 
  delay(50);  
  colorWipe (HSVheadLightIsOn,      5);   
  delay(250);

  colorWipeR(HSVdarkAmber,          5);
  delay(100);      
  colorWipe (HSVamber,              5); 
  delay(50);   
  colorWipeR(HSVdarkAmber,          5);
  colorWipeR(strip.Color( 0, 0, 0), 5); //     
  delay(250);;

  strip.setBrightness(255);
  colorWipe(reverse,                5);        
  colorWipe(strip.Color(  0, 0, 0), 5); 
  colorWipeR(reverse,               5); 
  colorWipeR(strip.Color( 0, 0, 0), 5);  
 
  strip.clear();            
  strip.setBrightness(100); 
  // ---------------- end ignition sequence -----------------
  
  brightenToRun();
  delay(1000);
}

//AUX animation
void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {   
    strip.rainbow(firstPixelHue);   
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void setup(){

  pinMode(headLightDetect,INPUT); // sets up headlight detection input
  pinMode(brakeDetect,INPUT);     // sets up brake detect input

  pinMode(rightTurn,  INPUT);     // sets up right detect input
  pinMode(leftTurn,   INPUT);     // sets up left detect input

  pinMode(rainBow, INPUT);

  strip.begin();  //initializing object
  strip.show();   // guide claims this turns OFF pixels but docs claim this "update" idk
  strip.setBrightness(50); // running brightness = 50
                          // Headlight on brightness = 100
                          // brake brightness = 255 (max)
  start();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //headLightFlag = digitalRead(headLightDetect); // check headlight input for determinate flag
                                                // to change intensity and brightness
  brakeFlag     = digitalRead(brakeDetect);     // check brake input for determinate flag
  rightTurnFlag = digitalRead(rightTurn);
  leftTurnFlag  = digitalRead(leftTurn);
  //rainFlag      = digitalRead(rainBow);
  headLightFlag = 0;
  rainFlag      = false;
  
  switch(headLightFlag){
    case 0:                     // if daytime
      strip.setBrightness(150); // increase strip brightness
      break;
    case 1:                     // if night time
      strip.setBrightness(100); // decrease strip brightness
      break;
      }
  if(rainFlag){
    strip.setBrightness(255);
    rainbow(2);
  }else{
      if(leftTurnCount>0 && rightTurnCount<=0){
        turn(brakeFlag,'L');
      }else if(rightTurnCount>0 && leftTurnCount<=0){
        turn(brakeFlag,'R');
      }else{
        if(rightTurnFlag&&leftTurnFlag){
          for(int i = 0; i<3; i++){
            emergencies();
            delay(500);
           }
        }else if(rightTurnFlag || leftTurnFlag){   //if either turn signal is invoked 
          int   firstPixelRight = 0, rDirection = 1;   // variables used for Right turn() signals
          delay(75);
          if(leftTurnFlag){
            leftTurnCount  = 3;
            turn(brakeFlag,'L');  //if-statement to change wipe depending on brake input is already incorporated within method body
          }else{
            rightTurnCount = 3;
            turn(brakeFlag,'R');
         }
        }else if(brakeFlag && !(rightTurnFlag || leftTurnFlag)){
          if(maintainedBrake){      
            strip.setBrightness(255);
            strip.fill(strip.ColorHSV(0,255,255), 0  , strip.numPixels() - 1);
            strip.show();
            maintainedBrake = true;
          }else{
            strip.clear(); // test without
            strip.setBrightness(255);
            engagedBrake();
            maintainedBrake = true;
          }
        }else{ 
          maintainedBrake = false;
          knightSequence();
        }
      }
}
        }
