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
//----firebase--------

#define API_KEY "WFlGeZDmqtQqeEkFlcSXPtIVt4KPt8BtNH2KyBn7"
#define DATABASE_URL "https://esp32-7691f-default-rtdb.firebaseio.com/"

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

uint8_t caculator_sum (String *Data)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh );
  int sum = 0;  
  for(int i =0; i<lengh ; i++)
  {
    sum += _data[i];
  }
  return (~sum + 2);
}

uint8_t get_checksum (String *Data)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh );
  int sum = 0;  
  for(int i =0; i<lengh-1 ; i++)
  {
    sum += _data[i];
  }
  return (~sum + 2);
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

void setup() {
    Serial.begin(115200);
    
   // smartconfigwifi();
   // setupfirebase();

}

void loop() {
     
   if (E32.available()) {
    String Data_From_Lora = E32.readString();
    Serial.println(Data_From_Lora);
      if(check_lora_add(&Data_From_Lora) == true)
      {
          uint8_t ID_node = Get_node_ID(&Data_From_Lora);
          uint8_t humi,temp;
          Get_data(&Data_From_Lora,&humi,&temp);
//          Serial.println("ID NODE: %i \n",ID_node);
//          Serial.println("humi: %i \n",humi);
//          Serial.println("temp: %i \n",temp);
         // E32.println(0x01); //return 1 byte status oke 

         /*sent data to firebase*/
      }
    
  }
}
