/* Author: Nachiket Dalvi

    This is a code for an automatic watering system that waters plants when the soil moisture.
    In addition to this it also stores the moisture data and ph levels of the soil on a sd card.
    Components used:
    1: Soil moisture sensor
    2: Ph Sensor
    3: SPI sd card reader 
    4: RTC(PCF8563)
    5: 12 V relay for Pump
*/


#include<AltSoftSerial.h>
#include<SPI.h>
#include<SD.h>
#include<Wire.h>
#include<Rtc_Pcf8563.h>
AltSoftSerial debug;
Rtc_Pcf8563 rtc;
#define MOISTURE A0
#define PH A1
#define RELAY 3
#define BUTTON1 4
#define BUTTON2 5
#define LED1 6
#define LED2 7


struct data{
float moisture,pH;
int cal_low, cal_high,t;  
}d;

char packet[70];
int buf[10];
int flag,temp,packetno;
File myfile;

void setup()
{

  debug.begin(9600);
  //pinMode(A0,INPUT);
  //pinMode(A1,INPUT);
  pinMode(BUTTON1,OUTPUT);
  pinMode(BUTTON2,INPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);

  debug.print("Initializing SD card...");
  if (!SD.begin(10)) 
  {
    debug.println("initialization failed!");
  }
}

void loop()
{
  flag = 0;
  calibration();
  moisture_value();
  getph();
  packetform();
  store_data();
  pump();
}



void calibration()
{
  while(digitalRead(BUTTON1 == HIGH))
  {
    if(flag == 0)
    {
      d.cal_low = analogRead(MOISTURE);
      delay(100);
      digitalWrite(LED1,HIGH);
      flag = 1;
    }
    
    if(digitalRead(BUTTON2 == HIGH))
    {
      d.cal_high = analogRead(MOISTURE);
      delay(100);
      digitalWrite(LED2,HIGH);
      break;
    }
    
  }
  delay(100);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
}


void moisture_value()
{
  d.t = analogRead(MOISTURE);
  d.moisture = map(d.t, d.cal_low, d.cal_high, 100, 0);
  delay(100);
}


void getph()
{
  int i,j;
  float avg;
  for(i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
    { 
      buf[i]=analogRead(PH);
      delay(10);
    }
    for(i=0;i<9;i++)        //sort the analog from small to large
    {
      for(int j=i+1;j<10;j++)
      {
          if(buf[i]>buf[j])
          {
            temp=buf[i];
            buf[i]=buf[j];
            buf[j]=temp;
          } 
      }
    }
    avg = 0;
    for(i=2;i<8;i++)
    {
      avg = avg + buf[i];
    }
    avg = avg/6.0;
    d.pH = avg*3.3*3.5/1024;
    delay(100);

 
}

void packetform()
{
  int i;
  char convert[6];
  strncat(packet,rtc.getDay(),2);
  strncat(packet,"/",1);
  strncat(packet,rtc.getMonth(),2);
  strncat(packet,"/",1);
  strncat(packet,rtc.getYear(),4);
  strncat(packet,",",1);
  strncat(packet,rtc.getHour(),2);
  strncat(packet,":",1);
  strncat(packet,rtc.getMinute(),2);
  strncat(packet,",",1);
  dtostrf(d.moisture,3,2,convert);
  strncat(packet,convert,sizeof(convert));
  for(i=0;i<sizeof(d.moisture);i++)
  {
    convert[i]= NULL;
  }
  strncat(packet,",",1);
  dtostrf(d.pH,3,1,convert);
  strncat(packet,convert,sizeof(convert));
  for(i=0;i<sizeof(d.pH);i++)
  {
    convert[i]= NULL;
  }
  debug.println("the packet: ");
  debug.println(packet);

}


void store_data()
{
  myfile = SD.open("data.csv", FILE_WRITE);
  if(myfile)
  {
    debug.println("Writing to SD card");
    myfile.println(packet);
  }

}

void pump()
{
  if(d.moisture<10)
  {
    digitalWrite(RELAY,HIGH);
  }
  if(d.moisture>80)
  {
    digitalWrite(RELAY,LOW);
  }
}
