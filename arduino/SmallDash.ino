#include <genieArduino.h>
#include <TinyGPS++.h>

// The TinyGPS++ object
TinyGPSPlus gps;
TinyGPSCustom gpsSpeed(gps, "GPVTG", 7);


int ampPin = A0;   // select the input pin for the current sensor
int voltPin = A1;  // select the input pin for the voltage divider
int ampValue = 0;  // variable for measured amps
int voltValue = 0; // varible for measured volts
float speedKph = 0;
float kwValue = 0;
char textToWrite[ 16 ];
int gpsSats = 0;

unsigned long previousMillis = 0;        
const long interval = 1000;  

Genie genie;
#define RESETLINE 4  // Change this if you are not using an Arduino Adaptor Shield Version 2 (see code below)

void setup()
{

  Serial.begin(115200);  // monitor
  Serial1.begin(115200); // gps

  Serial2.begin(56000); // Display
  genie.Begin(Serial2);   // Use Serial2 for talking to the Genie Library, and to the 4D Systems display


  Serial.print("TingGPS++ Version: ");
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println();

  while (!Serial1.available()) {
    //Wait untile serial connected
  }

  genie.AttachEventHandler(myGenieEventHandler); // Attach the user function Event Handler for processing events
  // Reset the Display (change D4 to D2 if you have original 4D Arduino Adaptor)
  pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(RESETLINE, 0);  // unReset the Display via D4
  delay(3500); //let the display start up after the reset (This is important)
  //Turn the Display on (Contrast) - (Not needed but illustrates how)
  genie.WriteContrast(0); // 1 = Display ON, 0 = Display OFF.
  delay(50);
  genie.WriteContrast(1); // 1 = Display ON, 0 = Display OFF.


  //Write a string to the Display to show the version of the library used
  genie.WriteStr(0, "DrifTech");
  genie.WriteObject(GENIE_OBJ_LED, 0x00, 0);
  delay(500);
}

void loop()
{
  consumeGPS();
  readVehicle();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (gps.satellites.value() > 5) {
      displayInfo();
      // Write the GPS Lock... needs 5 Satellites
      if (gpsSats > 4) {
        genie.WriteObject(GENIE_OBJ_LED, 0x00, 1);
      }
    }

  }

  //genie.DoEvents(); // This calls the library each loop to process the queued responses from the display

  // note cool gauge is the frame number so -40 is frame 0 for amp meter.
  genie.WriteObject(GENIE_OBJ_COOL_GAUGE, 0x02, constrain(ampValue, -40, 160) + 40);

  // Calculate and display kW
  genie.WriteObject(GENIE_OBJ_GAUGE, 0x00, map(abs(int(kwValue*1000)), 0, 20000, 0, 99));

  // Write to the LED digits what the speed is
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0x00, floor(speedKph));    

}

// Read the serial feed from the GPS... often
void consumeGPS(void)
{
  while (Serial1.available() > 0)
  {
    gps.encode(Serial1.read());
  }
  // get the GPVTG speed value
  if (gpsSpeed.isValid()) {
    speedKph = (float) gps.parseDecimal(gpsSpeed.value())/100;
  }
  if (gps.satellites.isValid()) {
    gpsSats = (int) gps.satellites.value();
  }

}

//Read the curent and voltaga measurements
void readVehicle(void)
{
  ampValue = map(analogRead(ampPin), 0, 1023, -272, 300);
  voltValue = map(analogRead(voltPin), 0, 1023, 0, 99);
  kwValue = ampValue * voltValue / 1000;
  // also read G-Meter
}



/////////////////////////////////////////////////////////////////////
//
// This is the user's event handler. It is called by genieDoEvents()
// when the following conditions are true
//
//    The link is in an IDLE state, and
//    There is an event to handle
//
// The event can be either a REPORT_EVENT frame sent asynchronously
// from the display or a REPORT_OBJ frame sent by the display in
// response to a READ_OBJ request.
//
// LONG HAND VERSION, MAYBE MORE VISIBLE AND MORE LIKE VERSION 1 OF THE LIBRARY
void myGenieEventHandler(void)
{
  genieFrame Event;
  genie.DequeueEvent(&Event);

  int slider_val = 0;

  //If the cmd received is from a Reported Event (Events triggered from the Events tab of Workshop4 objects)
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER)                // If the Reported Message was from a Slider
    {
      if (Event.reportObject.index == 0)                              // If Slider0
      {
      }
    }
  }

  //This can be expanded as more objects are added that need to be captured

  //Event.reportObject.cmd is used to determine the command of that event, such as an reported event
  //Event.reportObject.object is used to determine the object type, such as a Slider
  //Event.reportObject.index is used to determine the index of the object, such as Slider0
  //genie.GetEventData(&Event) us used to save the data from the Event, into a variable.
}



void displayInfo()
{
  if (!gps.location.isValid() || !gps.location.isUpdated())
  {
    //Serial.println(gps.failedChecksum());
    return;
  }
  
  Serial.print(speedKph);
  Serial.println(F(" Kph"));
  Serial.print(voltValue);
  Serial.println(F(" V"));
  Serial.print(kwValue);
  Serial.println(F(" Kw"));
  Serial.println();

}




