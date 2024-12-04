#include <WiFiManager.h>
#include <ESPping.h>
#include <EEPROM.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Parametros do wifi manager
WiFiManager wm;
char output[2] = "5";
WiFiManagerParameter custom_output("output", "output", output, 2);

//#define TOKEN   "rFI2SUd8CXjff0Dvdmt7" // Smart Plug 1  OK
//#define TOKEN   "Nk2HfP2x4N57Qzsgq6kX" // Smart Plug 2  OK
//#define TOKEN   "czId22LdbdFQ0DvjvR4q" // Smart Plug 3  OK
//#define TOKEN   "XxcuXTmlIWrQf6ZDPHUg" // Smart Plug 4  OK
//#define TOKEN   "tzJ10UlgN4thTUIFVZpg" // Smart Plug 5  OK
//#define TOKEN   "3VULMkAyFXcCNnABOb6g" // Smart Plug 6  OK
//#define TOKEN   "N1KGOGINBVeqp715DOx5" // Smart Plug 7  OK
//#define TOKEN   "lKise9EdlzRbFmS4Zqhf" // Smart Plug 8  OK
//#define TOKEN   "1TkrUTnUamVsvNULBSf6" // Smart Plug 9  OK
//#define TOKEN   "usy2yP3AiOGCTTgqEhkj" // Smart Plug 10 OK
//#define TOKEN   "mGYEzaaafZsaM8UNkIKq" // Smart Plug 11 OK
//#define TOKEN   "sigJIwya1Yb7LRNasAue" // Smart Plug 12 OK
//#define TOKEN   "dtyLZnr5GIX9Mo6AcaEc" // Smart Plug 13 OK
//#define TOKEN   "lkoZeGAJHNGkT9Lmq8t2" // Smart Plug 14 OK
//#define TOKEN   "lYykcii6I4oeJtRXgWJT" // Smart Plug 15 OK
//#define TOKEN   "al7L5wJhzCwvr8wVr09E" // Smart Plug 16 OK
//#define TOKEN   "SdFgCVIBe9CiIcHQQewA" // Smart Plug 17 OK
//#define TOKEN   "ZIFl8vk0Be0RtslzdpOO" // Smart Plug 18 OK
//#define TOKEN   "2qy2uMcZDxSY14aVJeh0" // Smart Plug 19 OK
  #define TOKEN   "F5rmYbD3ApjwBGF3bjqV" // Smart Plug 20 OK

#define THINGSBOARD_SERVER "thingsboard.eletromidia.com.br"


// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

int timeZone = -3;

// Inicialize o cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.br.pool.ntp.org", timeZone * 3600, 60000); // Offset de 0 segundos e atualização a cada 60 segundos

#define RLY_1  14
#define RLY_2
#define RLY_3
#define Led    13

const int MAX_PING_RETRIES = 5;
const int MAX_RECONNECT_RETRIES = 3;
const int RECONNECT_DELAY = 3;
const int RETRY_DELAY = 300000; // 2 Minutos tempo que o Roteador fica desligado
const int WAITING_DELAY = 30000; // 30 Segundos
const int THINGSBOARD_DELAY = 300000; // A cada 5 Minutos enviar dados para o ThingsBoard 2 = 300000
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

int valuePingNet;
int PingNet;
int valuePingRot;
int valueROT;
int valueRES;
int valueESP;

int attempt = 0;

bool Send = false;
bool routerReset = false; // Indica se o roteador foi resetado

void setup() {

  Serial.begin(115200);
  EEPROM.begin(512);

  pinMode(RLY_1, OUTPUT);// Define o Rele como saída
  pinMode(Led, OUTPUT);

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

  delay(100);

  Serial.print("Ping Net  - "); Serial.print(valuePingNet , DEC);  Serial.println();
  Serial.print("Ping Rot  - "); Serial.print(valuePingRot , DEC);  Serial.println();
  Serial.print("Reset ROT - "); Serial.print(valueROT , DEC);  Serial.println();
  Serial.print("Reset ESP - "); Serial.print(valueESP , DEC);  Serial.print(" / "); Serial.print(valueRES, DEC);  Serial.println();

  //wm.resetSettings();
  WiFiManager wm;

  wm.addParameter(&custom_output);
  wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar

  //Constrói o nome da rede com o endereço MAC
  String ssid = "SmartPlug 20 - "; //**************************************************************   Renomear com  numero de dispositivos  **********************************************
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; ++i) {
    if (i > 0) ssid += ".";
    ssid += String(mac[i], HEX);
  }

  //Abertura do wifi manager
  // bool connect_to_me = wm.autoConnect(ssid.c_str(), "3l3tr0m1d1@");

  //if (!connect_to_me) {
  if (!wm.autoConnect(ssid.c_str(), "3l3tr0m1d1@")) {
    // Resetar ou colocar em modo de espera profundo (deep sleep)
    valueESP++;
    EEPROM.write(addressESP, valueESP);
    EEPROM.commit();
    Serial.println("Erro connect_to_me");
    return;
  }

  delay(500);
  attempt = 0;
  while (!tb.connected() && attempt < MAX_PING_RETRIES) {
    // Reconectar apenas se a conexão com o ThingsBoard foi perdida
    Serial.println("Reconnecting to ThingsBoard...");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      attempt++;
      Serial.print("Failed to reconnect = ");
      Serial.println(attempt);
      //return;
    }
    else {
      Serial.println("Conectado ao ThingsBoard.");
      //--- Dados fixos que vão para o thingsboard
      tb.sendAttributeString("LocalIP", WiFi.localIP().toString().c_str());
      tb.sendAttributeString("MacAddress", WiFi.macAddress().c_str());
      tb.sendAttributeString("SSID", WiFi.SSID().c_str());
      tb.sendAttributeString("Smart Plug ", " 20 ");   // **************************************************************   Renomear com  numero de dispositivos  **********************************************
      delay(50);
      break;
    }
  }

  millisTarefa2 = THINGSBOARD_DELAY + 1000;
  Serial.println("");
  Serial.print("***  VOID SETUP ***");
  Serial.println("");

}

void loop() {
  //Serial.println("VOID LOOP");
  WiFiManager wm;

  attempt = 0;
  while (!tb.connected() && (attempt < MAX_PING_RETRIES)) {
    // Reconectar apenas se a conexão com o ThingsBoard foi perdida
    Serial.println("");
    Serial.println("Reconectando ao ThingsBoard");
    Serial.println("");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      attempt++;
      Serial.println("");
      Serial.print("Falha a reconectar ao ThingsBoard");
      Serial.println("");
      delay(100);
      //return;
    }
    else {
      Serial.println("");
      Serial.println("Conectado ao ThingsBoard.");
      Serial.println("");
      EnviarThingsBoard(); //Enviar para o ThingsBoard
      tb.loop();
      break;
    }
  }
  
  // Atualizar o cliente NTP e obter a hora atual
  timeClient.update();
  time_t currentEpoch = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentEpoch);

  // Verificar se é 20h para reiniciar o roteador
  if (timeinfo->tm_hour == 5 && (timeinfo->tm_min >= 0 && timeinfo->tm_min <= 2) && routerReset == false) {
    char timeString[6];
    sprintf(timeString,"%02d:%02d",timeinfo->tm_hour,timeinfo->tm_min);
    tb.sendTelemetryString("Hora Reset",timeString);
    tb.sendTelemetryInt("RESET HORA", 1); // Envia um valor indicando que houve um reset por horário
    delay(50);
    routerReset = true;
    desliga_();
  }

  // Verificar se é 20h para reiniciar o roteador
  if (timeinfo->tm_hour == 17 && (timeinfo->tm_min >= 0 && timeinfo->tm_min <= 2) && routerReset == false) {
    char timeString[6];
    sprintf(timeString,"%02d:%02d",timeinfo->tm_hour,timeinfo->tm_min);
    tb.sendTelemetryString("Hora Reset",timeString);
    tb.sendTelemetryInt("RESET HORA", 1); // Envia um valor indicando que houve um reset por horário
    delay(50);
    routerReset = true;
    desliga_();
  }

  // Ping local IP
  Serial.println("");
  Serial.println("Ping local IP");
  Serial.println("");
  attempt = 0;
  millisTarefa1 = millis();
  while (attempt < MAX_PING_RETRIES) {
    if ((millis() - millisTarefa1) < WAITING_DELAY) {
      Serial.print(".");
      digitalWrite(Led, LOW);
    }
    if ((millis() - millisTarefa1) > WAITING_DELAY) { // cada 30 segundos da um ping no roteados
      if (Ping.ping(WiFi.gatewayIP()) > 0) {
        Serial.println("");
        Serial.println("Ping no Roteador OK");
        Serial.println("");
        digitalWrite(Led, HIGH);
        delay(100);
        break;
      }
      else {
        Serial.println("");
        Serial.println("Falha Ping no Roteador!");
        Serial.println("");
        digitalWrite(Led, HIGH);
        attempt++;
        valuePingRot++;
        delay(50);
        EEPROM.write(addressPingRot, valuePingRot);
        EEPROM.commit();
        delay(50);
        if (attempt >= MAX_PING_RETRIES) {
          Serial.println("");
          Serial.println("Desligar Modem Roteador");
          Serial.println("");
          desliga_();
          break;
        }
        millisTarefa1 = millis();
      }
    }
  }

  attempt = 0;
  millisTarefa1 = millis();

  // Ping Host
  Serial.println("");
  Serial.println("Ping Host");
  Serial.println("");
  while (attempt < MAX_PING_RETRIES) {
    if ((millis() - millisTarefa1) < WAITING_DELAY) {
      Serial.print(".");
      digitalWrite(Led, LOW);
    }
    if ((millis() - millisTarefa1) > WAITING_DELAY) {
      if (Ping.ping("www.google.com") > 0) {
        Serial.println("");
        Serial.println("Ping na Internet OK");
        Serial.println("");
        digitalWrite(Led, HIGH);
        delay(100);
        break;
      }
      else {
        Serial.println("");
        Serial.println("Falha Ping na Internet");
        Serial.println("");
        digitalWrite(Led, HIGH);
        attempt++;
        valuePingNet++;
        delay(50);
        EEPROM.write(addressPingNet, valuePingNet);
        EEPROM.commit();
        delay(50);
        millisTarefa1 = millis();
        if (attempt >= MAX_PING_RETRIES) {
          digitalWrite(Led, LOW);
          valueRES++;
          delay(50);
          EEPROM.write(addressRES, valueRES);
          EEPROM.commit();
          delay(50);
          Serial.println("");
          Serial.println("Resetar SmartPlug");
          Serial.println("");
          if (valueRES >= MAX_RECONNECT_RETRIES) {
            Serial.println("");
            Serial.println("Desligar Modem Roteador");
            Serial.println("");
            valueRES = 0;
            EEPROM.write(addressRES, valueRES);
            EEPROM.commit();
            desliga_();
            break;
          }
          ESP.restart();//ESP.reset();
        }

      }

    }
  }

}

void EnviarThingsBoard() {
  // Atualizar o cliente NTP e obter a hora atual
  timeClient.update();
  time_t currentEpoch = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentEpoch);

  if ((millis() - millisTarefa2) > THINGSBOARD_DELAY) {
    Serial.println("");
    Serial.println("Enviar ThingsBoard");
    tb.sendTelemetryInt("Ping Net", valuePingNet);
    tb.sendTelemetryInt("Ping Rot", valuePingRot);
    tb.sendTelemetryInt("Reset ROT", valueROT);
    tb.sendTelemetryInt("Reset ESP", valueESP);
    //tb.sendTelemetryInt("Hora",timeinfo->tm_hour + ":" + timeinfo->tm_min);
    char timeString[6];
    sprintf(timeString,"%02d:%02d",timeinfo->tm_hour,timeinfo->tm_min);
    tb.sendTelemetryString("Hora",timeString);
    //tb.sendTelemetryInt("Hora", timeinfo->tm_hour);
    //tb.sendTelemetryInt("Minuto",timeinfo->tm_min);
    //millisTarefa2 = millis();
    delay(50);
    //LimpaEEPROM();
  }
  if ((millis() - millisTarefa2) > THINGSBOARD_DELAY + 3000) {
    Serial.println("");
    Serial.println("Limpa EEPROM");
    Serial.println("");
    tb.sendTelemetryInt("Ping Net", valuePingNet);
    tb.sendTelemetryInt("Ping Rot", valuePingRot);
    tb.sendTelemetryInt("Reset ROT", valueROT);
    tb.sendTelemetryInt("Reset ESP", valueESP);
    tb.sendTelemetryInt("RESET HORA", 0); // Envia um valor indicando que houve um reset por horário
    delay(50);
    millisTarefa2 = millis();
    Serial.print("millisTarefa2 = ");
    Serial.println(millisTarefa2);
    delay(100);
    LimpaEEPROM();
    routerReset = false;
  }
}

void LimpaEEPROM() {
  valuePingNet = 0;
  valuePingRot = 0;
  valueROT = 0;
  valueESP = 0;
  valueRES = 0;
  EEPROM.write(addressPingNet, valuePingNet);
  EEPROM.write(addressPingRot, valuePingRot);
  EEPROM.write(addressROT, valueROT);
  EEPROM.write(addressESP, valueESP);
  EEPROM.write(addressRES, valueRES);
  EEPROM.commit();

}

void desliga_() {//--- Função que desliga o roteador
  millisTarefa3 = millis();
  millisPisca = millis();
  digitalWrite(RLY_1, LOW);
  Serial.print("Desliga Box");
  valueROT++;
  delay(50);
  EEPROM.write(addressROT, valueROT);
  EEPROM.commit();
  delay(50);
  while ((millis() - millisTarefa3) < RETRY_DELAY) {

    if ((millis() - millisPisca) < PISCA_DELAY / 2) {
      digitalWrite(Led, HIGH);
    }
    if ((millis() - millisPisca) > PISCA_DELAY / 2) {
      digitalWrite(Led, LOW);
    }
    if ((millis() - millisPisca) > PISCA_DELAY) {
      Serial.print("millisTarefa3 = ");
      Serial.println((millis() - millisTarefa3) / 1000);
      millisPisca = millis();
    }
  }
  liga_();
}

void liga_() { //---Função que liga o roteador
  digitalWrite(RLY_1, HIGH);
  digitalWrite(Led, HIGH);
  Serial.println("Ligar Box");
  millisTarefa2 = millis();
 
}
