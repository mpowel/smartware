# Sprint 2: Adding Intelligence

### Sprint Catalog

| Priority |                           Description                               |           Notes          |
|----------|---------------------------------------------------------------------|--------------------------|
|    P0    | Ambient UX for various states (Wait, Start, Countdown, Off)         | Done.
|    P0    | Product shut off triggered by temperature >80F                      | Done.
|    P0    | Create a fixed countdown duration time                              | Done.
|    P1    | Set countdown duration time dynamically using the ROTARY            | Done.
|    P1    | Design a removeable / waterproof housing for electronics	         | Next.
|    P2    | Add VOC gas sensors to monitor food freshness                       | Next.
|    P2    | Set countdown duration time dynamically using the VOC GAS SENSOR    | Advanced feature. Nice to have.
|    P3    | Add "assign to me" and text reminder through ifttt                  | UX. Nice to have.


### Sprint Review  

I had to play quite a bit of catch up, but luckily I only had a few photon melt downs right at the end. Relatively this went smoothly, and because it was smooth I could have (should have) pushed myself to do more with the VOC data. I'll definitely do that this weekend.


### End of Sprint Prototype

The key milestones I accomplished this sprint were:
1) Map out new product design and pivot from fridge beacon tracking to smart tupperware
2) Meet all P0 milestones

Here is a core snippet of my code wherein the potentiometer reading is used to control the countdown duration. See 
[full code here](Sexy_Tupperware_Sprint2_Updated/SexyTuperware.ino).

#include "neopixel.h"

//=============================================================================
//  Potentiometer Controls
//=============================================================================

int potPin = A5; // Define a pin that we'll place the pot on
int potReading = 0; // Create a variable to hold the pot reading
int ledPin = D0; // Define a pin we'll place an LED on
int ledBrightness = 0; // Create a variable to store the LED brightness.
volatile int numberOfDays = 1;  // Create dynamic variable for neopixel countdown duration

//The numberOfDays variable was where I had the most trouble. I still don't fully understand when to use volatile and when not to. Shouldn't all variables be volatile?

//=============================================================================
//  Neopixel Controls
//=============================================================================

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN D3
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Neopixel colors
int red               = 0xff0000;
int blue              = 0x0000ff;
int green             = 0x00ff00;
int yellow            = 0xFFFF00;


void setup(){

  Serial.begin(9600);
  // Set up the LED for output
  pinMode(ledPin, OUTPUT);

  // Create a cloud variable of type integer
  // called 'light' mapped to photoCellReading
  Spark.variable("pot", &potReading, INT);

  //  Initiate the Neopixels. Set them all to off
  strip.begin();
  strip.show();
  strip.setBrightness(3);

}

void loop() {

  potentPotables();
  doCountDown();

}

//=============================================================================
// Light transition for duration of the time interval
//=============================================================================

long startedIntervalAt2 = -1;  // Make sure the timer starts at 0

void doCountDown(){
  int INTERVAL_TIME2 = (10000 * numberOfDays); // 5 seconds  --> update once potentiometer is set up //CHANGE ME CHANGE ME CHANGE ME
  if( startedIntervalAt2 == -1 )   // if this is the beginning, keep a record of the time elapsed
    startedIntervalAt2 = millis();

  // how many milliseconds have elapsed
  int timeElapsed2 = millis() - startedIntervalAt2;

  // light up a certain number of pixels based on time elapsed.
  int numPixelsToLight = map( timeElapsed2, 0, INTERVAL_TIME2, 0, strip.numPixels() );

  Serial.println(INTERVAL_TIME2);

  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 127,255,0 )); //green chartreuse
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

  delay( 100 );
}


//=============================================================================
// Potentiometer helper
//=============================================================================

void potentPotables(){
  potReading = analogRead(potPin);   // Use analogRead to read the potentiometer reading (0 to 4095)
  numberOfDays = ((potReading - 95) / 500) + 2 ;   // Allows user to set max days willing to store leftovers.
  Serial.print( "\t# of Days = ");
  Serial.println( numberOfDays );
  ledBrightness = map(potReading, 0, 4095, 0, 255);   // Map to PWM range (0-255) and store as led brightness
  analogWrite(ledPin, ledBrightness); // DEBUGGER: fade the LED to the desired brightness
  delay(100);

}


### Sprint Retrospective 
What worked well:
Writing the code for each sensor one by one and waiting to combine functionality. This allowed me to debug quickly.

What didn't work well:
Most of the code merge worked, but when I tried to make the potentiometer control the countdown, I had trouble with global, local and volatile variables. I now have a better understanding. Also, I haven't yet been able to detect gas emissions from food. This will be a critical next step and will make the difference between overly gadgetized tupperware and tupperware that is truly "smart". I hope this works out!

### Next

1) Work on the form factor:
 - Sauder gas sensor and protect from water damage
 - Sauder button and attach to tupperware housing
 - Design the form factor as a waterproof insert
 
2) Gather VOC sensor data on rotting raspberries (outside fridge) to correlate ethynol emmission with food rot

3) Constantly check potentiometer (or gas) readings and adjust countdown duration on the fly

## Bonus
Automatically set countdown duration based on rate of decay of the food.


