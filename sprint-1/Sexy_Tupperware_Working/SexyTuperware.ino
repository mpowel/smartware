


//=============================================================================
//  Libraries Used
//=============================================================================
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "neopixel.h"

//=============================================================================
//  Neopixel Controls
//=============================================================================

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN D3
#define PIXEL_TYPE WS2812

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

//=============================================================================
//  Test LED to respond to different temperatures and times
//=============================================================================

int LEDBLUE = D0; //indicates first drop in temp and start of timer
int LEDGREEN = D1;  //indicates <X time has passed and food is still good
int LEDYELLOW = D2;  //indicates >X time has passed and food

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

/*bool isTimerDone = false;  //  Checks if the timer is completed or not
Timer timer(200, TempTimer, true);  // Temperature Timer*/

// Neopixel colors
int red               = 0xff0000;
int blue              = 0x0000ff;
int green             = 0x00ff00;


void setup()
{

  // Low pass filter smoothening array to smooth temp readings
  for(int i = 0; i < filterSamples; i++)
  {
    tempFSampleArray[i]= 0;
  }

  dallas.begin(); //  Initiate the temperature sensor library

  // Cloud variables

  Particle.variable("temperatureF", &temperatureF, DOUBLE);
  pinMode(LEDBLUE, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDYELLOW, OUTPUT);

  //  Initiate the Neopixels. Set them all to off
  strip.begin();
  strip.show();
  strip.setBrightness(1);

  Serial.begin(9600); //  Start the serial monitor

}

//=============================================================================
//  Run this loop Continuously
//=============================================================================

void loop()
{
    //  Continuously check this function when the glvoe is on
    readTemperature();
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
  if(tempFSmoothedValue < 70)  // Temperature based reaction
  {
    TurnOnLEDs();            // Lights turn on when device enters fridge
    digitalWrite(D2, HIGH);  // Blue indicates low temp & start of timer
  }
  else
  {
    TurnOffLEDs();
    digitalWrite(D2, LOW);
  }
}

//  Turn all LEDs off
void TurnOnLEDs()
{
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, blue);
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
