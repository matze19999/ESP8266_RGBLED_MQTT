#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

const char* ssid     = "";
const char* password = "";
const char* deviceName = "ANAVI";
const char* mqtt_server = "";
const int mqttPort = 1883;

const int pinAlarm = 16;
const int pinButton = 0;
const int pinLedRed = 12;
const int pinLedGreen = 14;
const int pinLedBlue = 13;

// rainbow effect
int r = 0;
int g = 0;
int b = 0;

int buttonState = 0;
int program = 0;
int brightness = 1023;
int rainbowspeed = 25;
String ioBrokerColor = "null";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ANAVI");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ArduinoOTA.setHostname("ANAVI");
  ArduinoOTA.begin();

  Serial.begin(115200);

  pinMode(pinAlarm, OUTPUT);
  pinMode(pinLedRed, OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedBlue, OUTPUT);
  pinMode(pinButton, INPUT);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void dark() {
  analogWrite(pinLedRed, 0);
  analogWrite(pinLedGreen, 0);
  analogWrite(pinLedBlue, 0);
  client.publish("ANAVI/color", "dark");
  Serial.println("Dark wird gestartet");
}

void white() {
  analogWrite(pinLedRed, brightness);
  analogWrite(pinLedGreen, brightness);
  analogWrite(pinLedBlue, brightness);
  client.publish("ANAVI/color", "white");
  Serial.println("White wird gestartet");
}

void red() {
  analogWrite(pinLedRed, brightness);
  analogWrite(pinLedGreen, 0);
  analogWrite(pinLedBlue, 0);
  client.publish("ANAVI/color", "red");
  Serial.println("Red wird gestartet");
}

void green() {
  analogWrite(pinLedRed, 0);
  analogWrite(pinLedGreen, brightness);
  analogWrite(pinLedBlue, 0);
  client.publish("ANAVI/color", "green");
  Serial.println("Green wird gestartet");
}

void blue() {
  analogWrite(pinLedRed, 0);
  analogWrite(pinLedGreen, 0);
  analogWrite(pinLedBlue, brightness);
  client.publish("ANAVI/color", "blue");
  Serial.println("Blue wird gestartet");
}

void yellow() {
  analogWrite(pinLedRed, brightness);
  analogWrite(pinLedGreen, brightness);
  analogWrite(pinLedBlue, 0);
  client.publish("ANAVI/color", "blue");
  Serial.println("Yellow wird gestartet");
}

void purple() {
  analogWrite(pinLedRed, brightness);
  analogWrite(pinLedGreen, 0);
  analogWrite(pinLedBlue, brightness);
  client.publish("ANAVI/color", "blue");
  Serial.println("Purple wird gestartet");
}

void aqua() {
  analogWrite(pinLedRed, 0);
  analogWrite(pinLedGreen, brightness);
  analogWrite(pinLedBlue, brightness);
  client.publish("ANAVI/color", "blue");
  Serial.println("Purple wird gestartet");
}

void rainbow() { 
    Serial.println("Rainbow wird gestartet");
    while (ioBrokerColor == "rainbow") {
        setColor(255, 0, 0);    // red
        setColor(0, 255, 0);    // green
        setColor(0, 0, 255);    // blue
        setColor(255, 255, 0);  // yellow
        setColor(80, 0, 80);    // purple
        setColor(0, 255, 255);  // aqua
        loop();
    }
}

void setColor(int red, int green, int blue) {
  while ( r != red || g != green || b != blue ) {
    if ( r < red ) r += 1;
    if ( r > red ) r -= 1;

    if ( g < green ) g += 1;
    if ( g > green ) g -= 1;

    if ( b < blue ) b += 1;
    if ( b > blue ) b -= 1;

    _setColor();
    delay(rainbowspeed);
  }
}

void _setColor() {
  analogWrite(pinLedRed, r);
  analogWrite(pinLedGreen, g);
  analogWrite(pinLedBlue, b); 
}


void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';
  String strTopic = String(topic);
  String strPayload = String((char * ) payload);

  if (strTopic == "ANAVI/color") {
    ioBrokerColor = strPayload;
    if (strPayload == "dark") dark();
    if (strPayload == "white") white();
    if (strPayload == "red") red();
    if (strPayload == "green") green();
    if (strPayload == "blue") blue();
    if (strPayload == "yellow") yellow();
    if (strPayload == "purple") purple();
    if (strPayload == "aqua") aqua();
    if (strPayload == "rainbow") rainbow();
  }
  if (strTopic == "ANAVI/brightness") {
    brightness = strPayload.toInt();
    if (ioBrokerColor == "white") white();
    if (ioBrokerColor == "red") red();
    if (ioBrokerColor == "green") green();
    if (ioBrokerColor == "blue") blue();
    if (ioBrokerColor == "yellow") yellow();
    if (ioBrokerColor == "purple") purple();
    if (ioBrokerColor == "aqua") aqua();
  }
  if (strTopic == "ANAVI/rainbowspeed") {
    rainbowspeed = abs(strPayload.toInt());
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // SUBSCRIBE TOPIC HERE
      client.subscribe("ANAVI/color");
      client.subscribe("ANAVI/brightness");
      client.subscribe("ANAVI/rainbowspeed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  ArduinoOTA.handle();

  buttonState = digitalRead(pinButton);

  if (buttonState == LOW) {
    delay(250);
    Serial.println("Button gedrückt!");
    program++;
    
    Serial.println((String)"Program is " + program);
    digitalWrite(pinAlarm, HIGH);

    if (program == 0) {
      dark();
    }
    else if (program == 1) {
      white();
    }
    else if (program == 2) {
      red();
    }
    else if (program == 3) {
      green();
    }
    else if (program == 4) {
      blue();
    }
    else if (program == 5) {
      yellow();
    }
    else if (program == 6) {
      purple();
    }
    else if (program == 7) {
      aqua();
    }
    else if (program == 8) {
        rainbow();
    }
    else if (program == 9) {
      program = -1;
    }

  }
  else if (buttonState == HIGH) {
    digitalWrite(pinAlarm, LOW);
  }
}
