// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_AM2315.h>
// This #include statement was automatically added by the Particle IDE.
#include <Sunrise.h>

/*
 * Project Terrarium
 * Description: Light, Humidity and Temperature control for a carnivorous plant terrarium.
 * Author: Jade Watkins
 * Updated 11/7/2018
 */

//pin definitions
#define lightpin4k1   D2 //4k light
#define lightpin4k2   D3 //2nd 4k light
#define lightpin3k   D4 //3.5k light
#define lightpin5k    D5 //5k light

#define fanpin  A0
#define heatpin D6
#define fogpin  D7

//sensor connected to the I2C pins: D0 & D1 on the Photon

//variables
float temphigh = 0; //tracks highest temp each day
float templow = 1000; //tracks lowest temp each day
float tempvar = 0; //stores current temperature
float tempceling = 75.0; //temperature at which the heat should be turned off
float tempfloor = 60.0; //temperature at which the heat should be turned on

float humidhigh  = 0; //tracks highest humidity each day
float humidlow = 1000; //tracks lowest humidity each day
float humidvar = 0; //stores current humidity
float humidceling = 70.0; //humidity at which the fogger should be turned off
float humidfloor = 50.0; //humidity at which the fogger should be turned on

unsigned int interval = 900000; //15 minutes in miliseconds

bool updated = false;

Sunrise seattle(47.6062,122.3321); //Sunrise object for Seattle
Adafruit_AM2315 am2315; //object for the sensor I'm using -- yay libraries!

//checks current time vs sunset/sunrise
//uses Sunrise library
int lightfunc()
{
    int nowhour = Time.hour();
    int nowminute = Time.minute();
    int risehour = seattle.sunRiseHour;
    int riseminute = seattle.sunRiseMinute;
    int sethour = seattle.sunSetHour;
    int setminure = seattle.sunSetMinute;
    //if within 15 minutes of sunrise, turn lights on
    if(nowhour == risehour && abs(nowminute - riseminute) < 15){
        DigitalWrite(lightpin4k1, LOW);
        DigitalWrite(lightpin4k2, LOW);
        DigitalWrite(lightpin3k, LOW);
        DigitalWrite(lightpin5k, LOW);
        updated = false; //set flag to false at the beginning of the day
    } 
    //else if within 15 minutes of sunset, turn lights off and update tracker variables
    else if(nowhour == sethour && abs(nowminute - setminute) < 15){
        DigitalWrite(lightpin4k1, HIGH);
        DigitalWrite(lightpin4k2, HIGH);
        DigitalWrite(lightpin3k, HIGH);
        DigitalWrite(lightpin5k, HIGH);
        if(!updated){ //update daily highs and lows once per day
            trackingfunc();
            updated = true; //change flag
        }
    }
}

//reads current temperature and humidity from sensor, updates tempvar and humidvar
void sensorfunc()
{
    tempvar = am2315.readTemperature();
    humidvar = am2315.readHumidity();
}

//simply resets the daily trackers so that each day's highs/lows are new
void resettrackers(){
    temphigh = 0;
    templow = 1000;
    humidhigh = 0;
    humidlow = 1000;
}

//checks current temp and humidity against the highs and lows, and updates highs/lows if needed
void trackingfunc()
{
    //update variables
    if(tempvar > temphigh) temphigh = tempvar;
    else if(tempvar < templow) templow = tempvar;
    if(humidvar > humidhigh) humidhigh = humidvar;
    else if(humidvar < humidlow) humidlow = humidvar;
    
    //publish to the cloud
    Particle.publish("Temperature High: ", temphigh);
    Particle.publish("Temperature Low: ", templow);
    Particle.publish("Humidity High: ", humidhigh);
    Particle.publish("Humidity Low: ", humidlow);
    
    //reset
    resettrackers();
}

//checks current temp and humidity against ceiling/floors and turns off/on as needed
void temphumidfunc()
{
    if(tempvar < tempfloor) DigitalWrite(heatpin, LOW); //if current temp is lower than the temp floor, turn heat on
    else if(tempvar > tempceiling) DigitalWrite(heatpin, HIGH); //else if it's higher than the ceiling, turn heat off
    if(humidvar < humidfloor) DigitalWrite(fogpin, LOW); //if current humidity is lower than the floor, turn fogger on
    else if(humidvar > humidceiling) DigitalWrite(fogpin, HIGH); //else if it's higher than the ceiling, turn fogger off
}

//Timers for checking for updates instead of the loop--don't need to update that often or use weird delays
Timer sensortimer(interval, sensorfunc); //sensor reading
Timer thtimer(interval, temphumidfunc); //turn on/off heater/fogger
Timer lighttimer(interval, lightfunc); //turn on/off lights

// setup() runs once, when the device is first turned on.
void setup() {
  //light pins, fan, heat and fog pins all are digitalWrite (simply on or off)
  pinMode(lightpin4k1, OUTPUT);
  pinMode(lightpin4k2, OUTPUT);
  pinMode(lightpin3_k, OUTPUT);
  pinMode(lightpin5k, OUTPUT);

  Time.zone(-8); //set time zone to current time zone (offset from UTC)
  
  //start the timers--functions will run every 15 minutes
  sensortimer.start();
  thtimer.start();
  lighttimer.start();
  
  //enable i2c communication with sensor
  Serial.begin(9600);
  am2315.begin();

  //start up fans after everything else is finished initilizing (may add greater airflow control later)
  DigitalWrite(fanpin, LOW);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //currently just for debugging outputs
}