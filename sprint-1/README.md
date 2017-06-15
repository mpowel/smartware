# Sprint 2: Adding Intelligence

### Sprint Catalog

Need to re-create

### Sprint Review  

What I did:
 - Got all my new components (have yet to update the BOM)
 - Fixed/refined my existing code
 - Found all future code I'll need (except the light sensor, which I'm not convinced is needed)

### Explanation of Files

Folder Name: Sexy_Tupperware_Working
Fix made: Main working file. This has the smooth temp sensor readings and a blue LED (triggered by drop in temp)

Folder Name: slide_potent_day
Fix made: Set up potentiometer as "timer duration" (between 2-10 days)
Fix made: Feed potentiometer values into neopixel timer countdown

Folder Name: Stateful_Working_advaitedits
Fix made: Add interrupt to light patterns to allow both breathing and count down (added an interrupt and a button) 

Fix made: Design light pattern options (see below)

### End of Sprint Prototype

Coming soon... 

### Sprint Retrospective 

Simplified light patterns for better UX. No more Las Vegas in the fridge.


### Up Next (by Sprint 2)

Connect all functions for OPTION 1A: Preset counter for 7 days SIMPLE DESIGN (starts with push of button)

1 A) Configure stages of light readings
  - Lights off: Waiting
  - Blinks twice: Push of button triggers blink, changes state, and starts timer
  - Color fade: Blue to green to yellow
  - Blinks yellow: Time passed set threshold
  - Auto turn off neopixel: Temp > 60F for > 4 hours
  
Make final decisions about other options and choose which one to build for the next sprint.

### Sprint 3

OPTION 1B: Preset counter for 7 days TECHNICAL DESIGN (starts with temp drop + power saver mode when in dark + system shut off when warm for > 4 hours)
  - Lights off: Waiting
  - Light countdown: Placed in fridge (timer auto starts when Temp < 30F)
  - Blinks yellow: Time passed set threshold
  - Auto turn off: Temp > 60F for > 4 hours
  - Light sensor triggers neopixel to dim and turn up
    (not turn off because that might destroy the neopixel counting)


OPTION 2: Custom counter for 5-10 days (Start of timer triggerd by "4095" reading from potentiometer, duration and set right after)
  - Lights off: Waiting
  - Blinks blue: Timer started (potentiometer goes to 10)
  - Light countdown: Timer set (you can see it start to move)
  - Blinks yellow: Time passed set threshold
  - Temperature sensor could be used to auto turn off (if warm for > 4 hours)
  - Light sensor triggers neopixel to dim and turn up
    (not turn off because that might destroy the neopixel counting)
