// radion volume säädin, versio 1.5
// mukana myös käynnistysledin ohjaus, D2
// 10.3.2018


#include <Arduino.h>
#include <ESP8266WiFi.h>

int sensorPin = A0;
int ledPin = D2;
int sensorValue;
int saadin;
int ero;

void setup() 
{
  int kaynnistys=0,valoisuus=0,suunta=1;
  saadin=0;
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  pinMode(sensorPin,INPUT);
    
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);

  do
  {
    if (Serial.available()>0)
      if (Serial.read()=='k') kaynnistys=1;
    delay(120);
    valoisuus=valoisuus+suunta;
    if (valoisuus==255) suunta=-1;
    if (valoisuus==0) suunta=1;
    analogWrite(ledPin,valoisuus);
  } while (kaynnistys==0);
  analogWrite(ledPin,255);  
}

void loop() 
{
  sensorValue=analogRead(sensorPin);
  if (sensorValue>999) sensorValue=999;
  ero=sensorValue-saadin;
  if (ero<0) ero=-ero;
  if (((saadin<100) && (ero>2)) || ((saadin>99) && (ero>5))) 
  {
    saadin=sensorValue;
    Serial.print('a');
    if (saadin<100) Serial.print('0');
    if (saadin<10) Serial.print('0');    
    Serial.println(saadin);
  }
  delay(100);
}

