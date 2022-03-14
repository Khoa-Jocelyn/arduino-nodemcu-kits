#include <Wire.h> //Thư viện I2C
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets

//-----------------------------------------------------------------------------------------

WebSocketsClient webSocket;
ESP8266WiFiMulti WiFiMulti;
const char *ssid = "CAY DUA 2";
const char *password = "chinvemot";
const String ip_host = "autodoor.herokuapp.com";
const uint16_t port = 80;
int reqServer = 0;
int temp = 0;
int tempErr = 0;
String resServer = "";

//-----------------------------------------------------------------------------------------

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:

    Serial.printf("[WSc] Disconnected!\n");

    break;

  case WStype_CONNECTED:

    Serial.printf("[WSc] Connected to url: %s\n", payload);

    break;

  case WStype_BIN:

    webSocket.sendTXT("NodeMCU ESP8266 connected!");

    Serial.println((char *)payload);

    if (strcmp((char *)payload, "MANUAL_ON") == 0)
    {
      reqServer = 1;
    }
    else if (strcmp((char *)payload, "MANUAL_OFF") == 0)
    {
      reqServer = -1;
    }
    else if (strcmp((char *)payload, "AUTO_ON") == 0)
    {
      reqServer = 2;
    }
    else if (strcmp((char *)payload, "AUTO_OFF") == 0)
    {
      reqServer = -2;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_ON") == 0)
    {
      reqServer = 3;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_OFF") == 0)
    {
      reqServer = -3;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_CONFIRM") == 0)
    {
      reqServer = 4;
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);

  Wire.begin(D1, D2);

  Serial.println("ESP8266-WebSocketClient");

  WiFi.begin(ssid, password);

  WiFiMulti.addAP("ssid", "passwords");

  Serial.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(500);

    Serial.print(".");
  }

  webSocket.begin(ip_host, port);

  webSocket.onEvent(webSocketEvent);
}

//-----------------------------------------------------------------------------------------

void loop()
{

  webSocket.loop();
  Wire.beginTransmission(8);
  Wire.write(reqServer);
  Wire.endTransmission();
  Wire.requestFrom(8, 13);
  reqServer = 0;

  if (Wire.available())
  {
    int reqArduino = Wire.read();
    if (reqArduino == 1 && temp == 0)
    {
      resServer = "open successfully";
      Serial.println(reqArduino);
      temp = 1;
      tempErr = 1;
    }
    else if (reqArduino == 2 && temp == 1)
    {
      resServer = "close successfully";
      Serial.println(reqArduino);
      temp = 0;
      tempErr = 0;
    }
    else if (reqArduino == 255 && tempErr == 0)
    {
      resServer = "open error";
      Serial.println(reqArduino);
      temp = 1;
      tempErr = 1;
    }
    else if (reqArduino == 254 && tempErr == 1)
    {
      resServer = "close error";
      Serial.println(reqArduino);
      temp = 0;
      tempErr = 0;
    }
  }
  if (resServer != "")
  {

    webSocket.sendTXT(resServer);

    resServer = "";
  }
}

//-----------------------------------------------------------------------------------------