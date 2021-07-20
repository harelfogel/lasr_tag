#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/************************* Timer Library *********************************/

#include "Timer.h"

Timer timer;      // create a timer with default settings


/************************* Input Output Pins *********************************/

#define DETECT   2  // pin 2  for  sensor
#define ACTION   9  // pin 9  for action to do buzzer
#define ACTIONv  10 // pin 10 for action to do vibrate

/************************* Global variables **********************************/

const int sensorPin     = DETECT;
const int buzzerPin     = ACTION;
      int count         = 0;
      int score         = count;
      int count_before  = -1;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "KRL"
#define WLAN_PASS       "smj8ywq6kq6s6"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "k6k6k"
#define AIO_KEY         "aio_lqXy43YXq8LfGxeuMEuc1QjlKD90"


/************ Global State (you don't need to change this!) ******************/

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish player = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/player_2");


Adafruit_MQTT_Subscribe player_1_score = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/player_1");
Adafruit_MQTT_Subscribe player_2_score = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/player_2");

/*************************** Sketch Code ************************************/

void MQTT_connect();


#include "Arduino.h"
#include "MaxMatrix.h"

int CS =  12;
int DIN = 13;
int CLK = 14;

int maxInUse = 1;


MaxMatrix m(DIN, CS, CLK, maxInUse);

byte poker[] = {8, 8, 0xff, 0x81, 0xa5, 0xa1, 0xa1, 0xa5, 0x81, 0xff};
byte smile[] = {8, 8, 0xff, 0x81, 0xb5, 0xa1, 0xa1, 0xb5, 0x81, 0xff};
byte sad[]   = {8, 8, 0xff, 0x81, 0xb5, 0x91, 0x91, 0xb5, 0x81, 0xff};


void get_score()
{
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) 
  {
    if (subscription == &player_1_score) 
    {
      Serial.print(F("player 1 score:  "));
      Serial.println((char *)player_1_score.lastread);
      Serial.println();
    }
    if (subscription == &player_2_score) 
    {
      Serial.print(F("player 2 score:  "));
      Serial.println((char *)player_2_score.lastread);
      Serial.println();
    }

    int p1 = atoi((char *)player_1_score.lastread), p2 = atoi((char *)player_2_score.lastread);
    
    if(p1>p2)
    {
      Serial.println("Player 1 WON  !!!");
      m.writeSprite(0, 0, smile);
      delay(1000);
    }
    else if(p2>p1)
    {
      Serial.println("Player 2 WON  !!!");
      m.writeSprite(0, 0, sad);
      delay(1000);
    }
    else
    {
      Serial.println("It's a tie :("); 
      m.writeSprite(0, 0, poker);
      delay(1000);
    }
  }

  for (int i = 0; i < 8; i++) {
    m.shiftLeft(false, false);
    delay(300);
  }
  m.clear();
}

void send_score() 
{
  MQTT_connect();
  //vabration needs to be on here
  
  if(count > 0)
  {
    if (!player.publish(count)) 
    {
      Serial.println(F("Failed"));
    } 
    else 
    {
      Serial.println(F("OK!"));
    }
  }
  else
  {
    if (!player.publish(0)) 
    {
      Serial.println(F("Failed"));
    } 
    else 
    {
      Serial.println(F("OK!"));
    }
  }

  get_score();
}




void setup() 
{
  count     = 0;

  Serial.begin(115200);
  pinMode(DETECT,  INPUT);   //define detect input pin
  pinMode(ACTION,  OUTPUT);  //define ACTION output pin
  pinMode(ACTIONv, OUTPUT);  //define ACTION output pin
  m.init();
  m.setIntensity(8);

  timer.every(180000, send_score);

  //_______________________________________________________________ print conection to wifi status
  
  delay(1000);
  Serial.println(F("Starting..."));
  delay(1000);
  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  Serial.println();


 //_______________________________________________________________ Player 1

  Serial.println();
  Serial.println(); 
  Serial.println("You are Player 1:");
  Serial.println();
  Serial.println(); 

  //_______________________________________________________________ SUBSRCIPTIONS

  
  mqtt.subscribe(&player_1_score);           // Setup MQTT subscription for player_1 feed.
  mqtt.subscribe(&player_2_score);           // Setup MQTT subscription for player_2 feed.
}


void loop() 
{
  int detected   = digitalRead(DETECT);     // read Laser sensor

  if( detected == HIGH)
  {
    digitalWrite(ACTION, HIGH);           // set the buzzer  ON
    digitalWrite(ACTIONv,HIGH);           // set the vibrate ON
    m.writeSprite(0, 0, sad);
    
    ++count;
    delay(300);
    digitalWrite(ACTIONv,LOW);           // set the  vibrate OFF
    
    if(count_before != count)
    {
      count_before = count;
      Serial.print(F("\nSending value "));
      Serial.print(count);
      Serial.print("...");
      Serial.println("Detected!");
    }
  }
  else
  {
    digitalWrite(ACTION, LOW);            // Set the buzzer  OFF
    digitalWrite(ACTIONv,LOW);           // set the  vibrate OFF
    m.clear();
  }
 
  timer.update();

  for (int i = 0; i < 8; i++) {
    m.shiftLeft(false, false);
    delay(300);
  }
  m.clear();
}


void MQTT_connect() {
  int8_t ret;

  
  if (mqtt.connected())                                             // Stop if already connected.
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) 
  {                                                                 // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         while (1);                                                 // basically die and wait for WDT to reset me
       }
  }
  
  Serial.println("MQTT Connected!");
}
