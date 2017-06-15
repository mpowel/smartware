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

# define STATE_WELCOME 0
# define STATE_BEACON_FOUND 1
# define STATE_2DAYS 2   //green chartreuse (127,255,0)
# define STATE_4DAYS 3   //deep sky blue    (0,191,255)
# define STATE_6DAYS 4   //yellow           (255,96,0)
# define STATE_THROW_OUT 5

// we also need a variable to store the state...

int state = 0;

//No button input for now
/*int buttonPin = D0;
int buttonValue = HIGH;*/

//Definitions for setting up a timer
Timer timer(10000, triggerPublish);  //do WELCOME_STATE for 10 seconds
volatile bool itsTime = false;

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 16
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

//No speaker for now
/*int speakerPin = D2;*/


void setup() {

 Serial.begin(9600);
 Serial.println("setup complete");

 //No button needed
 /*pinMode( buttonPin, INPUT_PULLUP );*/

 // add the neopixels
 strip.begin();
 strip.show();
 strip.setBrightness(5);
 // Initialize all pixels to 'off'

 //Start timer
 timer.start(); // Start timer on set up
//No speaker for now
 /*pinMode( speakerPin, OUTPUT );*/
}


void loop() {
 //attempt at getting the breathe to last longer.... didn't work
 // sprint one is focusing only on the ambient light count down
 /*
while ( itsTime == false ) {
      doWelcomeState();
      Serial.print("while loop is going");
    }

    state = STATE_2DAYS;
    Serial.print("2 Day Count Started");*/

if(state = STATE_2DAYS) {


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

/*}*/


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

   BUTTON METHODS                               //CHANGE ME CHANGE ME CHANGE ME

***************************/


/*void getButtonValue()
{
 buttonValue = digitalRead(buttonPin);
 Serial.print( "buttonValue = " );
 Serial.println( buttonValue );

}*/


/***************************

   SPEAKER METHODS                      //CHANGE ME CHANGE ME CHANGE ME

***************************/


/*void beep(unsigned char delayms){
  analogWrite(speakerPin, 20);      // Almost any value can be used except 0 and 255
                           // experiment to get the best tone
  delay(delayms);          // wait for a delayms ms
  analogWrite(speakerPin, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms
}*/

/***************************

  TIMER HELPER

***************************/

//Not yet used, but set up in case it's needed.
void triggerPublish()
{
    static int count = 0;
    Serial.println(count++);

		itsTime = true;


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

/* Calc the sin wave for the breathing green led */
/*float breatheGreen( ){

  float val = (exp(sin(millis()/2000.0*M_PI)) - 0.36787944)*108.0;
  Serial.println( val );

  uint16_t i;
  uint32_t c = strip.Color(val, val, val);

  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, c );
  }
  strip.show();
}*/


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
