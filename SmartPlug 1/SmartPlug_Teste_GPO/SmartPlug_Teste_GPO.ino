 #include <WiFiManager.h>
#include <ESPping.h>
#include <EEPROM.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


 // Parametros do wifi manager
  WiFiManager wm;
  char output[2] = "5";
  WiFiManagerParameter custom_output("output", "output", output, 2);

 //#define TOKEN   "rFI2SUd8CXjff0Dvdmt7" // Smart Plug 1
 //#define TOKEN   "Nk2HfP2x4N57Qzsgq6kX" // Smart Plug 2
 #define TOKEN   "czId22LdbdFQ0DvjvR4q" // Smart Plug 3
 //#define TOKEN   "XxcuXTmlIWrQf6ZDPHUg" // Smart Plug 4
 //#define TOKEN   "tzJ10UlgN4thTUIFVZpg" // Smart Plug 5

 #define THINGSBOARD_SERVER "thingsboard.eletromidia.com.br"
 
 //#define TOKEN   "l4qI7NT0dvxvGJqFTSSO"  // 1  Substitua pelo seu token ThingsBoard 
 //#define THINGSBOARD_SERVER "thingsboard.cloud"


// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

#define RLY_1  14              
#define RLY_2      
#define RLY_3        
#define Led    13

const int MAX_PING_RETRIES = 5;
const int MAX_RECONNECT_RETRIES = 3;
const int RECONNECT_DELAY = 3;
const int RETRY_DELAY = 120000; // 2 Minutos tempo que o Roteador fica desligado
const int WAITING_DELAY = 15000; // 30 Segundos
const int THINGSBOARD_DELAY = 300000; // A cada 5 Minutos enviar dados para o ThingsBoard 2 = 120000
const int PISCA_DELAY = 300; // 

unsigned long millisTarefa1;
unsigned long millisTarefa2;
unsigned long millisTarefa3;
unsigned long millisPisca ;

int addressPingNet = 0;
int addressPingRot = 1;
int addressROT     = 2;
int addressESP     = 3;
int addressRES     = 4;
int addressENE     = 5;

int valuePingNet;
int PingNet;
int valuePingRot;
int valueROT;
int valueRES;
int valueESP;
int valueENE;

int attempt = 0;

bool Send = false;
bool ENERGIA = false;

void setup(){
  
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(100);
   
  pinMode(RLY_1, OUTPUT);// Define o Rele como saída
  pinMode(Led, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(RLY_1, HIGH);// Liga o rele do roteador
  digitalWrite(Led, HIGH);
  delay(1000);
  digitalWrite(Led, LOW);
  delay(1000);

  //LimpaEEPROM();

  valuePingNet = EEPROM.read(addressPingNet);
  valuePingRot = EEPROM.read(addressPingRot);
  valueROT = EEPROM.read(addressROT);
  valueESP = EEPROM.read(addressESP);
  valueRES = EEPROM.read(addressRES);
  valueENE = EEPROM.read(addressENE);

  delay(100);
  
  Serial.print("Ping Net  - ");Serial.print(valuePingNet , DEC);  Serial.println();
  Serial.print("Ping Rot  - ");Serial.print(valuePingRot , DEC);  Serial.println();
  Serial.print("Reset ROT - ");Serial.print(valueROT , DEC);  Serial.println();
  Serial.print("Reset ESP - ");Serial.print(valueESP , DEC);Serial.print(" / ");Serial.print(valueRES, DEC);  Serial.println();
  Serial.print("Energia - ");Serial.print(valueENE , DEC);  Serial.println();
 
  //wm.resetSettings();
  WiFiManager wm;

  wm.addParameter(&custom_output);
  wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar

  //Constrói o nome da rede com o endereço MAC
  String ssid = "SmartPlug 3 - ";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for(int i = 0; i < 6; ++i){
    if(i > 0) ssid += ".";
    ssid += String(mac[i],HEX);
  }
  
  //Abertura do wifi manager
 // bool connect_to_me = wm.autoConnect(ssid.c_str(), "3l3tr0m1d1@");
  
  //if (!connect_to_me) {
  if(!wm.autoConnect(ssid.c_str(), "3l3tr0m1d1@")){
    // Resetar ou colocar em modo de espera profundo (deep sleep)
    valueESP++;
    EEPROM.write(addressESP,valueESP);
    EEPROM.commit();
    Serial.println("Erro connect_to_me");
    return;
  }
     
  delay(500);
  attempt = 0;
   while(!tb.connected() && attempt < MAX_PING_RETRIES){
    // Reconectar apenas se a conexão com o ThingsBoard foi perdida
    Serial.println("Reconnecting to ThingsBoard...");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
       attempt++;
      Serial.print("Failed to reconnect = ");
      Serial.println(attempt);
      //return;
    }
    else{
         Serial.println("Conectado ao ThingsBoard.");
         //--- Dados fixos que vão para o thingsboard
         tb.sendAttributeString("LocalIP", WiFi.localIP().toString().c_str());
         tb.sendAttributeString("MacAddress", WiFi.macAddress().c_str());
         tb.sendAttributeString("SSID", WiFi.SSID().c_str());
         tb.sendAttributeString("Smart Plug ", "3"); // Renomear com  numero de
         delay(50);
         break; 
    }
  }
  
  millisTarefa2 = THINGSBOARD_DELAY+1000;
  Serial.println("");
  Serial.print("***  VOID SETUP ***");
  Serial.println("");
  
}

void loop(){
  Serial.println("VOID LOOP");
  WiFiManager wm;
/*
  //Serial.println("");
  //Serial.print("Enviar ThingsBoard = ");
  //Serial.print((millis() - millisTarefa2));
   attempt = 0;
   while(!tb.connected() && (attempt < MAX_PING_RETRIES)){
      Serial.println("");
      // Reconectar apenas se a conexão com o ThingsBoard foi perdida
      Serial.println("Reconnecting to ThingsBoard...");
      if(!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
         attempt++;
         Serial.print("Failed to reconnect = ");
         Serial.println(attempt);
         delay(100);
         //return;
      }
      else{
        Serial.println("Conectado ao ThingsBoard.");
        EnviarThingsBoard(); //Enviar para o ThingsBoard
        //tb.loop();
        break;
      }
   }
   

  // Ping local IP
  Serial.println("Ping local IP");
  attempt = 0;
  millisTarefa1 = millis();
  while(attempt < MAX_PING_RETRIES){
        if((millis() - millisTarefa1) < WAITING_DELAY){
            Serial.print(".");
            digitalWrite(Led, HIGH); 
         }
        if((millis() - millisTarefa1) > WAITING_DELAY){ // cada 30 segundos da um ping no roteados
           if(Ping.ping(WiFi.gatewayIP()) > 0){
              Serial.println("");
              Serial.println("Ping no Roteador OK");
              digitalWrite(Led, LOW);
              delay(100);
              break;
           }
           else{
                Serial.println("");
                Serial.println("Ping no Roteador Error !");
                digitalWrite(Led, LOW);
                attempt++;//Serial.print("attempt = ");Serial.println(attempt);
                valuePingRot++;//Serial.print("valuePingRot = ");Serial.println(valuePingRot);
                delay(50);
                EEPROM.write(addressPingRot,valuePingRot);
                EEPROM.commit();
                delay(50);
                if(attempt >= MAX_PING_RETRIES){
                   Serial.println("desliga_();");
                   //desliga_();
                   break; 
                }
                millisTarefa1 = millis();
           }    
        }
  }

*/
   attempt = 0;
   millisTarefa1 = millis();
   
   // Ping Host
   Serial.println("Ping Host");
   while(attempt < MAX_PING_RETRIES){
        if((millis() - millisTarefa1) < WAITING_DELAY){
           Serial.print(".");
           digitalWrite(Led, HIGH);
        }
        if((millis() - millisTarefa1) > WAITING_DELAY){
           if(Ping.ping("www.google.com") > 0){
              Serial.println("");
              Serial.println("Ping na Internet OK");
              digitalWrite(Led, LOW);
              delay(100);
              break;
           }
           else{
                Serial.println("");
                Serial.println("Ping na Internet Error ");
                digitalWrite(Led, LOW);
                attempt++;//Serial.print("attempt = ");Serial.println(attempt);
                valuePingNet++;//Serial.print("valuePingNet = ");Serial.println(valuePingNet);
                delay(50);
                EEPROM.write(addressPingNet,valuePingNet);
                EEPROM.commit();
                delay(50);
                millisTarefa1 = millis();
                if(attempt > MAX_PING_RETRIES){
                   digitalWrite(Led, LOW);
                   valueESP++;
                   valueRES++;
                   delay(50);
                   EEPROM.write(addressRES,valueRES);
                   EEPROM.commit();
                   EEPROM.write(addressESP,valueESP);
                   EEPROM.commit();
                   delay(50);
                   ESP.restart();
                   //ESP.reset();
                }
                Serial.println("");
                Serial.print("valueRES =");Serial.println(valueRES);
                Serial.println("");
                if(valueRES >= MAX_RECONNECT_RETRIES){
                    Serial.println("PingNet desliga_();");
                    valueRES = 0;
                    EEPROM.write(addressRES,valueRES);
                    EEPROM.commit();
                   //desliga_();
                   break; 
                }
           }
           
        }
   }

   
}

void EnviarThingsBoard(){

  if((millis() - millisTarefa2) > THINGSBOARD_DELAY){
      Serial.println("");
      Serial.println("Enviar ThingsBoard");
      tb.sendTelemetryInt("Ping Net",valuePingNet);
      tb.sendTelemetryInt("Ping Rot",valuePingRot);
      tb.sendTelemetryInt("Reset ROT",valueROT);
      tb.sendTelemetryInt("Reset ESP",valueESP);
      //millisTarefa2 = millis();
      delay(50);
      //LimpaEEPROM();
  }
  if((millis() - millisTarefa2) > THINGSBOARD_DELAY+3000){
      Serial.println("");
      Serial.println("Limpa EEPROM");
      tb.sendTelemetryInt("Ping Net",valuePingNet);
      tb.sendTelemetryInt("Ping Rot",valuePingRot);
      tb.sendTelemetryInt("Reset ROT",valueROT);
      tb.sendTelemetryInt("Reset ESP",valueESP);
      delay(50);
      millisTarefa2 = millis();
      Serial.print("millisTarefa2 = ");
      Serial.println(millisTarefa2);
      delay(100);
      LimpaEEPROM();
  }
  
   
      
}

void LimpaEEPROM(){
  valuePingNet = 0;
  valuePingRot = 0;
  valueROT = 0;
  valueESP = 0;
  valueENE = 0;
  valueRES = 0;
  EEPROM.write(addressPingNet, valuePingNet);
  EEPROM.write(addressPingRot, valuePingRot);
  EEPROM.write(addressROT, valueROT);
  EEPROM.write(addressESP, valueESP);
  EEPROM.write(addressENE, valueENE);
  EEPROM.commit();
 
}

void desliga_() {//--- Função que desliga o roteador
  millisTarefa3 = millis();
  millisPisca = millis();
  digitalWrite(RLY_1, LOW);
  Serial.print("Desliga");
  valueENE = 1;
  valueROT++;
  EEPROM.write(addressENE, valueENE);
  EEPROM.write(addressROT,valueROT);
  EEPROM.commit();
  while((millis() - millisTarefa3) < RETRY_DELAY){
      
    if((millis() - millisPisca) < PISCA_DELAY/2){
       digitalWrite(Led, HIGH);
    }
    if((millis() - millisPisca) > PISCA_DELAY/2){
       digitalWrite(Led, LOW);
    }
    if((millis() - millisPisca) > PISCA_DELAY){
      Serial.print("millisTarefa3 = ");
      Serial.println((millis() - millisTarefa3)/1000);
       millisPisca = millis();
    } 
  }
  liga_();
}

void liga_() { //---Função que liga o roteador
  digitalWrite(RLY_1, HIGH);             
  digitalWrite(Led, HIGH);
  Serial.println("Ligar");
   millisTarefa2 = millis();
   valueENE = 1;
}
