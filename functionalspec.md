# Functional Specification

INPUT: 
 - User sets reminder countdown duration (2-8 days)
 - Device shall constantly monitor environment temperature
 - Device shall constantly monitor environment gas fluxuations

OUTPUT: 
 - Ambient light indicator showing different states & time remaining on countdown
 

## Product Usecase
### Step up 

## Smart Container Product Backlog
| Priority |                           Description                               |           Notes          |
|----------|---------------------------------------------------------------------|--------------------------|
|    P0    | Ambient UX for various states (Wait, Start, Countdown, Off)         | Must. Basic functionality.
|    P0    | Product shut off triggered by temperature > 60F for >3-4 hours)     | Must. Basic functionality.
|    P0    | Create a fixed countdown duration time                              | Temporary feature.
|    P1    | Set countdown duration time dynamically using the ROTARY            | Temporary feature.
|    P1    | Design a removeable / waterproof housing for electronics	         | Must. Basic functionality.
|    P2    | Add VOC gas sensors to monitor food freshness                       | Advanced feature. Nice to have.
|    P2    | Set countdown duration time dynamically using the VOC GAS SENSOR    | Advanced feature. Nice to have.
|    P3    | Add "assign to me" and text reminder through ifttt                  | UX. Nice to have.



//-----------------------------------------

The following product specification was abandoned after the first sprint after discovering the wasteful nature of this product design.
# Blue Tooth or RFID Smart Tags for food
## Smart Tag Functional Specification

DATA COLLECTED (REQUIRED): 
 - Food type 
 - Date added to fridge
 - Time in fridge

DATA COLLECTED (OPTIONAL): 
 - Quantity
 - Estimated expiration date
 
API INTEGRATION
 - Yumly (Recipe suggestions)


## Smart Tag Product Backlog
| Priority |                           Description                               |           Notes          |
|----------|---------------------------------------------------------------------|--------------------------|
|    P0    |Feedback indicating time in fridge and time remaining until expired  | Must. Basic functionality.
|    P0    |Track inventory using BLE beacon or RFID reader/tag                  | Must. Basic functionality.
|    P0    |Feed data to a database                                              | Must. Basic functionality.
|    P0    |Integrate with Slack or sms for UI                                   | Must. Basic functionality to access data.
|    P1    |Connect with Yummly API. Generate recipes using existing ingredients | 2ary feature. Nice to have.
|    P2    |Add VOC gas sensors to custom tupperware to monitor food freshness   | Advanced feature. Nice to have.
|    P2    |Integrate with Google Cloud Vision API to determine Food Type        | Advanced feature. Nice to have.






