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

#define TOKEN   "1TkrUTnUamVsvNULBSf6" // Smart Plug 9 
#define THINGSBOARD_SERVER "thingsboard.eletromidia.com.br"

// Initialize ThingsBoard client
WiFiClient espClient;
ThingsBoard tb(espClient);

int timeZone = -3;

// Inicialize o cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.br.pool.ntp.org", timeZone * 3600, 60000); // Offset de 0 segundos e atualização a cada 60 segundos

// Definição dos pinos
#define Relay  14
#define Button 3
#define Led    13

const int MAX_RETRIES = 5; // Número máximo de tentativas
int ATTEMPT = 0; // Tentar

const int RETRY_DELAY = 60000; // 5 Minutos tempo que o Roteador fica desligado = 300000
const int WAITING_DELAY = 30000; // 30 Segundos
const int THINGSBOARD_DELAY = 120000; // A cada 5 Minutos enviar dados para o ThingsBoard
const int PISCA_DELAY = 300; //

unsigned long lastPing;
unsigned long lastStatusSend;
unsigned long millisTarefa;
unsigned long millisPisca ;


unsigned long buttonPressStart = 0;
bool apModeTriggered = false;

int hora   = 0;
int minuto = 0;

bool Send = false;
bool routerReset = false; // Indica se o roteador foi resetado

// Variáveis globais para armazenar o SSID e a senha aleatórios
String randomSSID;
String randomPassword;

String generateRandomString(int length) {
  String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  String randomString;
  for (int i = 0; i < length; i++) {
    randomString += chars[random(0, chars.length())];
  }
  return randomString;
}


void setup() {

  Serial.begin(115200);

  pinMode(Button, INPUT_PULLUP);
  pinMode(Relay , OUTPUT);// Define o Rele como saída
  pinMode(Led, OUTPUT);

  digitalWrite(Relay , HIGH);// Liga o rele do roteador
  digitalWrite(Led, HIGH);
  delay(1000);
  digitalWrite(Led, LOW);
  delay(1000);

  //wm.resetSettings();
  WiFiManager wm;
  wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar

  lastStatusSend = millis();
  lastPing = millis();
  millisTarefa = millis();
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

  if (RESET == true ) {
    Serial.println(" ");
    Serial.println("Box desligado");
    Serial.println(" ");
    routerReset = true;
    millisTarefa = millis();
    millisPisca = millis();
  }
  // Just an response example
  return RPC_Response("getReset", true);
}

const size_t callbacks_size = 3;
RPC_Callback callbacks[callbacks_size] = {
  { "setHora", Hora},
  { "setMinuto", Minuto},
  { "setReset", Reset}
};



void loop() {
  WiFiManager wm;

  if ( routerReset == false) {
    Serial.print("lastPing = ");
    Serial.print(millis() - lastPing);
    Serial.print("  /  lastStatusSend  = ");
    Serial.println(millis() - lastStatusSend );
    if ((millis() - lastPing) >= WAITING_DELAY) {
      digitalWrite(Led, HIGH);
      lastPing = millis();
      checkConnectivity();

    }
    else if ((millis() - lastStatusSend) >= THINGSBOARD_DELAY && pingInternet()) {
      digitalWrite(Led, HIGH);
      lastStatusSend = millis();
      EnviarThingsBoard();
    }
    else {
      digitalWrite(Led, LOW);
    }
  }
  checkButtonPress();
  resetModem();
  OffTime();

}

void checkButtonPress() {
  // Verifica o estado do botão
  if (digitalRead(Button) == LOW) {
    if (buttonPressStart == 0) {
      // Inicia o contador ao pressionar o botão
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart > 5000 && !apModeTriggered) {
      // Se o botão for pressionado por mais de 5 segundos, inicia o modo AP
      randomSSID = "Smart_" + generateRandomString(6);
      randomPassword = generateRandomString(8);
      Serial.println("Modo AP ativado.");
      digitalWrite(Led, HIGH);
      WiFiManager wm;
      wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar
      wm.startConfigPortal("SmartPlug_AP");
      apModeTriggered = true;
    }
  } else {
    // Reseta o contador e flag quando o botão é solto
    buttonPressStart = 0;
    apModeTriggered = false;
    digitalWrite(Led, LOW);
  }
}

void OffTime() {
  timeClient.update();
  time_t currentEpoch = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentEpoch);

  // Verificar se é hora para reiniciar o roteador
  if (timeinfo->tm_hour == hora && timeinfo->tm_min >= minuto) {
    char timeString[6];
    sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    tb.sendTelemetryString("Hora Reset", timeString);
    delay(50);
    routerReset = true;
    millisTarefa = millis();
    millisPisca = millis();
  }
}

void checkConnectivity() {
  Serial.println("checkConnectivity()");
  if (!pingModem()) {
    routerReset = true;
    millisTarefa = millis();
    resetModem();
  } else if (!pingInternet()) {
    ESP.restart();
  }
}

bool pingModem() {
  return Ping.ping(WiFi.gatewayIP(), 5);
}

bool pingInternet() {
  return Ping.ping("www.google.com", 5);
}


void EnviarThingsBoard() {
  Serial.println("EnviarThingsBoard()");
  // Atualizar o cliente NTP e obter a hora atual
  timeClient.update();
  time_t currentEpoch = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentEpoch);
  Serial.println("");
  Serial.println("Enviar ThingsBoard");
  //tb.sendTelemetryInt("Ping Net", valuePingNet);
  //tb.sendTelemetryInt("Ping Rot", valuePingRot);
  //tb.sendTelemetryInt("Reset ROT", valueROT);
  //tb.sendTelemetryInt("Reset ESP", valueESP);
  //tb.sendTelemetryInt("Hora",timeinfo->tm_hour + ":" + timeinfo->tm_min);
  char timeString[6];
  sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  tb.sendTelemetryString("Hora", timeString);
  tb.sendAttributeString("LocalIP", WiFi.localIP().toString().c_str());
  tb.sendAttributeString("MacAddress", WiFi.macAddress().c_str());
  tb.sendAttributeString("SSID", WiFi.SSID().c_str());
  //LimpaEEPROM();
}


void resetModem() { //--- Função que desliga o roteador

  if ( routerReset == true) {
    Serial.print("resetModem()");
    Serial.print(" / millisTarefa = ");
    Serial.println((millis() - millisTarefa)/1000);
    if ((millis() - millisTarefa) <= RETRY_DELAY && routerReset == true) {
      digitalWrite(Relay , LOW);
      Serial.println("Desliga Box");
      if ((millis() - millisPisca) < PISCA_DELAY / 2) {
        digitalWrite(Led, HIGH);
      }
      if ((millis() - millisPisca) > PISCA_DELAY / 2) {
        digitalWrite(Led, LOW);
      }
      if ((millis() - millisPisca) > PISCA_DELAY) {
        millisPisca = millis();
      }
    }
    else if ((millis() - millisTarefa) > RETRY_DELAY && routerReset == true) {
      liga_();
    }
    else if ((millis() - millisTarefa) > (RETRY_DELAY + (RETRY_DELAY / 3)) && routerReset == true) {
      digitalWrite(Led, LOW);
      routerReset = false;
      millisTarefa = millis();
      lastStatusSend = millis();
      lastPing = millis();
    }
  }

}

void liga_() { //---Função que liga o roteador
  Serial.println("liga_()");
  digitalWrite(Relay , HIGH);
  digitalWrite(Led, HIGH);
}
