#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Arduino.h"
#include <HardwareSerial.h>
#include <BlynkSimpleEsp32.h>
//----firebase--------

#define API_KEY "WFlGeZDmqtQqeEkFlcSXPtIVt4KPt8BtNH2KyBn7"
#define DATABASE_URL "https://esp32-7691f-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;
//-------Blynk v2-----

#define BLYNK_TEMPLATE_ID "TMPLZ9qSov21"
#define BLYNK_DEVICE_NAME "Lora"


//-----Lorawan------
/**
 * format data: lora_add | ID_node | data_temp | data_humi | checksum
 */
#define LORA_ADD  0x11

//UART ESP32 - LORA
#define TXLORA 17
#define RXLORA 16
#define M0 4
#define M1 0

#define LED 2

HardwareSerial E32(2);

typedef enum 
{
  mode0 = 0,
  mode1 = 1,
  mode2 = 2,
  mode3 = 3,
} E32_mode;

void setuplora()
{
  E32.begin(115200, SERIAL_8N1, RXLORA, TXLORA);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  
}

void Lora_SetMode ( E32_mode _mode)
{  
    //config mode
    switch (_mode)
    {
      case mode0:
      {
          digitalWrite(M0,LOW);
          digitalWrite(M1,LOW);
          break;
      }
      case mode1:
      {
          digitalWrite(M0,HIGH);
          digitalWrite(M1,LOW);
          break;
      }
      case mode2:
      {
          digitalWrite(M0,LOW);
          digitalWrite(M1,HIGH);
          break;
      }
      case mode3:
      {
          digitalWrite(M0,HIGH);
          digitalWrite(M1,HIGH);
          break;
      }
      default:
      {
          break;
      }
    }
    
}

//-----------Smartconfig Wifi setup--------------------------
void smartconfigwifi () {
  WiFi.mode(WIFI_AP_STA);
  /* start SmartConfig */
  WiFi.beginSmartConfig();
 
  /* Wait for SmartConfig packet from mobile */
  Serial.println("Waiting for SmartConfig...");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("SmartConfig done!");
 
  /* Wait for WiFi to connect to AP */
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  
 }

 //---setup firebase------
 void setupfirebase () {

   Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION); 
   Firebase.begin(DATABASE_URL, API_KEY);
   Firebase.reconnectWiFi(true);
   Firebase.setDoubleDigits(5);
  
}

/*--Handler data--------*/

bool check_lora_add(String *Data)
{
    int lengh = Data->length() + 1;
    char _data[lengh];
    Data->toCharArray(_data,lengh );
    if(_data[0] == LORA_ADD)
    {
      return true; 
    }
    else
    {
      return false;
    }
}

uint8_t checksum (String *Data)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh );
  uint8_t sum = 0;  
  for(int i =0; i<lengh ; i++)
  {
    sum += _data[i];
  }
  return (~sum + 2);
}

uint8_t checksum_rxData (String *Data)
{
  int lengh = Data->length();
  Serial.println(lengh);
  char _data[lengh];
  Data->toCharArray(_data,lengh );
  Serial.println("check sum rxdata");
  Serial.println(_data);
  uint8_t sum = 0;  
  for(int i =0; i<lengh ; i++)
  {
    sum +=(uint8_t) _data[i];
  }
  Serial.println("sum = ");
  Serial.println(~sum + 2);
  return (~sum + 2);
}

bool verify_rxdata(String *Data)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh );
  uint8_t sum = checksum_rxData(Data);
  if(sum == _data[lengh])
  {
      return true;  
  }
  else
  {
      return false;  
  }
}


uint8_t Get_node_ID(String *Data)
{
    int lengh = Data->length() + 1;
    char _data[lengh];
    Data->toCharArray(_data,lengh );
    return (uint8_t)_data[1];
} 

void Get_data(String *Data, uint8_t *humi, uint8_t *temp)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh ); 
  *temp = _data[2];
  *humi = _data[3];
}


int hexToDec(String hexString)
{
  int decValue = 0;
  int nextInt;

  for (int i = 0; i < hexString.length(); i++)
  {
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    decValue = (decValue * 16) + nextInt;
  }
  return decValue;
}


void setup() {
    Serial.begin(115200);
    pinMode(2,OUTPUT);
    setuplora();
    Lora_SetMode(mode0);
    smartconfigwifi();
    setupfirebase();
    digitalWrite(2,HIGH);
}

void loop() {
     
   if (E32.available()) {
    String Data_From_Lora = E32.readString();
    Serial.println(Data_From_Lora);
    
   // if(verify_rxdata(&Data_From_Lora)==true)
   // {
   //   Serial.println("check sum oke !");
      if(check_lora_add(&Data_From_Lora) == true)
      {
          //Serial.println("oke em !");
          uint8_t ID_node = Get_node_ID(&Data_From_Lora);
          uint8_t humi,temp;
          Get_data(&Data_From_Lora,&humi,&temp);
          Serial.println("du lieu tu Node");
          Serial.println(ID_node);
         Serial.println("gia tri do am dat: ");
         Serial.println(humi);
         // E32.println(0x01); //return 1 byte status oke 

         /*sent data to firebase*/
         Firebase.setString(fbdo,  "/Node "+String(ID_node)+"/humi" , humi );
         Firebase.setString(fbdo,  "/Node "+String(ID_node)+"/temp" , temp );

         digitalWrite(2,HIGH);
         delay(1000);
         digitalWrite(2,LOW);
         delay(1000);
         digitalWrite(2,HIGH);
      }

    //}
    
  }
}
