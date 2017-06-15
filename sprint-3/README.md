# Sprint 3: Connectivity

### Sprint Catalog

Since I removed the potentiometer, which I was going to use to set the duration, I wanted to explore different ways to integrate user input. During this sprint, I experimented with webhooks, IFTTT, and started playing with Particle / Ruby integration.


### Sprint Review  

- Webhook POST: able to send SMS messages based on events published by Particle.
- Webhook GET: wasn't able to subscribe to events published by the user (via Twilio). Turns out the Twilio api doesn't recieve incoming messages.
- IFTTT: Instead of subscribing to Twilio webhook responses (which is what I thought I could do to get around the API limitation), I used IFTTT to subscribe to incoming SMS messages. IFTTT would often have a long delay and sometimes required that I click the "check now" button on the applet before the event was triggered.

There must be a better way...

- A Particle gem for Ruby! Leveraging skills from a previous class, I was able to build a Ruby chatbot app through Twilio and Heroku and get that to publish events to which Particle could subscribe. SUCCESS!

### End of Sprint Prototype

#### Code
For easier troubleshooting, I set up each of the above code in separate folders.

- Webhook POST    INSERT LINK
- Webhook GET     INSERT LINK
- IFTTT           INSERT LINK
- Ruby for Particle  INSERT LINK

#### Form Factor
I had tried laser cutting an acryllic box to hold my device, but the acryllic was too thick, the box too big, and the whole process, while fast, took too long to only find out it was too heavy. I ended up finding a lightweight transparent plastic pencil pouch and started experimenting with how small I could make my encasement. With a smaller encasement, I was encouraged to make my circuit board even more compact. 

INSERT PHOTOS

#### Sodering

Breadboard: Waaaaaaay too big. But, hey, it works!
INSERT PHOTOS

Attempt 1: Cut down two soder boards and connected sensors directly to the particle.
INSERT PHOTOS

Attempt 2: VOC gas sensor is now located under the particle.
INSERT PHOTOS

#### Strawberry Data
I tried collecting reliable data to eventually make the timer set itself automatically based on rate of decay, but there are too many variables and additional testing will be needed. Variables include:
 - Type of food being testing  (limited testing to strawberries for now)
 - Temperature of storage  (tested outside the fridge to speed up decay process to get more data in less time)
 - Number of times tupperware container is opened (opened the container twice for photos during the test, which reset the gas particle concentration)
 - VOC gas sensor catches 4 different types of gas, only 1 of which is an indicator of decay
 - VOC gas sensor is cheap and somewhat unreliable

### Sprint Retrospective 

While the interactions are simple, I'm quite please with where things are heading. From user testing, I learned having SMS integration, especially for tupperware, is a great low friction way for users to interact with the app. Little to no load time 

I'm also excited I was able to refresh my Ruby skills and integrate the two projects together. 


### Next

Put it all together!  (And work on this here form factor)