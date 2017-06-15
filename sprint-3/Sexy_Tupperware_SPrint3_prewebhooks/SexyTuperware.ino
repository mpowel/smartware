//=============================================================================
//  Future iterations
//=============================================================================

// Have temp >75 for time be total shut off
// Sauder gas sensor and button (attach button to tupperware surface)
// Have gas sensor & cloud reading set the duration of the coundown &
// Collect VOC data and understand the speed of decay of food (outside the fridge)

//=============================================================================
//  What this code should do:
//=============================================================================

// For this demo, the tupperware is always on. The light breaths white while
// it waits for the user to push the button. Once you push the button, the
// lights blink blue twice. This signals a changes of state and starts the timer.
// The user can set the total the timer using an app, which adjusts
// the speed of the green countdown. Once the neopixel turns yellow and the
// timer is up OR if the VOC gas reading determines the food is bad, the
// neopixel will flash yellow until the dish is removed.

// The device automatically turns off when the tupperware temperature sensor
// is >50F for more than 4 hours. (Shut off is when >75 and immediate for demo)

// In the future, this device could also have a dimmer that lowers the neopixel's
// Strip.brightness once it's inside the fridge and the interior fridge light
// turns off.

//=============================================================================
//  Libraries Used
//=============================================================================
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "neopixel.h"
#include "math.h"

//=============================================================================
//  State Change
//=============================================================================

# define STATE_WELCOME 0    // Breath white until button is pushed
# define STATE_TURN_ON 1    // Blinks blue twice: Push of button triggers blink, changes state, and starts timer
# define STATE_TIMER 2      //PREV 2DAYS Color fade: Blue (0,191,255) to green (127,255,0) to yellow (255,96,0)
# define STATE_THROW_OUT 3  // PREV STATE 5 Blinks yellow: Time passed set threshold
# define STATE_TURN_OFF 4   // Wipe red: Push button to shut off
# define STATE_WAIT 5       // Final state, nothing happens until turn on interrupt

//=============================================================================
//  Neopixel Controls
//=============================================================================

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN D3
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

//=============================================================================
//  Temperature Sensor Controls
//=============================================================================

OneWire oneWire(A0);  // Pass oneWire reference to Dallas Temperature.
DallasTemperature dallas(&oneWire);

double temperature        = 0.0;    //  Temperature varibale in Degrees Celsius
double temperatureF       = 0.0;    //  Temperature variable in Degrees Farenheight
double tempFSmoothedValue = 0.0;    //  Smoothed Temperature variable

// Low Pass Filter (temp)
#define filterSamples   5
int tempFSampleArray [filterSamples];

//=============================================================================
//  Variable Identification
//=============================================================================

//Initialize and store a value in the state to get things started
int state = 0;
int buttonPin = D1;  //Button for interrupt
int buttonValue = HIGH;

/*int GasReading = 0; // init variable to publish to cloud*/

// Low Pass Filter (gas)
#define filterGasReading   12
int gasReadingArray [filterGasReading];   // array for holding raw sensor values for sensor2
int gasReadingRawValue = 0;
int gasReadingSmoothedValue = 0;

// Neopixel colors
int red               = 0xff0000;
int blue              = 0x0000ff;
int green             = 0x00ff00;
int yellow            = 0xFFFF00;

void setup()
{

  Serial.begin(9600); //  Start the serial monitor

  // Low pass filter smoothening array to smooth temp readings
  for(int i = 0; i < filterSamples; i++)
  {
    tempFSampleArray[i]= 0;
  }

  //  Initiate the temperature sensor library
  dallas.begin();

  // Cloud variables
  Particle.variable("temperatureF", &temperatureF, DOUBLE);
  Particle.variable("melsGasVar", &gasReadingSmoothedValue, INT);

  //  Initiate the Neopixels. Set them all to off
  strip.begin();
  strip.show();
  strip.setBrightness(3);

  // Set time to EST and track once started
  Serial.println("VOC sensor is on!");
  Time.zone(-4);
  // attach an interrupt using the button (turns device on)
  pinMode( buttonPin, INPUT_PULLUP );
  attachInterrupt( buttonPin, getButtonValueOn, RISING);

  // init the array
  for(int i=0; i<filterGasReading; i++)
  {
    gasReadingArray[i]= 0;
  }

}

//=============================================================================
//  Run this loop Continuously
//=============================================================================

void loop()
{

    readTemperature(); // Continuously check temp reading

    doMonitoGas(); // Continuously monitor gas sensor

    /*Particle.publish("melsGas", GasReading, PRIVATE);*/

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
}

//=============================================================================
// Blink twice when on button is clicked
//=============================================================================

void doTurnOn(){

  // Once the button is pushed, the device turns on
  if( state == STATE_TURN_ON ){
    blinkBlue(5);
    delay(50);
    state = STATE_TIMER;
  }

}

//=============================================================================
// Light transition for duration of the time interval
//=============================================================================

long startedIntervalAt = -1;

void doCountDown(){
  // 25 minutes would be:  int INTERVAL_TIME = 25*60*1000;
  int INTERVAL_TIME = (10000);
  // if the beginning, keep a record of time elapsed
  if( startedIntervalAt == -1 )
    startedIntervalAt = millis();

  int timeElapsed = millis() - startedIntervalAt; // check time passed

  // light up a certain number of pixels based on time elapsed.                  <---- THIS IS WHERE I CAN FEED IN THE GAS SENSOR
  int numPixelsToLight = map( timeElapsed, 0, INTERVAL_TIME, 0, strip.numPixels() );

  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 127,255,0 )); //green chartreuse
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

  // if we're beyond the time, let's move on
  if( timeElapsed > INTERVAL_TIME){
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
  //If currently in STATE_WELCOME, button turns the countdown on
  if(state == STATE_WELCOME){
    state = STATE_TURN_ON;
  }else{
    //Otherwise the button resets to STATE_WELCOME
    state = STATE_WELCOME;
    doTurnOn();
    startedIntervalAt = -1;
  }

}

//=============================================================================
// Breathing light pattern
//=============================================================================

/* Calc the sin wave for the breathing white led */
float breatheWhite(){

  float val = (exp(sin(millis()/2000.0*M_PI)) - 0.36787944)*108.0;

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

void blinkBlue( int times ){  //  Blink blue int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnBlue();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }
}

void blinkRed( int times ){  //  Blink red int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnRed();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }

}

void blinkYellow( int times ){  //  Blink yellow int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnYellow();
    delay( 150);
    allNeopixelsOff();
    delay( 150);
  }

}

void TurnOnRed()  // Turn all LEDs on (red)
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, red);
      strip.show();
    }
}

void TurnOnBlue()   //  Turn all LEDs on (blue)
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, blue);
      strip.show();
    }
}

void TurnOnYellow()  //  Turn all LEDs on (yellow)
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, yellow);
      strip.show();
    }
}

void TurnOffLEDs()  //  Turn all LEDs off
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
//  VOC Sensor Helper
//=============================================================================

void doMonitoGas(){
  time_t time = Time.now();
    String tStr = Time.format(time, TIME_FORMAT_ISO8601_FULL);
    // 2017-04-05T07:08:47-05:00
    Serial.print( tStr );
    // each value must be followed by a comma
    Serial.print( ", ");

    gasReadingRawValue = analogRead(A5);  // grab gas sensor reading from pin A0

    if ( gasReadingRawValue <= 0){    // remove extreme values
      delay(1);
    } else {
      gasReadingRawValue = analogRead(A5);
      gasReadingSmoothedValue = digitalSmooth( gasReadingRawValue , gasReadingArray );
    }
    Serial.print("\tRaw gas reading:,");
    Serial.print(gasReadingRawValue);
    Serial.print( ",\tSmoothed gas reading:,");
    Serial.print(gasReadingSmoothedValue);
    Serial.print( ",");
  }


  //=============================================================================
  //  Low Pass Filter to smooth gas readings
  //=============================================================================

  int digitalSmoothGas(int gasDataValue, int * gasSensorSampleArray ){
  	int j, k, temp, top, bottom;
  	long total;
  	static int i;
  	static int sorted[filterGasReading];
  	boolean done;

  	// increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  	i = (i + 1) % filterGasReading;
  	gasSensorSampleArray[i] = gasDataValue;                 // input new data into the oldest slot

  	for (j=0; j<filterGasReading; j++){     // transfer data array into anther array for sorting and averaging
  		sorted[j] = gasSensorSampleArray[j];
  	}

  	// flag to know when we're done sorting
  	done = 0;
  	// simple swap sort, sorts numbers from lowest to highest
  	while(done != 1){
  		done = 1;
  		for (j = 0; j < (filterGasReading - 1); j++){
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
  	bottom = max(((filterGasReading * 15)  / 100), 1);
  	top = min((((filterGasReading * 85) / 100) + 1  ), (filterGasReading - 1));
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




//=============================================================================
//  Temperature Control Functions
//=============================================================================

// This function gets the current temperature reading from the Dallas temp sensor
void readTemperature()
{
  dallas.requestTemperatures();   // Request temperature
  sin( 23423 );
  float tempC = dallas.getTempCByIndex(0);    // get the temperature in Celcius
  if ( tempC <= -100 || tempC >= 50 ){    // remove extreme values getting read every 3-4 readings
    delay(2);
  } else {
  temperature = (double)tempC;   // convert to double
  float tempF = DallasTemperature::toFahrenheit( tempC );    // convert to Fahrenheit
  temperatureF = (double)tempF;   // convert to double
  }
  //  Smooth the temperature reading
  tempFSmoothedValue = digitalSmooth( temperatureF , tempFSampleArray );

  Serial.print( "\tRaw Temp = ");
  Serial.print( temperatureF );
  Serial.println( ", ");

  Serial.print( "\tSmoothed Temp = ");
  Serial.print( tempFSmoothedValue );
  Serial.println( ", ");

  //  Adjust the heater output based on temperature reading
  checkTemperature();
}

void checkTemperature()
{
  if(tempFSmoothedValue > 85 && state != STATE_WAIT)  // Temperature based reaction
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
