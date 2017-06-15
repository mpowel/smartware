// Figure out how to deal with someone trying to change timer mid-countdown. Can states work?
// Rewire circuit with shorter wires & more predictable temp readings
// webhook for in the fridge  (adjust temp)
// webhook for out of the fridge  (turns off)
// webhook for PHOTON TIME 50%                "Food waste costs the average American family up to $2,000/year, but you're not average are you? ;)"
// webhook for PHOTON TIME 75%                "30-40% of food produced in the US gets wasted. 43% of that happens at the hands of the consumer. Eat leftovers. Check. Save the world. Check."
// webhook for PHOTON TIME 1 day remain


//=============================================================================
// Meet Smartware, your smart tupperware! Food freshness with confidence.
//=============================================================================

                       //    HOW IT WORKS   //

// The user switches the device on and neopixel breathes white light.
// User sets timer through text messaging and the lights blink blue
// (int times based on # of days set).

// To start the timer, just put the tupperware in the fridge. It will start
// once the temperature drops below 40F.

// The next time you open the fridge, you'll be sure not to miss your tupperware
// at the back of the fridge and the countdown will show how much time you have
// to eat it.

// Once time is up, the neopixel blinks yellow. If at any point, the VOC gas
// reading goes above a 600, the neopixel will flash red until the dish is
// removed from the fridge.

// The device automatically turns off when the tupperware temperature sensor
// is >50F for more than 30 mins.

                    //    DEMO ADJUSTMENTS    //

// In the demo version, "Days" are converted to minutes
// Counter starts when temperature is <60F
// Shut off triggered if temp >75F, no time delay
// Depending on exterior encasement, red light may need to change to white

//=============================================================================
//  Libraries Used
//=============================================================================
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "neopixel.h"
#include "math.h"
#include "string"

//=============================================================================
//  State Change
//=============================================================================

# define STATE_WELCOME 0    // Breath white until button is pushed
# define STATE_TURN_ON 1    // Blinks blue twice: Push of button triggers blink, changes state, and starts timer
# define STATE_TIMER 2      // Turn on green LEDs based on preset timer
# define STATE_THROW_OUT 3  // 5 blinks yellow: Time passed set threshold
# define STATE_TURN_OFF 4   // Blinks blue twice (same as turn on)
# define STATE_WAIT 5       // LEDs off. Click button again to turn on
# define STATE_SMS_SENT 6   // State to avoid duplicate SMS messages
# define STATE_GAS_HIGH 7   // Wipe red: Gas sensor >600

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
int state = 0;  // Start things off with breathing white & initial sms
int buttonPin = D1;  // Button for interrupt
int buttonValue = HIGH;  // Button for interrupt initial value
int set_smartware = 7;  // Initial timer set to 7 days
String set_smartware_str = ""; // Global variable for user input coming from app.rb
int INTERVAL_TIME = 0;
int timeElapsed = 0;

bool hasElapsed2 = 0;   // Use as trigger to send SMS after 2 days
bool hasSentElapsed2 = 0;   // Stops duplicates from being sent
bool has3daysLeft = 0;  // Use as trigger to send SMS 3 days before complete
bool hasSent3daysLeft = 0;  // Stops duplicates from being sent
bool has1dayLeft = 0;   // Use as trigger to send SMS 1 day before complete
bool hasSent1dayLeft = 0;   // Stops duplicates from being sent
bool hasSentStinkySMS = 0;

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

//=============================================================================
//  SMS / MMS handler for app.rb responses
//=============================================================================

void handleSMSEvent(const char *event, const char *data) {
  Serial.println("handler started");
  Serial.println(data);   // data is coming from linked app.rb
  set_smartware_str = (data);
}

//=============================================================================
//  Setup
//=============================================================================

void setup(){
  Serial.begin(9600); //  Start the serial monitor
  // Low pass filter smoothening array to smooth temp readings
  for(int i = 0; i < filterSamples; i++){
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
  Time.zone(-4);
  // Attach an interrupt using the button (turns device on)
  pinMode( buttonPin, INPUT_PULLUP );
  attachInterrupt( buttonPin, getButtonValueOn, RISING);
  // Initiate the array
  for(int i=0; i<filterGasReading; i++)
  {
    gasReadingArray[i]= 0;
  }
  // Subscribe to incoming messages coming through app.rb
  Particle.subscribe("smart_food/sms/incoming", handleSMSEvent);
}

//=============================================================================
//  Run this loop Continuously
//=============================================================================

void loop(){
    readTemperature(); // Continuously check temp reading
    doMonitorGas(); // Continuously monitor gas sensor
 switch( state ){
   case STATE_WELCOME:     // Breathe white + welcome SMS goes out
     doWelcomeState();
     break;
   case STATE_SMS_SENT:    // Breathe white, stop sending SMS
     doWelcomeState();
     break;
   case STATE_TURN_ON:     // Blink blue
     doTurnOn();
     break;
   case STATE_TIMER:       // Green count down
     doCountDown();
     break;
   case STATE_GAS_HIGH:   // Blink red: Gas sensor >600
     blinkRed(1);
     break;
   case STATE_THROW_OUT:   // Blink yellow, time elapsed
		 doThrowOutReminder();
     break;
   case STATE_TURN_OFF:    // Blinks blue twice then shuts off
     doTurnOff();
     break;
 }
delay(50);
}

//=============================================================================
//  Webhooks
//=============================================================================

void doWebhookSMS(){
  if( state == 0 ){
    Particle.publish("smart_food_webhook_sms", "Hey! Smartware here. Put me in the fridge to start my 7 day timer or reply SET to set the timer manually.", PRIVATE);
    state = STATE_SMS_SENT;
  }else if( state == STATE_TURN_ON ){
    Particle.publish("smart_food_webhook_sms", "Aaa, nice and cool. Your smartware timer has started.", PRIVATE);
  }else if( state == 1 ){
    Particle.publish("smart_food_webhook_sms", "Aaa, nice and cool. Your smartware timer has started.", PRIVATE);
  }else if( state == 2 && hasElapsed2 == 1 && hasSentElapsed2 == 0 ){ // Send SMS after 2 days
    Particle.publish("smart_food_webhook_sms", "Food waste costs the average American family up to $2,000/year, but youre not average are you? ;)", PRIVATE);
  }else if( state == 2 && has3daysLeft == 1 && hasSent3daysLeft == 0 ){ // Send SMS 3 days before complete
    Particle.publish("smart_food_webhook_sms", "30-40 percent of food produced in the US gets wasted. 43 percent of that happens at the hands of the consumer. Eat leftovers. Check. Save the world. Check.", PRIVATE);
  }else if( state == 2 && has1dayLeft == 1 && hasSent1dayLeft == 0 ){ // Send SMS 1 day before complete
    Particle.publish("smart_food_webhook_sms", "Remember to eat your leftovers! The smartware timer ends tomorrow.", PRIVATE);
  }else if( state == STATE_TURN_OFF ){
    Particle.publish("smart_food_webhook_sms", "I hope you enjoyed your leftovers! Shutting down now.", PRIVATE);
  }else if( state == STATE_GAS_HIGH  && hasSentStinkySMS == 0){
    Particle.publish("smart_food_webhook_sms", "Whewwwww, thats getting ripe. Check your leftovers, my ditigal sniffer detected something funky.", PRIVATE);
    hasSentStinkySMS = 1; // only send one message
  }
}

//=============================================================================
//  Welcome state
//=============================================================================

void doWelcomeState(){
  breatheWhite();
  doWebhookSMS();
}

//=============================================================================
// Blink twice when on button is clicked
//=============================================================================

void doTurnOn(){
  if( state == STATE_TURN_ON ){   // Once the button is pushed, the device turns on
    blinkBlue(5);
    delay(50);
    doWebhookSMS();     // Send "timer turned on" SMS triggered by temp
    state = STATE_TIMER;
  }
}

//=============================================================================
// Light transition for duration of the time interval
//=============================================================================

long startedIntervalAt = -1;

// c++ for includes: if(str1.find(str2) != std::string::npos){
/*set_smartware = set_smartware_str.toInt();  // turns user input (string) to int*/

void doCountDown(){

String nan_trick = ":";  // All non integer inputs should include ":"
char * checking; // set up for string function to check if one string contains another substring
checking = strstr(set_smartware_str, nan_trick);

  if( checking != NULL || set_smartware_str == "" ){  // this function checks to see if a response like "bye" or "set" was entered
    set_smartware = 7;   // if NAN or the user hasn't input a value, leave default timer.
  }else{                                         // set smartware based on user input
     set_smartware = set_smartware_str.toInt();  // turn user input (string) to int
    if  ( set_smartware > 30 ) {                 // if user sets something too large
     set_smartware = 30;                         // default max is 30 days
     }
  }

  Serial.println( set_smartware );  // check to make sure we're not getting any NANs

  INTERVAL_TIME = (2000 * set_smartware); // SMS incoming messages used to set duration
  if( startedIntervalAt == -1 ) // if start, keep record of time elapsed
    startedIntervalAt = millis();
  timeElapsed = millis() - startedIntervalAt; // check time passed
  // light up a certain number of pixels based on time elapsed.
  int numPixelsToLight = map( timeElapsed, 0, INTERVAL_TIME, 0, strip.numPixels() );
  for( int i = 0; i < strip.numPixels(); i++ ){
    if( i <= numPixelsToLight ){
      strip.setPixelColor(i, strip.Color( 127,255,0 )); //green chartreuse
    }else{
      strip.setPixelColor(i, 0);  // turn all pixels off
    }
  }
  strip.show();

    if( timeElapsed > INTERVAL_TIME){    // if beyond time, let's move on
      state = STATE_THROW_OUT;
      hasElapsed2 = 0;   // Use as trigger to send SMS after 2 days
      has3daysLeft = 0;  // Use as trigger to send SMS 3 days before complete
      has1dayLeft = 0;   // Use as trigger to send SMS 1 day before complete
      doWebhookSMS();
    }else if(timeElapsed > INTERVAL_TIME - 2000 ){  //&& timeElapsed > INTERVAL_TIME - 2300
      has1dayLeft = 1;
      hasElapsed2 = 0;
      has3daysLeft = 0;
      doWebhookSMS();
      delay(10);
      hasSent1dayLeft = 1;  // Stops duplicates from being sent
    }else if(timeElapsed > INTERVAL_TIME - 6000 ){ //&& timeElapsed > INTERVAL_TIME - 7000
      has3daysLeft = 1;
      hasElapsed2 = 0;
      has1dayLeft = 0;
      doWebhookSMS();
      delay(10);
      hasSent3daysLeft = 1;  // Stops duplicates from being sent
    }else if(timeElapsed > 4000 ){ //&& timeElapsed < 4000
      hasElapsed2 = 1;
      has3daysLeft = 0;
      has1dayLeft = 0;
      doWebhookSMS();
      delay(10);
      hasSentElapsed2 = 1;  // Stops duplicates from being sent
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
    blinkBlue( 2 );
    wipeDownNeoPixels();
    blinkBlue( 5 );
    TurnOffLEDs();
    doWebhookSMS();  // send "device turned off" message triggered by temp
    state = STATE_WAIT;
}

//=============================================================================
//   Button methods
//=============================================================================

void getButtonValueOn(){
  if(state == STATE_WELCOME){ //If STATE_WELCOME, button turns countdown on
    state = STATE_TURN_ON;
  }else if(state == STATE_THROW_OUT || state == STATE_TIMER ){
    state = STATE_TURN_OFF;
  }else{
    state = STATE_WELCOME;  //Otherwise reset to STATE_WELCOME
    doTurnOn();
    startedIntervalAt = -1;
  }
}

//=============================================================================
// Breathing light pattern
//=============================================================================

float breatheWhite(){
  // Calc the sin wave for the breathing white led
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

void wipeDownNeoPixels(){ // Wipe down red
  for (int i=strip.numPixels() - 1; i >= 0; i--) {
    strip.setPixelColor(i, 255, 0, 0 );//red
    strip.show();
    delay( 200 );
  }
}

void blinkBlue( int times ){  // Blink blue int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnBlue();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }
}

void blinkRed( int times ){  // Blink red int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnRed();
    delay(150);
    TurnOffLEDs();
    delay(150);
  }

}

void blinkYellow( int times ){  // Blink yellow int number of times
  for( int i = 0 ; i < times; i++ ){
    TurnOnYellow();
    delay( 150);
    allNeopixelsOff();
    delay( 150);
  }
}

void TurnOnRed(){  // Turn all LEDs on (red)
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, red);
      strip.show();
    }
}

void TurnOnBlue(){   //  Turn all LEDs on (blue)
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, blue);
      strip.show();
    }
}

void TurnOnYellow(){  //  Turn all LEDs on (yellow)
  int i;
  for(i=0; i < 16; i++)
    {
      strip.setPixelColor(i, yellow);
      strip.show();
    }
}

void TurnOffLEDs(){  //  Turn all LEDs off
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

void doMonitorGas(){
  time_t time = Time.now();
    String tStr = Time.format(time, TIME_FORMAT_ISO8601_FULL); // 2017-04-05T07:08:47-05:00
    Serial.print( tStr ); //track the time and date the data was sent
    Serial.print( ", "); // In order to use data in CSV file, separate w/ comma
    gasReadingRawValue = analogRead(A3);  // grab gas sensor reading from pin A0
    if ( gasReadingRawValue <= 0){    // remove extreme values
      delay(1);
    }else if( gasReadingRawValue >= 265 ){
      state = STATE_GAS_HIGH;
      // Flashes red if gas reading goes above normal & still in count down.
      // Normal varies based on food. Current solution doesn't
      doWebhookSMS();
    }else{
      gasReadingRawValue = analogRead(A3);
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
  	gasSensorSampleArray[i] = gasDataValue; // input new data into the oldest slot
  	for (j=0; j<filterGasReading; j++){     // transfer data array into anther array for sorting and averaging
  		sorted[j] = gasSensorSampleArray[j];
  	}
  	done = 0; // flag to know when we're done sorting
  	while(done != 1){ // simple swap sort, sorts numbers from lowest to highest
  		done = 1;
  		for (j = 0; j < (filterGasReading - 1); j++){
  			if (sorted[j] > sorted[j + 1]){ // numbers are out of order - swap
  				temp = sorted[j + 1];
  				sorted [j+1] =  sorted[j] ;
  				sorted [j] = temp;
  				done = 0;
  			}
  		}
  	}
  	// throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
    // +1 makes up for asymmetry caused by integer rounding
  	bottom = max(((filterGasReading * 15)  / 100), 1);
  	top = min((((filterGasReading * 85) / 100) + 1  ), (filterGasReading - 1));
  	k = 0;
  	total = 0;
  	for ( j = bottom; j< top; j++){
  		total += sorted[j];  // total remaining indices
  		k++;
  	}
  	return total / k; // divide by number of samples
  }

//=============================================================================
//  Temperature Control Functions
//=============================================================================

void readTemperature(){ // Function gets temp reading from Dallas temp sensor
  dallas.requestTemperatures();   // Request temperature
  sin( 23423 );
  float tempC = dallas.getTempCByIndex(0);    // Get the temperature in Celcius
  if ( tempC <= -100 || tempC >= 50 ){    // Eemove extreme values
    delay(2);
  }else{
  temperature = (double)tempC;   // Convert to double
  float tempF = DallasTemperature::toFahrenheit( tempC );    // Convert to Fahrenheit
  temperatureF = (double)tempF;   // Convert to double
  }
  tempFSmoothedValue = digitalSmooth( temperatureF , tempFSampleArray ); //  Smooth temp reading
  ////////////////////////////Serial.print( "\tRaw Temp = ");
  ////////////////////////////Serial.print( temperatureF );
  ////////////////////////////Serial.println( ", ");
  Serial.print( "\tSmoothed Temp = ");
  Serial.print( tempFSmoothedValue );
  Serial.println( ", ");

  checkTemperature(); //  Adjust heater output based on temp reading
}

void checkTemperature(){     // Start timer when put in fridge
  if(tempFSmoothedValue > 68 && tempFSmoothedValue < 72 && state == STATE_SMS_SENT){
    state = STATE_TURN_ON;  // Triggers doTurnOn(); Blink, send SMS message, start counter
  }else if(tempFSmoothedValue > 76 && state == STATE_TIMER){ // only runs when countdown is on
    state = STATE_TURN_OFF;  // Triggers doTurnOff (); Blink, send shutdown SMS, turn off
  }else if (tempFSmoothedValue > 76 && state == STATE_THROW_OUT){ // or after countdown passes threshold
    state = STATE_TURN_OFF;  // Triggers doTurnOff (); Blink, send shutdown SMS, turn off
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
	sensorSampleArray[i] = dataValue;    // input new data into the oldest slot
	for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
		sorted[j] = sensorSampleArray[j];
	}
	done = 0; // flag to know when we're done sorting
	while(done != 1){ // simple swap sort, sorts numbers from lowest to highest
		done = 1;
		for (j = 0; j < (filterSamples - 1); j++){
			if (sorted[j] > sorted[j + 1]){ // numbers are out of order - swap
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
		total += sorted[j];  // total remaining indices
		k++;
	}
	return total / k; // divide by number of samples
}
