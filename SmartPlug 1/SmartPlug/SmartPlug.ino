#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESPping.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ThingsBoard.h>
#include <Ticker.h> 
#include <EEPROM.h>

const int MAX_PING_RETRIES = 3;
const int MAX_RECONNECT_RETRIES = 3;
const int RECONNECT_DELAY = 3;
const int RETRY_DELAY = 30000;
const int WAITING_DELAY = 120000;
int pingRetry;
int RLY_1_State;

 // Parametros do wifi manager
  WiFiManager wm;

#define RLY_1  14              
#define RLY_2      
#define RLY_3        
#define Led  13

Ticker blueticker;   

char output[2] = "5";
WiFiManagerParameter custom_output("output", "output", output, 2);

bool web_ping;
bool local_ping;

//--- Tokens de configuração do thingsboard
#define TOKEN               "NGAd2W9yOwHYRKjRFna6"    //"EUNUTiDAx3L1N9iNNPMp" 
#define THINGSBOARD_SERVER  "thingsboard.cloud"

//--- Variaveis do thingsboard
WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;
bool subscribed = false;

// Inicia variáveis de tempo
unsigned long millisTarefa1 = millis();
unsigned long millisTarefa2 = millis();
unsigned long millisTarefa3 = millis();
unsigned long millisTarefa4 = millis();
unsigned long millisTarefa5 = millis();
unsigned long millisTarefa6 = millis();
unsigned long millisTarefa7 = millis();
unsigned long millisTarefa8 = millis();
unsigned long millisTarefa9 = millis();

 int Roteador = 0;
 int Internet = 0;
 int Reset = 0;

//--- Função reconnet, tenta a conexão novamente com o roteador.
bool reconnect(WiFiManager* wm, bool restart) {
  int timeout;
  int retry = 1;
  
  String ssid = wm->getWiFiSSID();
  String psk = wm->getWiFiPass();

  while (WiFi.status() != WL_CONNECTED) {
    if (retry > MAX_RECONNECT_RETRIES || ssid == "" || psk == "") {
      if (restart) {
        ESP.restart();
        return false;
      }
      return false;
    }
    WiFi.begin(ssid.c_str(), psk.c_str());
    if((millis() - millisTarefa8) < 5000){
          }
            if((millis() - millisTarefa8) > 5000){
            millisTarefa8 = millis();
          }
    if (WiFi.status() != WL_CONNECTED) {
      timeout = pow(RECONNECT_DELAY, retry);
      if((millis() - millisTarefa9) < 5000){
          }
            if((millis() - millisTarefa9) > 5000){
            millisTarefa9 = millis();
          }
      retry++;
    }
  }
  return true;
}

RPC_Response ts1(const RPC_Data &data) {
  char params[10];
  serializeJson(data, params);
  String _params = params;
  if (_params == "true") {
    
    Serial.println("Desligando Rele");
    digitalWrite(RLY_1, LOW);             
    RLY_1_State = LOW;                    
    digitalWrite(Led, LOW);
    delay(180000); 
    Serial.println("Ligando o Rele");
    digitalWrite(RLY_1, HIGH);             
    RLY_1_State = HIGH;                    
    digitalWrite(Led, HIGH);
    delay(180000);

     bool revived = reviveRouter(&wm, 3);
        if (!revived) { 
          if((millis() - millisTarefa2) < 2000){
           }
            if((millis() - millisTarefa2) > 2000){
            millisTarefa2 = millis();
            ESP.restart();
          }    
       }
  }
  // Aqui está faltando o retorno do RPC_Response
  return RPC_Response("result", "success");
}

const size_t callbacks_size = 1;
RPC_Callback callbacks[callbacks_size] = {
  { "getValue_1", ts1 }
};


void setup() {
  
  Serial.begin(115200);
  EEPROM.begin(12);
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, HIGH); 
   
   pinMode(RLY_1, OUTPUT);// Define o Rele como saída
   pinMode(Led, OUTPUT);
    
   digitalWrite(RLY_1, HIGH);// Liga o rele do roteador             
   RLY_1_State = HIGH;             
   digitalWrite(Led, HIGH);
   delay(1000);
   digitalWrite(Led, LOW);
   
   delay(WAITING_DELAY);
   Roteador = EEPROM.read(0);
   Internet = EEPROM.read(1);
   Reset = EEPROM.read(2);
   //EEPROM.write(0,0);
   //EEPROM.write(1,0);
   //EEPROM.write(2,0);
   //EEPROM.commit();
  wm.addParameter(&custom_output);
  //O portal fica 3 minutos no ar 
  wm.setTimeout(180); //Configura o tempo do wifi manager, ate reiniciar

  //Abertura do wifi manager
  bool connect_to_me = wm.autoConnect("SmartPlug - Eletromidia", "3l3tr0m1d1@123");

  if (!connect_to_me) {
    // Resetar ou colocar em modo de espera profundo (deep sleep)
    ESP.restart();
  }
  
 //Colocar um verificação se a conexão com o thinksboards é verdadeira
    if (!tb.connected()) {
    // Connect to the ThingsBoard
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      return;
    }
  }else{
    return;
  }

  //--- Dados fixos que vão para o thingsboard
  tb.sendAttributeString("localIp", WiFi.localIP().toString().c_str());
  tb.sendAttributeString("macAddress", WiFi.macAddress().c_str());
  tb.sendAttributeString("ssid", WiFi.SSID().c_str());
  //tb.sendTelemetryString("MacAddress", WiFi.macAddress().c_str());
  tb.sendTelemetryInt("Roteador",Roteador);
  tb.sendTelemetryInt("Internet",Internet);
  tb.sendTelemetryInt("Reset",Reset);
  delay(40);
    
}

void loop() {
  
 WiFiManager wm;
   
  if((millis() - millisTarefa3) < 1000){ // Verifica se teve alteração no RPC - Botão de reset 
          }
            if((millis() - millisTarefa3) > 1000){
            millisTarefa3 = millis();
       }
          if (!subscribed) {
         if (!tb.RPC_Subscribe(callbacks, callbacks_size)) {
           return;
      }
          subscribed = true;
     }        
 tb.loop();
//------------------------

 if (try_ping_roteador() == 0) { //---- Tenta realizar o ping no roteador
    
    bool revived = reviveRouter(&wm, 3);
    if (!revived) {
      
      if((millis() - millisTarefa1) < 2000){
      }
        if((millis() - millisTarefa1) > 2000){
        millisTarefa1 = millis();
        EEPROM.write(0,Roteador+1);
        EEPROM.commit();
        digitalWrite(Led, HIGH);
        ESP.restart();
      }
    }
    return; 
  }
//--------------------------
   if (try_ping_Internet() == 0) { //---- Tenta realizar o ping na internet
      bool revived = reviveRouter(&wm, 3);
        if (!revived) { 
          if((millis() - millisTarefa2) < 2000){
           }
            if((millis() - millisTarefa2) > 2000){
            millisTarefa2 = millis();
            EEPROM.write(1,Internet+1);
            EEPROM.commit();
            digitalWrite(Led, HIGH);
            ESP.restart();
          }    
       }
    return;
  }
}

bool try_ping_Internet() { //---- Função que tenta o ping na internet.
   int attempt = 0; // Tentativa
  while(attempt < MAX_PING_RETRIES) {
    if(attempt > 0) { // Esperar o atraso em todas as tentativas exceto a primeira
      if((millis() - millisTarefa4) < RETRY_DELAY){
          }
            if((millis() - millisTarefa4) > RETRY_DELAY){
            millisTarefa4 = millis();
          }
    }
    web_ping = Ping.ping("www.google.com", 1);
    if (web_ping == 1) {
      digitalWrite(Led, LOW);
      return true;
    }
    attempt++;
  }
  return false;
}

bool try_ping_roteador(){//---- Função que tenta o ping no roteador.
  int attempt = 1; // Tentativa
  while (attempt < MAX_PING_RETRIES) {
    if (attempt > 1) { // Esperar o atraso em todas as tentativas exceto a primeira
      delay(RETRY_DELAY);
    }
    local_ping = Ping.ping(WiFi.gatewayIP()); 
    if((millis() - millisTarefa5) < 500){
          }
            if((millis() - millisTarefa5) > 500){
            millisTarefa5 = millis();
          }
          if(Ping.ping(WiFi.gatewayIP())) {
            digitalWrite(Led, LOW);
             return true;
          } else {
             attempt++;
           }     
         }
     return false;
}

void desliga_() {//--- Função que desliga o roteador
    digitalWrite(RLY_1, LOW);
    RLY_1_State = LOW;
    //digitalWrite(Led, LOW);
    delay(60000);  
}

void liga_() { //---Função que liga o roteador
  digitalWrite(RLY_1, HIGH);             
  RLY_1_State = HIGH;                    
  //digitalWrite(Led, HIGH); 
}

//----- Função para reviver o roteador
bool reviveRouter(WiFiManager *wm, int tries) {
  int thisTry = 1;
  while (thisTry < tries) { //--- Desligando e Religando o roteador
    desliga_();
        if((millis() - millisTarefa6) < 5000){
          }
            if((millis() - millisTarefa6) > 5000){
            millisTarefa6 = millis();
          }
    liga_();
         if((millis() - millisTarefa6) < 5000){
          }
            if((millis() - millisTarefa6) > 5000){
            millisTarefa6 = millis();
          }
    if (reconnect(wm, true)) { //--- Chama a função para tentar conectar ao roteador
      return true;
    }
    if((millis() - millisTarefa7) < 500){
          }
            if((millis() - millisTarefa7) > 500){
            millisTarefa7 = millis();
          }
    thisTry++;
  }
  return false;
}

//---- Controle do led
void bluticker() {
  int state = digitalRead(Led);  
  digitalWrite(Led, !state);     
}
