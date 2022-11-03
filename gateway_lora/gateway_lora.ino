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
#include <HardwareSerial.h>
#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
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
#define M0 0
#define M1 4
#define LED 2
#define OUT1 12
#define OUT2 14
#define OUT3 27
#define OUT4 24

HardwareSerial E32(2);
SemaphoreHandle_t  xMutex;
typedef enum 
{
  mode0 = 0,
  mode1 = 1,
  mode2 = 2,
  mode3 = 3,
} E32_mode;

void disable_wdt()
{
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    disableCore0WDT();
    disableCore1WDT();
    disableLoopWDT();
}

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
/*Setup for control device*/
void control_setup(void)
{
  pinMode(OUT1,OUTPUT);
  pinMode(OUT2,OUTPUT);
  pinMode(OUT3,OUTPUT);
  pinMode(OUT4,OUTPUT);
  pinMode(LED,OUTPUT);
  /*state init*/
  digitalWrite(OUT1,HIGH);
  digitalWrite(OUT2,HIGH);
  digitalWrite(OUT3,HIGH);
  digitalWrite(OUT4,HIGH);
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

/*Task*/
int f;
void Task1( void * parameter) {
  while(1) {
     
        xSemaphoreTake(xMutex, portMAX_DELAY);  //take mutex
        Serial.println("task1");   
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/temp",10);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/humi",10);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/light",10);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/air",10);
  
        Firebase.setInt(fbdo,"/Node 1/threshold/temp",10);
        Firebase.setInt(fbdo,"/Node 1/threshold/humi",10);
        Firebase.setInt(fbdo,"/Node 1/threshold/light",10);
        Firebase.setInt(fbdo,"/Node 1/threshold/air",10);
  
        Firebase.setBool(fbdo,"/Node 1/autocontrol",false);
        Firebase.setBool(fbdo,"/Node 1/control/motor",true);
        Firebase.setBool(fbdo,"/Node 1/control/fan",true);
        Firebase.setBool(fbdo,"/Node 1/control/lamp",true);
        xSemaphoreGive(xMutex); // release mutex
      vTaskDelay(1000); 
  }
}

void Task2( void * parameter) {
  while(1) {
        
        
        xSemaphoreTake(xMutex, portMAX_DELAY);  //take mutex
        Serial.println("task2");
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/temp",30);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/humi",30);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/light",30);
        Firebase.setInt(fbdo,"/Node 1/DataFromNode/air",30);
  
        Firebase.setInt(fbdo,"/Node 1/threshold/temp",30);
        Firebase.setInt(fbdo,"/Node 1/threshold/humi",30);
        Firebase.setInt(fbdo,"/Node 1/threshold/light",30);
        Firebase.setInt(fbdo,"/Node 1/threshold/air",30);
  
        Firebase.setBool(fbdo,"/Node 1/autocontrol",true);
        Firebase.setBool(fbdo,"/Node 1/control/motor",false);
        Firebase.setBool(fbdo,"/Node 1/control/fan",false);
        Firebase.setBool(fbdo,"/Node 1/control/lamp",false);
        xSemaphoreGive(xMutex); // release mutex
        
      vTaskDelay(1000); 
  }
}




void setup() {
    Serial.begin(115200);
    disable_wdt();
    control_setup();
    digitalWrite(LED,LOW);
    setuplora();
    Lora_SetMode(mode0);
    smartconfigwifi();
    setupfirebase();
    digitalWrite(LED,HIGH);
    /*create task handle*/
    xMutex = xSemaphoreCreateMutex();
    xTaskCreate(Task1,"Task1",10000,NULL,1,NULL);
    xTaskCreate(Task2,"Task2",10000,NULL,1,NULL);
    f=0;
      

}


void loop() {
//    if (E32.available()) 
//      {
//        String Data_From_Lora = E32.readString();
//        Serial.println(Data_From_Lora);
//    
//          if(check_lora_add(&Data_From_Lora) == true)
//          {
//              //Serial.println("oke em !");
//              uint8_t ID_node = Get_node_ID(&Data_From_Lora);
//              uint8_t humi,temp;
//              Get_data(&Data_From_Lora,&humi,&temp);
//              Serial.println("du lieu tu Node");
//              Serial.println(ID_node);
//             Serial.println("gia tri do am dat: ");
//             Serial.println(humi);
//             // E32.println(0x01); //return 1 byte status oke 
//    
//             /*sent data to firebase*/
//            
//    
//         }
//      }
   
}
