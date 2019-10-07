/*
 * YouTube Subscriber Counter with NodeMCU, 7 segment display and YouTube Sight library
 * http://tools.tastethecode.com/youtube-sight
 * 
 * The YouTube Sight service and library are written by Blagojce Kolicoski
 * for the Taste The Code YouTube Channel
 * https://www.youtube.com/tastethecode
 */

#include "LedControl.h"
#include <YouTubeSight.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// NodeMCU Pin D8 to DIN, D7 to Clk, D6 to CS, no.of devices is 1
LedControl lc = LedControl(D8, D7, D6, 1);

/* Set these to your desired credentials. */
const char *ssid = "WIFI Name";
const char *password = "wifipassword";

#define CHANNEL_GUID "867fb57a-73e4-4020-b455-4fd4959b1cc0" //ENTER YOUR CHANNEL GUID
#define BUTTON_PIN D2

//required for non blocking delay implementation
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
bool first_run = true;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

//delay interval 60 seconds
const long interval = 60000;

WiFiClient http;
YouTubeSight sight(CHANNEL_GUID, http);

void setup()
{
 // Initialize the MAX7219 device
  lc.shutdown(0,false);   // Enable display
  lc.setIntensity(0,10);  // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register

  pinMode(BUTTON_PIN, INPUT);

  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  int digit = 7;
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lc.setChar(0,digit,'.',true);
    digit--;
    if (digit == 0) {
      digit = 7;
    }
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}
void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval || first_run) {
    first_run = false;
    // save the last time you checked
    previousMillis = currentMillis;
    if (sight.getData()) {
      lc.clearDisplay(0);
      Serial.println("views: " + sight.channelStats.views);
      Serial.println("subscribers_gained: " + sight.channelStats.subscribers_gained);
      Serial.println("subscribers_lost: " + sight.channelStats.subscribers_lost);
      Serial.println("subscribers_count: " + sight.channelStats.subscribers_count);
      Serial.println("estimated_minutes_watched: " + sight.channelStats.estimated_minutes_watched);
      Serial.println("average_view_duration: " + sight.channelStats.average_view_duration);
    } else {
      Serial.println("There was an error getting the data.");
    }
  }

  if (sight.channelStats.subscribers_count != "") {
    //Debounce
    int reading = digitalRead(BUTTON_PIN);
    //Serial.println(reading);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
      }
    }
    lastButtonState = reading;
    if (buttonState == HIGH){
      //display count till next milestone
      long count = sight.channelStats.subscribers_count.toInt();
      long nextMilestone = 99999999;
      if (count < 1000) {
        nextMilestone = 1000;
      }else if (count < 10000) {
        nextMilestone = 10000;
      }else if (count < 100000) {
        nextMilestone = 100000;
      }else if (count < 1000000) {
        nextMilestone = 1000000;
      }else if (count < 10000000) {
        nextMilestone = 10000000;
      }

      nextMilestone = -1 * (nextMilestone - count);
      displayString(String(nextMilestone));
    } else {
      //display regular count
      displayString(sight.channelStats.subscribers_count);
    }
  }
}

void displayString(String str)
{
  unsigned int len = str.length();
  int letter = len - 1;
  for (int i = 7; i >= 0; i--) {
    int rev = 7 - i;
    if (letter >= 0 ) {
      lc.setChar(0, rev, str[letter], rev % 3 == 0 && rev != 0 && str[letter] != '-');
      letter--;
    } else {
      lc.setChar(0, rev, ' ', false);
    }
  }
}
