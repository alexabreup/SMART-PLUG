#include <WiFiManager.h>
#include <ESPping.h>
#include <EEPROM.h>
#include <ThingsBoard.h> // versão 0.6.0
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Parametros do wifi manager
WiFiManager wm;
char output[2] = "5";
WiFiManagerParameter custom_output("output", "output", output, 2);

//#define TOKEN   "rFI2SUd8CXjff0Dvdmt7" // Smart Plug 1
//#define TOKEN   "Nk2HfP2x4N57Qzsgq6kX" // Smart Plug 2
//#define TOKEN   "czId22LdbdFQ0DvjvR4q" // Smart Plug 3
//#define TOKEN   "XxcuXTmlIWrQf6ZDPHUg" // Smart Plug 4
//#define TOKEN   "tzJ10UlgN4thTUIFVZpg" // Smart Plug 5
//#define TOKEN   "3VULMkAyFXcCNnABOb6g" // Smart Plug 6
//#define TOKEN   "N1KGOGINBVeqp715DOx5" // Smart Plug 7
//#define TOKEN   "lKise9EdlzRbFmS4Zqhf" // Smart Plug 8
#define TOKEN   "1TkrUTnUamVsvNULBSf6" // Smart Plug 9  
//#define TOKEN   "usy2yP3AiOGCTTgqEhkj" // Smart Plug 10
//#define TOKEN   "mGYEzaaafZsaM8UNkIKq" // Smart Plug 11
//#define TOKEN   "sigJIwya1Yb7LRNasAue" // Smart Plug 12
//#define TOKEN   "dtyLZnr5GIX9Mo6AcaEc" // Smart Plug 13
//#define TOKEN   "lkoZeGAJHNGkT9Lmq8t2" // Smart Plug 14
//#define TOKEN   "lYykcii6I4oeJtRXgWJT" // Smart Plug 15
//#define TOKEN   "al7L5wJhzCwvr8wVr09E" // Smart Plug 16
//#define TOKEN   "SdFgCVIBe9CiIcHQQewA" // Smart Plug 17
//#define TOKEN   "ZIFl8vk0Be0RtslzdpOO" // Smart Plug 18
//#define TOKEN   "2qy2uMcZDxSY14aVJeh0" // Smart Plug 19
//#define TOKEN   "F5rmYbD3ApjwBGF3bjqV" // Smart Plug 20
//#define TOKEN   "YTrNOYOLp0DqJkBvNjrW" // Smart Plug 21
//#define TOKEN   "pa4IikmkTUPLJLd24eXP" // Smart Plug 22
//#define TOKEN   "glBDxHeeugOKOadLSDUK" // Smart Plug 23
//#define TOKEN   "q30CnjBRLtNPaRfq3PyP" // Smart Plug 24
//#define TOKEN   "UZaRNf5GyJv8iRxIfRZa" // Smart Plug 25
//#define TOKEN   "C2KkEN4VgComd9n0ZDbj" // Smart Plug 26
//#define TOKEN   "HtcM2Xf12m6wYqoJfGtV" // Smart Plug 27
//#define TOKEN   "b0apHR23yxJIksgE9Iso" // Smart Plug 28
//#define TOKEN   "jurIofOyEABxsecmCtY9" // Smart Plug 29
//#define TOKEN   "iGU8QhlQzXeOg48DjbnW" // Smart Plug 30
//#define TOKEN   "olmVsVeh2u6kOdS3fopQ" // Smart Plug 31
//#define TOKEN   "2cjMgFXqJNGkqx4Otbf7" // Smart Plug 32
//#define TOKEN   "bfNRfYzCnnJtnm46EY2L" // Smart Plug 33
//#define TOKEN   "gpXONqaNL6QSiGxIC7LR" // Smart Plug 34
//#define TOKEN   "zvBdq5bfEx9bJdxLzKp0" // Smart Plug 35
//#define TOKEN   "8H46w2Jvuj26LkVmRtiM" // Smart Plug 36
//#define TOKEN   "zlGb0qmhiNbzHBhlGbWb" // Smart Plug 37
//#define TOKEN   "r4LFQcqDy34izW0fNnBj" // Smart Plug 38
//#define TOKEN   "dV6swUsJsb8xCauInT6u" // Smart Plug 39
//#define TOKEN   "NObaGo2D8pR7R8oU9XWt" // Smart Plug 40
//#define TOKEN   "MLqDJcvR4VwnstaEPYJt" // Smart Plug 41
//#define TOKEN   "OXlIOf4AG0WFAQfkr1uD" // Smart Plug 42
//#define TOKEN   "3kjGG6BxZdopNjQz7ozx" // Smart Plug 43
//#define TOKEN   "fK4BaHiWmCQeHNYqydlm" // Smart Plug 44
//#define TOKEN   "RfUCdbPKqZdViRDkQT59" // Smart Plug 45
//#define TOKEN   "3hfju8EYzdqVstuAH2I7" // Smart Plug 46
//#define TOKEN   "bMeIBOV3et6eXjUNKQbZ" // Smart Plug 47
//#define TOKEN   "cKZG2yaL2oqdckgpRzi3" // Smart Plug 48
//#define TOKEN   "97MVuruDlDbItBJCe6L4" // Smart Plug 49
//#define TOKEN   "JJvYJ8EIU1v8PLHiAkaB" // Smart Plug 50
//#define TOKEN   "4c4pHf5YJtWPvRa90cSy" // Smart Plug 51


//#define TOKEN   "yeiNCNclPh8jwBZCrCcE" // Smart Plug TESTE IOT

#define THINGSBOARD_SERVER "thingsboard.eletromidia.com.br"


// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

int timeZone = -3;

// Inicialize o cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.br.pool.ntp.org", timeZone * 3600, 60000); // Offset de 0 segundos e atualização a cada 60 segundos

#define RLY_1  14
#define Button_1  12
#define Button_2  5
#define Button_3  4
#define Button_4  3
#define Button_5  2
#define Button_6  1
#define Button_7  0
#define Led    13

const int MAX_PING_RETRIES = 5;
const int MAX_RECONNECT_RETRIES = 3;
const int RECONNECT_DELAY = 3;
const int RETRY_DELAY = 100000; // 5 Minutos tempo que o Roteador fica desligado = 300000
const int WAITING_DELAY = 30000; // 30 Segundos
const int THINGSBOARD_DELAY = 30000; // A cada 15 Minutos enviar dados para o ThingsBoard
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

int hora = 0;
int minuto = 0;

int attempt = 0;

bool Send = false;
bool routerReset = false; // Indica se o roteador foi resetado



void setup() {

  Serial.begin(115200);
  EEPROM.begin(512);

  pinMode(RLY_1, OUTPUT);// Define o Rele como saída
  pinMode(Led, OUTPUT);
  pinMode(Button_1, INPUT_PULLUP);
  pinMode(Button_2, INPUT_PULLUP);
  pinMode(Button_3, INPUT_PULLUP);
  pinMode(Button_4, INPUT_PULLUP);
  pinMode(Button_5, INPUT_PULLUP);
  pinMode(Button_6, INPUT_PULLUP);
  pinMode(Button_7, INPUT_PULLUP);
  

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
  String ssid = "SmartPlug 9 - "; //**************************************************************   Renomear com  numero de dispositivos  **********************************************
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
      tb.sendAttributeString("Smart Plug ", " 9 ");   // **************************************************************   Renomear com  numero de dispositivos  **********************************************
      delay(50);
      break;
    }
  }

  millisTarefa2 = THINGSBOARD_DELAY + 1000;
  Serial.println("");
  Serial.print("***  VOID SETUP ***");
  Serial.println("");
  millisTarefa2 = millis();
}

bool subscribed = false;

RPC_Response Hora(const RPC_Data &data)
{
  void processTime(const JsonVariantConst & data);

  int Hora = (data);
  hora = Hora;
  Serial.print("Hora = ");
  Serial.print(hora);
  Serial.print(" : ");
  Serial.println(minuto);


  // Just an response example
  return RPC_Response("getHora", true);
}



RPC_Response Minuto(const RPC_Data &data)
{
  void processTime(const JsonVariantConst & data);

  int Minuto = (data);
  minuto = Minuto;
  Serial.print("Hora = ");
  Serial.print(hora);
  Serial.print(" : ");
  Serial.println(minuto);

  // Just an response example
  return RPC_Response("getMinuto", true);
}


RPC_Response Reset(const RPC_Data &data)
{
  void processTime(const JsonVariantConst & data);

  int RESET = (data);

  if (RESET == HIGH ) {
    // Just an response example
    routerReset = true;
    //return RPC_Response("getReset", true);
    Serial.println(" ");
    Serial.println("Box desligado");
    Serial.println(" ");
    //desliga_();
  }

  // Just an response example
  return RPC_Response("getReset", true);
}

const size_t callbacks_size = 3;
RPC_Callback callbacks[callbacks_size] = {
  { "setMinuto", Minuto},
  { "setHora", Hora},
  { "setReset", Reset}
};



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
      subscribed = false;
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

  if (!subscribed) {
    Serial.println("Subscribing for RPC...");
    if (!tb.RPC_Subscribe(callbacks, callbacks_size)) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    Serial.println("Subscribe done");
    subscribed = true;
  }

  tb.loop();

  if ((millis() - millisTarefa2) >= THINGSBOARD_DELAY) {
    tb.sendTelemetryInt("Button_1", digitalRead(Button_1));
    tb.sendTelemetryInt("Button_2", digitalRead(Button_2));
    tb.sendTelemetryInt("Button_3", digitalRead(Button_3));
    tb.sendTelemetryInt("Button_4", digitalRead(Button_4));
    tb.sendTelemetryInt("Button_5", digitalRead(Button_5));
    tb.sendTelemetryInt("Button_6", digitalRead(Button_6));
    tb.sendTelemetryInt("Button_7", digitalRead(Button_7));
    
  }

  if ((millis() - millisTarefa2) > THINGSBOARD_DELAY) {
    millisTarefa2 = millis();
  }


  if(digitalRead(Button_4) == LOW){
    digitalWrite(Led, HIGH);
  }
  else{
    digitalWrite(Led, LOW);
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
    sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    tb.sendTelemetryString("Hora", timeString);
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
    char timeString[6];
    sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    tb.sendTelemetryString("Hora", timeString);
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
  tb.sendTelemetryInt("RESET HORA", 0); // Envia um valor indicando que houve um reset por horário
  tb.loop();
}
