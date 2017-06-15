//=============================================================================
// Notifications
//=============================================================================

// PHOTON TEMP <40                "Aaa, nice and cool. The {data} timer has started."

// PHOTON TEMP >60 for 20 sec     "I hope you enjoyed your leftovers! Shutting down now."

// PHOTON TIME 50%                "Food waste costs the average American family up to $2,000/year, but you're not average are you? ;)"
// PHOTON TIME 75%                "30-40% of food produced in the US gets wasted. 43% of that happens at the hands of the consumer. Eat leftovers. Check. Save the world. Check."
// PHOTON TIME 1 day remain       "Remember to eat your leftovers! The {data} day timer ends tomorrow."

// might be too icky              "Organic waste accounts for 16% of US methane emissions. No methane over here!"

// RUBY == SET                    "Enter a value 1-30 to set the number of days for the timer"
// RUBY 1-3                       "Timer set for {body}. You can now place me in the fridge. Fun fact, savethefood.com says raw meat and fish can be stored up to 3 days. To set a longer timer, type any number greater than 3."
// RUBY 4-30                      "Great! Your timer is set for {body} days. Go ahead and place me in the fridge."
// RUBY != nil                    "Sorry, I did not get that. Type SET to adjust the timer. Otherwise, put me in the fridge."
// RUBY == QUIET                  "Notifications have been turned off."

// RUBY timer_started             "To reset the timer once it starts, please push the reset button."

// NEXT STEPS
//  - Extract ints from inputs on particle .ino side  --> extract?
//  - Make button reset state such that the user can reset the timer
//  - Set up temp to trigger message

// DONE
//  - Make inputs all lowercase

// CAN'T DO IT
//  - Publish timer_started to app.rb  (not possible)

//=============================================================================
//  State Change
//=============================================================================

# define WEBHOOK_1 6        //
# define WEBHOOK_2 7      //

//=============================================================================
//  Variables
//=============================================================================

// Global variables
String userinputstr = "";
int set_smartware = 0;

// Init state variable
int state = 6;


void setup()
{
  Serial.begin(9600); //  Start the serial monitor
  Particle.subscribe("smart_food/sms/incoming", handleSMSEvent);
}

void loop()
{
   hook1Ask();
   delay(1000);
}

//=============================================================================

void hook1Ask(){   // Trigger sms without CLI
  if( state == 6 ){
    Particle.publish("HOOK_1_ASK", "Hey! Smartware here. Put me in the fridge to start my 7 day timer or reply SET to set the timer manually", PRIVATE);
    state = WEBHOOK_2;
  }

}


//=============================================================================
//   Handler to subscribe to a response
//=============================================================================


void handleSMSEvent(const char *event, const char *data) {
  Serial.println("handler started");
  Serial.println(data);
  set_smartware = data.toInt();

}

void gotWeatherData(const char *name, const char *data) {
    String str = String(data);
    String locationStr = tryExtractString(str, "<location>",
"</location>");
    String weatherStr = tryExtractString(str, "<weather>",
"</weather>");
    String tempStr = tryExtractString(str, "<temp_f>", "</temp_f>");
    String windStr = tryExtractString(str, "<wind_string>",
"</wind_string>");

    if (locationStr != NULL) {
        Serial.println("At location: " + locationStr);
    }

    if (weatherStr != NULL) {
        Serial.println("The weather is: " + weatherStr);
    }

    if (tempStr != NULL) {
        Serial.println("The temp is: " + tempStr + String(" *F"));
    }

    if (windStr != NULL) {
        Serial.println("The wind is: " + windStr);
    }
}

// Returns any text found between a start and end string inside 'str'
// example: startfooend  -> returns foo
String tryExtractInt(String str, const char* start, const char* end)
{
    if (str == NULL) {
        return NULL;
    }

    int idx = str.indexOf(start);
    if (idx < 0) {
        return NULL;
    }

    int endIdx = str.indexOf(end);
    if (endIdx < 0) {
        return NULL;
    }

    return str.substring(idx + strlen(start), endIdx);
}
