# Sprint 1: MVP



### Sprint Catalog

Sprint goals: 
1) Get familiar with beacon technology 
2) Think through ambient light patterns as user feedback
3) Research cloud functions from the beacon sent to the particle photon to set off

## Pseudo Code
//STATE 0: Waiting - white breathing light
//Beacon is detected by photon and timer is initiated triggering STATE 1
//STATE 1: Beacon found - green breathing light once beacon found (5seconds)
//STATE 2: Day 2 - Solid green led strip that counts down over x time (speed up to 10 sec for demo)
//STATE 3: Day 4 - Solid blue led strip that counts down over x time
//STATE 4: Day 6 - Solid yellow led strip that counts down over x time
//STATE 5: Day 8 - Flashing yellow once it's time to throw out

//Interrupt!!
//If the beacon is removed completely:
//STATE 0: Waiting - white breathing light

### Sprint Review  

## Color exploration:

void yellow () {
  uint32_t i;
  for(i=0; i< strip.numPixels(); i++)
  strip.setPixelColor(i, 255, 96, 0 );//yellow
  strip.show();
}

void red () {
  uint32_t i;
  for(i=0; i< strip.numPixels(); i++)
  strip.setPixelColor(i, 255, 0, 0 );//red
  strip.show();
}

void green () {
  uint32_t i;
  for(i=0; i< strip.numPixels(); i++)
  strip.setPixelColor(i, 127, 255, 0 );// green chartreuse http://www.tayloredmktg.com/rgb/
  strip.show();
}

void blue () {
  uint32_t i;
  for(i=0; i< strip.numPixels(); i++)
  strip.setPixelColor(i, 0, 191, 255 ); //deep sky blue http://www.tayloredmktg.com/rgb/
  strip.show();
}

Insights:
 - Using red or yellow may cause more waste by triggering user anxiety around bad / rotten food
 - Green is associated with nature and makes for a good positive indicator
 - Blue is a calming color and makes for a good middle indicator
 - Yellow could symbolize warning and may cause users to toss out food too early
 - Red is too strong and would definitely trigger removal. Must be used cautiously
 - MOST IMPORTANTLY: Lights on all products in fridge will create excessive visual noise


## Beacon Exploration:
Registered my beacon with Google's Developer Console and was able to add "attachments" to the beacon that could simulate the product information (type of food, date purchased, etc.) currently stored in the barcode.

Researched Nearby Messaging and Firebase Cloud Messaging to explore how I might connect my beacon's data with my particle photon.

Came to the realization that even with RFID tags, which are less bulky and less wasteful than beacons, it would still be a huge overhead cost for grocers or food producers to add this technology to their POS system. Pallets and other commercial packaging is controlled by that producer, but once it passes on to the consumer, the tag is thrown out.

This seems like a dead end.

### End of Sprint Prototype

#include "neopixel.h"
//Beacon is detected by my photon and timer is initiated:
//STATE 0: Waiting - white breathing light
//STATE 1: Beacon found - green breathing light once beacon found (5seconds)
//STATE 2: Day 2 - Solid green led strip that counts down over x time (speed up to 10 sec for demo)
//STATE 3: Day 4 - Solid blue led strip that counts down over x time
//STATE 4: Day 6 - Solid yellow led strip that counts down over x time
//STATE 5: Day 8 - Flashing yellow once it's time to throw out

//Interrupt!!
//If the beacon is removed completely:
//STATE 0: Waiting - white breathing light

#define STATE_WELCOME 0
#define STATE_BEACON_FOUND 1
#define STATE_2DAYS 2   //green chartreuse (127,255,0)
#define STATE_4DAYS 3   //deep sky blue    (0,191,255)
#define STATE_6DAYS 4   //yellow           (255,96,0)
#define STATE_THROW_OUT 5

// we also need a variable to store the state...

int state = 0;

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 16
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);



void setup() {

 Serial.begin(9600);
 Serial.println("setup complete");

 // add the neopixels
 strip.setBrightness(5);
 strip.begin();
 strip.show();  
 // Initialize all pixels to 'off'



}


void loop() {

 switch( state ){
   case STATE_WELCOME:
     doWelcomeState();
     break;
   case STATE_BEACON_FOUND:
     showBeaconFound();
     break;
   case STATE_2DAYS:
     do2DayCountDown();
     break;
   case STATE_4DAYS:
		 do4DayCountDown();
     break;
   case STATE_6DAYS:
		 do6DayCountDown();
     break;
   case STATE_THROW_OUT:
		 doThrowOutReminder();
     break;

 }


}

/***************************

  WELCOME STATE

***************************/

void doWelcomeState(){         //UPDATED UPDATED UPDATED UPDATED

  breatheWhite(5);
  delay(50);
  // The state change to beacon found will eventually be triggered by the beacon entering proximity of the fridge
  state = STATE_BEACON_FOUND;  //UPDATED UPDATED UPDATED UPDATED

}

/***************************

  BEACON FOUND

***************************/

void showBeaconFound(){        //UPDATED UPDATED UPDATED UPDATED

  // We're going to show and do nothing
  // until the right beacon is brought into proximity

  if( state == STATE_BEACON_FOUND ){   //CHANGE ME CHANGE ME CHANGE ME
    breatheWhite(5);
    delay(50);
    state = STATE_2DAYS;      //UPDATED UPDATED UPDATED UPDATED
  }

}


/***************************

  2 DAY COUNT DOWN

***************************/

//int INTERVAL_TIME = 25*60*1000; // 25 minutes   //CHANGE ME CHANGE ME CHANGE ME
int INTERVAL_TIME2 = 10000; // 5 seconds
long startedIntervalAt2 = -1;

void do2DayCountDown(){

  // if this is the beginning...
  // let's keep a record of the time elapsed
  if( startedIntervalAt2 == -1 )
    startedIntervalAt2 = millis();

  // how many milliseconds have elapsed
  int timeElapsed2 = millis() - startedIntervalAt2;

  int numPixelsToLight = map( timeElapsed2, 0, INTERVAL_TIME2, 0, strip.numPixels() );

  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 127,255,0 )); //green chartreuse
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

  // if we're beyond the time, let's move on
  if( timeElapsed2 > INTERVAL_TIME2){    //CHECK CHECK CHECK CHECK CHECK

    state = STATE_4DAYS;                //UPDATED UPDATED UPDATED UPDATED
  }

  delay( 100 );
}

/***************************

  4 DAY COUNT DOWN

***************************/


//int INTERVAL_TIME = 25*60*1000; // 25 minutes   //CHANGE ME CHANGE ME CHANGE ME
int INTERVAL_TIME4 = 10000; // 5 seconds
long startedIntervalAt4 = -1;

void do4DayCountDown(){

  // if this is the beginning...
  // let's keep a record of the time elapsed
  if( startedIntervalAt4 == -1 )
    startedIntervalAt4 = millis();

  // how many milliseconds have elapsed
  int timeElapsed4 = millis() - startedIntervalAt4;

  int numPixelsToLight = map( timeElapsed4, 0, INTERVAL_TIME4, 0, strip.numPixels() );

  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 0,191,255 )); //deep sky blue
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

  // if we're beyond the time, let's move on
  if( timeElapsed4 > INTERVAL_TIME4 ){

    state = STATE_6DAYS;
  }

  delay( 100 );
}

/***************************

  6 DAY COUNT DOWN

***************************/

//int INTERVAL_TIME = 25*60*1000; // 25 minutes   //CHANGE ME CHANGE ME CHANGE ME
int INTERVAL_TIME6 = 10000; // 5 seconds
long startedIntervalAt6 = -1;

void do6DayCountDown(){

  // if this is the beginning...
  // let's keep a record of the time elapsed
  if( startedIntervalAt6 == -1 )
    startedIntervalAt6 = millis();

  // how many milliseconds have elapsed
  int timeElapsed6 = millis() - startedIntervalAt6;

  int numPixelsToLight = map( timeElapsed6, 0, INTERVAL_TIME6, 0, strip.numPixels() );

  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 255,96,0 )); //yellow
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

  // if we're beyond the time, let's move on
  if( timeElapsed6 > INTERVAL_TIME6 ){

    state = STATE_THROW_OUT;
  }

  delay( 100 );
}

/***************************

  8 DAYS - TIME TO THROW OUT

***************************/


void doThrowOutReminder(){


  wipeDownNeoPixels();

  for( int i = 0; i<5; i++){
    blinkRed( 2 );
  }

  /*startedIntervalAt = -1;
  startedCountAt = -1;*/

  state =  STATE_WELCOME;

}


/***************************

  BREATHING LIGHT PATTERN

***************************/

/* Calc the sin wave for the breathing white led */
float breatheWhite( int times  ){

  float val = (exp(sin(millis()/2000.0*M_PI)) - 0.36787944)*108.0;
  Serial.println( val );

  uint16_t i;
  uint32_t c = strip.Color(val, val, val);

  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, c );
  }
  strip.show();
}


/***************************

  NEPIXEL HELPERS

***************************/

void wipeDownNeoPixels(){

  for (int i=strip.numPixels() - 1; i >= 0; i--) {
    strip.setPixelColor(i, 255, 0, 0 );//red
    strip.show();
    delay( 200 );
  }

}

void blinkRed( int times ){
  // blink n times
  for( int i = 0 ; i < times; i++ ){
    for(int i = 0 ; i < times; i++ )
    strip.setPixelColor(i, 255, 0, 0 );//red
    /*strip.show();*/
    allNeopixelsOn();
    delay( 150);
    allNeopixelsOff();
    delay( 150);
  }

}

void allNeopixelsOn(){
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color( 10,10,10 ));  // turn all pixels off
  }
  strip.show();
}

void allNeopixelsOff(){
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);  // turn all pixels off
  }
  strip.show();
}



### Sprint Retrospective 

Primary insights:
1) It's a wasteful (both cost and resources) to use RFID tags and beacons
2) Everything blicking in your fridge all the time is visual overload

### Next

I'll still be sticking with inventory tracking, but I'm going to change the form factor to a smart tupperware model that uses temperature change to detect duration in the fridge. 

Caveat: only solves for tracking leftovers stored in specially designed packages. Won't solve for produce left in original containers.

Challenges:
 - Keep costs down
 - Technology that can withstand extreme temperatures (microwave & freezer?)
 - Microwave & dishwasher safe
 - Dethawing functionality?

