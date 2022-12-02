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

//#define API_KEY "WFlGeZDmqtQqeEkFlcSXPtIVt4KPt8BtNH2KyBn7"
//#define DATABASE_URL "https://esp32-7691f-default-rtdb.firebaseio.com/"


#define API_KEY "igl4qYVDh02KscaKgZIZ7hdHGUeMT5oXFuwR55bG"
#define DATABASE_URL "https://smarthome-6495e-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;


//-----Lorawan------
/**
 * format data: lora_add | ID_node | data_temp | data_humi | data_light  checksum
 */
#define LORA_ADD  0x11

//UART ESP32 - LORA
#define TXLORA 17
#define RXLORA 16
#define M0 0
#define M1 4
#define LED 2
#define OUT1 14
#define OUT2 27
#define OUT3 26

HardwareSerial E32(2);
SemaphoreHandle_t  xMutex;

static uint8_t humi,light,temp;

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
  pinMode(LED,OUTPUT);
  /*state init*/
  digitalWrite(OUT1,HIGH);
  digitalWrite(OUT2,HIGH);
  digitalWrite(OUT3,HIGH);
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

void Get_data(String *Data, uint8_t *humi_var, uint8_t *temp_var, uint8_t *light_var)
{
  int lengh = Data->length() + 1;
  char _data[lengh];
  Data->toCharArray(_data,lengh ); 
  *temp_var = (uint8_t)_data[2];
  *humi_var = (uint8_t)_data[3];
  *light_var = (uint8_t)_data[4];
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
void Task1( void * parameter) {
  
  while(1) {
        /*sent data to firebase*/
            
        if (E32.available()) 
        {
          String Data_From_Lora = E32.readString();
          //Serial.println(Data_From_Lora);
          if(check_lora_add(&Data_From_Lora) == true)
          {
              //Serial.println("oke em !");
              uint8_t ID_node = Get_node_ID(&Data_From_Lora);
              
              Get_data(&Data_From_Lora,&humi,&temp,&light);
              Serial.println("du lieu tu Node");
              Serial.println(ID_node);
              Serial.println("gia tri do am dat: ");
              Serial.println(humi);
              Serial.println("gia tri nhiet do: ");
              Serial.println(temp);
              Serial.println("gia tri anh sang: ");
              Serial.println(light);
             // E32.println(0x01); //return 1 byte status oke 
    
             /*sent data to firebase*/
              xSemaphoreTake(xMutex, portMAX_DELAY);  //take mutex 
              Firebase.setInt(fbdo,"listNode/Node"+String(ID_node)+"/data/temp",temp);
              Firebase.setInt(fbdo,"listNode/Node"+String(ID_node)+"/data/humi",humi);
              Firebase.setInt(fbdo,"listNode/Node"+String(ID_node)+"/data/light",light);
              //Firebase.setInt(fbdo,"/Node"+String(ID_node)+"/DataFromNode/air",10);
              xSemaphoreGive(xMutex); // release mutex 
    
          }
        }

  }
}

void Task2( void * parameter) {
	bool autocontrol;
	bool fan, lamp ,motor;
  bool fan_temp, lamp_temp ,motor_temp;
	int _air,_humi,_light,_temp;
	  while(1) {
	      //Serial.println("task2");
			  xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
        if(Firebase.RTDB.getBool(&fbdo,"listNode/Node1/control/autoControl"))
			  {
          autocontrol = fbdo.boolData(); 
        }
        xSemaphoreGive(xMutex); // release mutex
			  /*check autocontrol*/
			  if(autocontrol == true)
			  {
            xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
            if(Firebase.RTDB.getInt(&fbdo,"listNode/Node1/threshold/humi"))
            {
                _humi = fbdo.intData();
            }
            if(Firebase.RTDB.getInt(&fbdo,"listNode/Node1/threshold/light"))
            {
                _light = fbdo.intData();
            }
            if(Firebase.RTDB.getInt(&fbdo,"listNode/Node1/threshold/temp"))
            {
                _temp = fbdo.intData();
            }
            xSemaphoreGive(xMutex); // release mutex
            /*check the threshold and control device*/
          
            if( temp > _temp)
            {
                Serial.println("Turn on the fan.");
                digitalWrite(OUT1,LOW);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/Node1/control/fan",true);
                xSemaphoreGive(xMutex); // release mutex
            }
            else
            {
                Serial.println("Turn off the fan.");
                digitalWrite(OUT1,HIGH);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/Node1/control/fan",false);
                xSemaphoreGive(xMutex); // release mutex
            }

            if( light < _light)
            {
                Serial.println("Turn on the lamp.");
                digitalWrite(OUT2,LOW);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/node1/control/lamp",true);
                xSemaphoreGive(xMutex); // release mutex
            }
            else
            {
                Serial.println("Turn off the lamp.");
                digitalWrite(OUT2,HIGH);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/Node1/control/lamp",false);
                xSemaphoreGive(xMutex); // release mutex
            }
            
            if( humi < _humi)
            {
                Serial.println("Turn on the motor.");
                digitalWrite(OUT3,LOW);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/Node1/control/motor",true);
                xSemaphoreGive(xMutex); // release mutex
            }
            else
            {
                Serial.println("Turn off the motor.");
                digitalWrite(OUT3,HIGH);
                xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
                Firebase.setBool(fbdo,"listNode/Node1/control/motor",false);
                xSemaphoreGive(xMutex); // release mutex
            }
             
           
			  }
			  else
			  {	 
              xSemaphoreTake(xMutex, portMAX_DELAY); //take mutex
              if(Firebase.RTDB.getBool(&fbdo,"listNode/Node1/control/fan"))
              {
                fan_temp = fbdo.boolData(); 
              }
              if(Firebase.RTDB.getBool(&fbdo,"listNode/Node1/control/lamp"))
              {
                lamp_temp = fbdo.boolData(); 
              }
              if(Firebase.RTDB.getBool(&fbdo,"listNode/Node1/control/motor"))
              {
                motor_temp = fbdo.boolData(); 
              }
              xSemaphoreGive(xMutex); // release mutex
            /*check status from user and control device*/
            if(fan != fan_temp)
            {
                fan = fan_temp;
                if(fan == true)
                {
                  Serial.println("Turn on the fan.");
                  digitalWrite(OUT1,LOW);
                }    
                else 
                {
                  Serial.println("Turn off the fan.");
                  digitalWrite(OUT1,HIGH);
                }               
            }

            if(lamp != lamp_temp)
            {
                lamp = lamp_temp;
                if(lamp == true)
                {
                  Serial.println("Turn on the lamp.");
                  digitalWrite(OUT2,LOW);
                }    
                else 
                {
                  Serial.println("Turn off the lamp.");
                   digitalWrite(OUT2,HIGH);
                }               
            }

            if(motor != motor_temp)
            {
                motor = motor_temp;
                if(motor == true)
                {
                  Serial.println("Turn on the motor.");
                   digitalWrite(OUT3,LOW);
                }    
                else 
                {
                  Serial.println("Turn off the motor.");
                   digitalWrite(OUT3,HIGH);
                }               
            }
			  } 
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
}


void loop() {
    vTaskDelay(1000);
}
