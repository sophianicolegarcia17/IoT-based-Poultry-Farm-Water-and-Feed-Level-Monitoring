#include <WiFi.h>
#include <WiFiClient.h>
#include "time.h"
#include <BlynkSimpleEsp32.h>
#include <Servo_ESP32.h>


#define BLYNK_TEMPLATE_ID "??????????????"
#define BLYNK_TEMPLATE_NAME "??????????????"
#define BLYNK_AUTH_TOKEN "??????????????"

#define WIFI_SSID "????????"        // WiFi SSID
#define WIFI_PASS "????????"   // WiFi password

//ultrasonic1
#define TRIGPIN1 18
#define ECHOPIN1 19

//ultrasonic2
#define TRIGPIN2 23
#define ECHOPIN2 22

#define FULL_DISTANCE_CM 26 // full distance of water and feed storage in cm (SIZE NG CONTAINER NATIN IN CM)

#define VPIN_BUTTON_13    V13 //servo
#define VPIN_TIME_LABEL   V14 //Time Label Display

static const int servoPin = 14; // printed G14 on the board
Servo_ESP32 servo1;

bool toggleState_1 = LOW; //Define integer to remember the toggle state for servo

BlynkTimer timer;

int hour;
int minute;

long duration1, distance1, duration2, distance2;

BLYNK_WRITE(V0){  // Blynk virtual pin to receive the number of days input
  days = param.asInt();  // Get the number of days from Blynk app
}

//servo
BLYNK_WRITE(VPIN_BUTTON_13) {
  toggleState_1 = param.asInt();
  if(toggleState_1 == 1){
    servo1.write(180);
    delay(3000);
    servo1.write(0);
  }
  else { 
    servo1.write(0);
  }
} 

void setTimezone(String timezone){
  Serial.printf("Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  // Clock settings are adjusted to show the new local time
  tzset();
}
void initTime(String timezone){
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // Connect to NTP server
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println("Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  timeinfo.tm_hour += 8;
  mktime(&timeinfo); // Normalize the time structure
  
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  // Update the LCD widget with the current hour and minute
  char time_str[16];
  sprintf(time_str, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  time_t now;
  char timeString[20];
  
  time(&now);
  strftime(timeString, sizeof(timeString), "%B %d %Y %H:%M:%S", &timeinfo);
  Blynk.virtualWrite(V14, String(timeString));
  
  hour = atoi(&timeString[14]);
  minute = atoi(&timeString[17]);

  Serial.print("Hour: ");
  Serial.println(hour);
  Serial.print("Minute: ");
  Serial.println(minute);

}

void setup() {
  Serial.begin(9600);
  servo1.attach(servoPin);

  initTime("Asia/Manila"); // Set timezone to Asia/Manila

  pinMode(TRIGPIN1, OUTPUT);
  pinMode(ECHOPIN1, INPUT);
  pinMode(TRIGPIN2, OUTPUT);
  pinMode(ECHOPIN2, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  
  Blynk.virtualWrite(VPIN_BUTTON_13, toggleState_1);

}

void loop() {
  printLocalTime();
  Blynk.run();
  timer.run();

  digitalWrite(TRIGPIN1, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIGPIN1, HIGH);
  delayMicroseconds(12);

  digitalWrite(TRIGPIN1, LOW);
  duration1 = pulseIn(ECHOPIN1, HIGH);
  distance1 = (duration1 / 2) / 29.1;

  digitalWrite(TRIGPIN2, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIGPIN2, HIGH);
  delayMicroseconds(12);

  digitalWrite(TRIGPIN2, LOW);
  duration2 = pulseIn(ECHOPIN2, HIGH);
  distance2 = (duration2 / 2) / 29.1;

  // calculate the distance as a percentage of the full distance
  int percentage1 = ((FULL_DISTANCE_CM - distance1) * 100) / FULL_DISTANCE_CM;
  int percentage2 = ((FULL_DISTANCE_CM - distance2) * 100) / FULL_DISTANCE_CM;

  Serial.print("Distance 1: ");
  Serial.print(distance1);
  Serial.print("cm, Percentage 1: ");
  Serial.print(percentage1);
  Serial.println("%");

  Serial.print("Distance 2: ");
  Serial.print(distance2);
  Serial.print("cm, Percentage 2: ");
  Serial.print(percentage2);
  Serial.println("%");

//Display values on blynk
  Blynk.virtualWrite(V15, percentage1);
  Blynk.virtualWrite(V16, percentage2);

  // Check if it's 8am, 12:30pm, or 7:30pm to rotate servo
  if (hour == 8 && minute == 0) {
      servo1.write(180);
      delay(3000);
      servo1.write(0);
  } else if (hour == 12 && minute == 30) {
      servo1.write(180);
      delay(3000);
      servo1.write(0);
  } else if (hour == 7 && minute == 30) {
      servo1.write(180);
      delay(3000);
      servo1.write(0);
  } 
    else { 
    servo1.write(0);
  }
 
}
