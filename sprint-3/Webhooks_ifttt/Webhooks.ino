#include <string.h>

//=============================================================================
//  State Change
//=============================================================================

# define STATE_IFTTT_1 6        // Triggers smsGreeting
# define STATE_IFTTT_WAIT 7     // Stops smsGreeting, waits for myHandler
# define STATE_IFTTT_2 8        // Triggers
# define STATE_IFTTT_3 9        //
# define STATE_IFTTT_4 10       //
int state = 6;

//=============================================================================
//  Variables
//=============================================================================
#include <string.h>

String userinputstr = "u";  //User input will be a string between 1 - 4
int userInput = 0; //Turn user input to integer
String timerdurationstr = "t";  //For use in smsTimerSet()
int timerDuration = 0;     //Preset timer will be based on user input


int myHandler(const char *event, const char *data) {
  userinputstr = data;
  userInput = userinputstr.toInt();
  Serial.println(userInput);
  state = STATE_IFTTT_2;
  return 1;

}

void setup() {
  Serial.begin(9600);
  Particle.subscribe("IFTTT_1_REPLY", myHandler);
  Particle.subscribe("IFTTT_4_REPLY", myHandler);
  Particle.variable("userinputstr", userinputstr);
}

void loop() {

switch( state ){
   case STATE_IFTTT_1:     // Triggers IFTTT SMS: What's in your tupperware?
     smsGreeting();
     break;
   case STATE_IFTTT_2:     // Triggers IFTTT SMS: Great! Time set for XX days
     smsTimerSet();
     break;
 }

   delay(100);
}


//=============================================================================
//  SMS Flow 1: What's in your tupperware?
//=============================================================================

void smsGreeting(){
  if (state == STATE_IFTTT_1){
    state = STATE_IFTTT_WAIT;
    Particle.publish("IFTTT_1_ASK","Turned_On");
    delay(1000);
  }
}

//=============================================================================
//  SMS Flow 2: Great! Time set for XX days
//=============================================================================

void smsTimerSet(){
  if (state == STATE_IFTTT_2) {

    String j = "";  //Convert user input to definition of food
    String k = ", great! Timer is set for "; //Confirmation for timer duration
    String l = "";  //Convert timerDuration to string
    String days = " days.";    //Unit

    if (userInput == 1) {
    j = "Fruit";
    j = j + k;
    timerDuration = 5;
    l = (String)timerDuration;
    j = j + l + days;
    Particle.publish("IFTTT_2_CONFIRM", j );  //Great! Time set for XX days
    Particle.publish("IFTTT_3_START","Add_to_Fridge"); //Add to fridge
    }if (userInput == 2) {
      j = "Veggies";
      j = j + k;
      timerDuration = 7;
      l = (String)timerDuration;
      j = j + l + days;
      Particle.publish("IFTTT_2_CONFIRM", j );  //Great! Time set for XX days
      Particle.publish("IFTTT_3_START","Add_to_Fridge"); //Add to fridge
    }if (userInput == 3) {
      j = "Raw Meat";
      j = j + k;
      timerDuration = 3;
      l = (String)timerDuration;
      j = j + l + days;
      Particle.publish("IFTTT_2_CONFIRM", j );  //Great! Time set for XX days
      Particle.publish("IFTTT_3_START","Add_to_Fridge"); //Add to fridge
    }if (userInput == 4) {
      j = "Dairy";
      j = j + k;
      timerDuration = 7;
      l = (String)timerDuration;
      j = j + l + days;
      Particle.publish("IFTTT_2_CONFIRM", j );  //Great! Time set for XX days
      Particle.publish("IFTTT_3_START","Add_to_Fridge"); //Add to fridge
    }else if (userInput == 0) {
      j = "Dairl";
      j = j + k;
      timerDuration = 7;
      l = (String)timerDuration;
      j = j + l + days;
      Particle.publish("IFTTT_2_CONFIRM", j );   //Great! Time set for XX days
      Particle.publish("IFTTT_3_START","Add_to_Fridge"); //Add to fridge
    }if (userInput != 0 || userInput != 1 || userInput != 2 || userInput != 3 || userInput != 4 ) {
      Particle.publish("IFTTT_2_ERROR", "Please enter 0, 1, 2, 3, or 4" );   //New ifttt event name
    }
    state = STATE_IFTTT_3;
    delay(100);
  }
}
