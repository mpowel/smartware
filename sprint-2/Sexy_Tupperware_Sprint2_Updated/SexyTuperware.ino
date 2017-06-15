//=============================================================================
//  Future iterations
//=============================================================================

// 1) Constantly check potentiometer reading and adjust countdown duration on the fly
// 2) Have temp >80 for time be total shut off
// 3) Sauder gas sensor and button (attach button to tupperware surface)
// 4) Have the potentiometer set the duration of the coundown &
// 5) Collect VOC data and understand the speed of decay of food (outside the fridge)

//=============================================================================
//  Goal State
//=============================================================================

// Use VOC sensor and food type to automatically set speed of countdown

//=============================================================================
//  What this code should do:
//=============================================================================

// For this demo, the tupperware is always on. The light breaths white while
// it waits for the user to push the button. Once you push the button, the
// lights blink blue twice. This signals a changes of state and starts the timer.
// The user can set the total the timer using a potentiometer, which adjusts
// the speed of the color fade. The color fade goes from blue to green to yellow.
// Once the neopixel turns yellow and the timer is up, the neopixel will flash
// yellow until the dish is removed.

// The device automatically turns off when the tupperware temperature sensor
// is >60F for more than 4 hours.

// In the future, this device will also have a dimmer that lowers the neopixel's
// Strip.brightness once it's inside the fridge and the interior fridge light
// turns off.

//=============================================================================
//  Libraries Used
//=============================================================================
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "neopixel.h"

//=============================================================================
//  State Change
//=============================================================================

# define STATE_WELCOME 0    // Breath white until button is pushed
# define STATE_TURN_ON 1    // Blinks blue twice: Push of button triggers blink, changes state, and starts timer
# define STATE_TIMER 2      //PREV 2DAYS Color fade: Blue (0,191,255) to green (127,255,0) to yellow (255,96,0)
# define STATE_THROW_OUT 3  // PREV STATE 5 Blinks yellow: Time passed set threshold
# define STATE_TURN_OFF 4   // Wipe red: Push button to shut off
# define STATE_WAIT 5       // Final state, nothing happens until turn on interrupt

//Interrupt to start: Push button
//Interrupt to auto shut off: Temp > 70F for > 20 seconds (demo only)
//Future: Light sensor triggers neopixel to dim and turn up

//=============================================================================
//  Neopixel Controls
//=============================================================================

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN D3
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

//=============================================================================
//  Potentiometer Controls
//=============================================================================

int potPin = A5;    // Define a pin that we'll place the pot on
int potReading = 0; // Create a variable to hold the pot reading
int ledPin = D0;    // Define a pin we'll place an LED on
int ledBrightness = 0;  // Create a variable to store the LED brightness.
volatile int numberOfDays = 1;  // Create variable for neopixel countdown duration

//=============================================================================
//  Temperature Sensor Controls
//=============================================================================

OneWire oneWire(A0);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallas(&oneWire);

double temperature        = 0.0;    //  Temperature varibale in Degrees Celsius
double temperatureF       = 0.0;    //  Temperature variable in Degrees Farenheight
double tempFSmoothedValue = 0.0;    //  Smoothed Temperature variable

// Low Pass Filter
#define filterSamples   5
int tempFSampleArray [filterSamples];

//=============================================================================
//  Variable Identification
//=============================================================================

//Definitions for setting up a timer
/*Timer timer(1000, triggerPublish);  //do WELCOME_STATE for 10 seconds
volatile bool itsTime = false;*/

/*int timerDuration = 1000 * 2 ; // Initialize timer duration variable.
                               // Sub "2" for Potentiometer output once set up.
bool isTimerDone = false;      // Checks if the timer is completed or not
Timer timer(timerDuration, TempTimer, true);  // Temperature Timer
volatile bool itsTime = false; // Allows itsTime to be modified*/

//Initialize and store a value in the state to get things started
int state = 0;
//Button for interrupt
int buttonPin = D1;
int buttonValue = HIGH;
//Blue LED to check the button. Delete once set up properly.
int LEDBLUE = D0;

// Neopixel colors
int red               = 0xff0000;
int blue              = 0x0000ff;
int green             = 0x00ff00;
int yellow            = 0xFFFF00;

void setup()
{

  Serial.begin(9600); //  Start the serial monitor

  // Set up the LED for output to test potentiometer
  pinMode(ledPin, OUTPUT);
  // Make potentiometer reading available
  Spark.variable("pot", &potReading, INT);

  // Low pass filter smoothening array to smooth temp readings
  for(int i = 0; i < filterSamples; i++)
  {
    tempFSampleArray[i]= 0;
  }

  //  Initiate the temperature sensor library
  dallas.begin();

  // Tiny blue led to check button status
  pinMode(LEDBLUE, OUTPUT);

  // Cloud variables
  Particle.variable("temperatureF", &temperatureF, DOUBLE);

  //  Initiate the Neopixels. Set them all to off
  strip.begin();
  strip.show();
  strip.setBrightness(3);

  // attach an interrupt using the button (turns device on)
  pinMode( buttonPin, INPUT_PULLUP );
  attachInterrupt( buttonPin, getButtonValueOn, RISING);
  // Check if the button is pressed on not. Remove once code is set up.
  Serial.println("The Button value is: " + digitalRead(buttonPin));
}

//=============================================================================
//  Run this loop Continuously
//=============================================================================

void loop()
{
  // Continuously check this function when tupperware is on.
    readTemperature();

 switch( state ){
   case STATE_WELCOME:     // Breathe white
     doWelcomeState();
     break;
   case STATE_TURN_ON:     // Blink blue
     doTurnOn();
     break;
   case STATE_TIMER:       // Color shift
     doCountDown();
     break;
   case STATE_THROW_OUT:   // Blink yellow
		 doThrowOutReminder();
     break;
   case STATE_TURN_OFF:    // Wipe red
     doTurnOff();
     break;
 }

delay(50);

}


//=============================================================================
//  Welcome state
//=============================================================================

void doWelcomeState() {
  breatheWhite();
  delay(50);
}

//=============================================================================
// Blink twice when on button is clicked
//=============================================================================

void doTurnOn(){

  // Once the button is pushed, the device turns on
  if( state == STATE_TURN_ON ){
    potentPotables(); // Read the potentiometer only once the button is pushed
    blinkBlue(5);
    delay(50);
    state = STATE_TIMER;
  }

}

//=============================================================================
// Light transition for duration of the time interval
//=============================================================================

//int INTERVAL_TIME = 25*60*1000; // 25 minutes
long startedIntervalAt2 = -1;

void doCountDown(){
  int INTERVAL_TIME2 = (10000 * numberOfDays); // 5 seconds  --> update once potentiometer is set up //CHANGE ME CHANGE ME CHANGE ME
  // if this is the beginning, keep a record of the time elapsed
  if( startedIntervalAt2 == -1 )
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

  // if we're beyond the time, let's move on
  if( timeElapsed2 > INTERVAL_TIME2){
    state = STATE_THROW_OUT;
  }
  delay( 100 );
}


//=============================================================================
//  Timer done, time to toss food
//=============================================================================

void doThrowOutReminder(){
    blinkYellow( 2 );
}

//=============================================================================
//  Bring temp sensor to >80, blink Red as warning and turn all lights off
//=============================================================================

void doTurnOff(){
    blinkRed( 2 );
    wipeDownNeoPixels();
    blinkRed( 5 );
    TurnOffLEDs();
    state = STATE_WAIT;
}

//=============================================================================
//   Button methods
//=============================================================================

void getButtonValueOn()
{
  if(state == STATE_WELCOME){  //Attach different interupts depending on the previous state
    digitalWrite(LEDBLUE, HIGH);
    state = STATE_TURN_ON;     //Move into "flashing blue" and then to timer
  }else{
    state = STATE_WELCOME;     //Reset to breathing white
    doTurnOn();
  }

}

//=============================================================================
// Timer helper
//=============================================================================

//Not yet used, but set up in case it's needed.
/*void triggerPublish()
{
    static int count = 0;
    Serial.println(count++);

		itsTime = true;
}*/

//=============================================================================
// Potentiometer helper
//=============================================================================

void potentPotables(){
  // Use analogRead to read the potentiometer reading (0 to 4095)
  potReading = analogRead(potPin);
  // Allows user to set max days willing to store leftovers.
  numberOfDays = ((potReading - 95) / 500) + 2 ;
  Serial.print( "\t# of Days = ");
  Serial.println( numberOfDays );
  // Map this value into the PWM range (0-255) and store as the led brightness
  ledBrightness = map(potReading, 0, 4095, 0, 255);
  analogWrite(ledPin, ledBrightness); // DEBUGGER: fade the LED to the desired brightness
  delay(100);

}

//=============================================================================
// Breathing light pattern
//=============================================================================

/* Calc the sin wave for the breathing white led */
float breatheWhite(){

  float val = (exp(sin(millis()/2000.0*M_PI)) - 0.36787944)*108.0;
  /*Serial.println( val );*/

  uint16_t i;
  uint32_t c = strip.Color(val, val, val);

  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, c );
  }
  strip.show();
}

//=============================================================================
//  Blink Helpers
//=============================================================================

//  Wipe down red
void wipeDownNeoPixels(){

  for (int i=strip.numPixels() - 1; i >= 0; i--) {
    strip.setPixelColor(i, 255, 0, 0 );//red
    strip.show();
    delay( 200 );
  }

}

//  Blink blue int number of times
void blinkBlue( int times ){
// blink n times
  for( int i = 0 ; i < times; i++ ){
    TurnOnBlue();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }
}

//  Blink red int number of times
void blinkRed( int times ){
  // blink n times
  for( int i = 0 ; i < times; i++ ){
    TurnOnRed();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }

}

//  Blink yellow int number of times
void blinkYellow( int times ){
  // blink n times
  for( int i = 0 ; i < times; i++ ){
    TurnOnYellow();
    delay( 150);
    allNeopixelsOff();
    delay( 150);
  }

}

//  Turn all LEDs on (red)
void TurnOnRed()
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, red);
      strip.show();
    }
}

//  Turn all LEDs on (blue)
void TurnOnBlue()
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, blue);
      strip.show();
    }
}

//  Turn all LEDs on (yellow)
void TurnOnYellow()
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, yellow);
      strip.show();
    }
}
//  Turn all LEDs off
void TurnOffLEDs()
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, 0x000000);
      strip.show();
    }
}

//=============================================================================
//    Additional Neopixel Helpers
//=============================================================================

void allNeopixelsOn(){
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color( 10,10,10 ));  // turn all pixels on
  }
  strip.show();
}

void allNeopixelsOff(){
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);  // turn all pixels off
  }
  strip.show();
}


//=============================================================================
//  Temperature Control Functions
//=============================================================================

// This function gets the current temperature reading from the Dallas temp sensor
void readTemperature()
{
  dallas.requestTemperatures();   // Request temperature
  sin( 23423 );
  float tempC = dallas.getTempCByIndex(0);    // get the temperature in Celcius
  if ( tempC <= -100){    // remove extreme values getting read every 3-4 readings
    delay(1);
  } else {
  temperature = (double)tempC;   // convert to double
  float tempF = DallasTemperature::toFahrenheit( tempC );    // convert to Fahrenheit
  temperatureF = (double)tempF;   // convert to double
  }
  //  Smooth the temperature reading
  tempFSmoothedValue = digitalSmooth( temperatureF , tempFSampleArray );

  Serial.print( tempC );
  Serial.print( "\tTemp = ");
  Serial.print( temperatureF );
  Serial.print( "\tSmoothed Temp = ");
  Serial.println( tempFSmoothedValue );

  //  Adjust the heater output based on temperature reading
  checkTemperature();
}

void checkTemperature()
{
  if(tempFSmoothedValue > 80 && state != STATE_WAIT)  // Temperature based reaction
  {
    digitalWrite (D0, LOW);    // Turn off the little blue light
    state = STATE_TURN_OFF;    // Go to final state
  }
  else{}
}

//=============================================================================
//  Low Pass Filter to smooth Temperature readings
//=============================================================================

//Low pass filter to smooth the temerature readings
int digitalSmooth(int dataValue, int * sensorSampleArray ){
	int j, k, temp, top, bottom;
	long total;
	static int i;
	static int sorted[filterSamples];
	boolean done;

	// increment counter and roll over if necc. -  % (modulo operator) rolls over variable
	i = (i + 1) % filterSamples;
	sensorSampleArray[i] = dataValue;                 // input new data into the oldest slot

	for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
		sorted[j] = sensorSampleArray[j];
	}

	// flag to know when we're done sorting
	done = 0;
	// simple swap sort, sorts numbers from lowest to highest
	while(done != 1){
		done = 1;
		for (j = 0; j < (filterSamples - 1); j++){
			if (sorted[j] > sorted[j + 1]){
				// numbers are out of order - swap
				temp = sorted[j + 1];
				sorted [j+1] =  sorted[j] ;
				sorted [j] = temp;
				done = 0;
			}
		}
	}

	// throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
	bottom = max(((filterSamples * 15)  / 100), 1);
	top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));
	// the + 1 is to make up for asymmetry caused by integer rounding

	k = 0;
	total = 0;
	for ( j = bottom; j< top; j++){
		// total remaining indices
		total += sorted[j];
		k++;
	}

	// divide by number of samples
	return total / k;
}
