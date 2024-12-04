
//Bibliotecas

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

// Inicialize o cliente NTP
int timeZone = -3;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.br.pool.ntp.org", timeZone * 3600, 60000); // Offset de 0 segundos e atualização a cada 60 segundos

// Definição dos pinos
#define Relay  14
#define Button 3
#define Led    13

const int MAX_RETRIES = 5; // Número máximo de tentativas
int attempt = 0; // Tentar

const int RETRY_DELAY = 60000; // 5 Minutos tempo que o Roteador fica desligado = 300000
const int WAITING_DELAY = 30000; // 30 Segundos
const int THINGSBOARD_DELAY = 120000; // A cada 5 Minutos enviar dados para o ThingsBoard
const int PISCA_DELAY = 300; //

unsigned long lastResetBox;
unsigned long lastStatusSend;
unsigned long lastWait;
unsigned long lastPing;
unsigned long millisPisca ;

int hora   = 0;
int minuto = 0;

int horatb   = 0;
int minutotb = 0;

int valuePingNet;
int valuePingBox;
int valueResetBox;
int valueResetESP;
int valueResetNtp;
int valueResetTb;

// Endereços EEPROM
byte addressPingBox = 0;
byte addressPingNet = 1;
byte addressBox     = 2;
byte addressESP     = 3;
byte addressNtp     = 4;
byte addressTB      = 5;

bool ConnectWifi = false;
bool PingBox = false;
bool PingNet = false;
bool UpdateTime = false;
bool ConnectTB = false;
bool SendTB = false;
bool ResetBox = false;
bool connectwifi = false;
bool timeNTP = false;

//Controle de tempo para tentativa Ping Box, Ping Net, NTP
unsigned long ultimatentativa = 0;
const unsigned long intervalotentativa = 30000;
int tentativas = 0;
const int maxtentativas = 5;

unsigned long buttonPressStart = 0;
bool apModeTriggered = false;

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

bool subscribed = false;

RPC_Response Hora(const RPC_Data &data) {
  void processTime(const JsonVariantConst & data);

  int Hora = (data);
  horatb = Hora;
  Serial.print("Hora = ");
  Serial.print(hora);
  Serial.print(" : ");
  Serial.println(minuto);

  // Just an response example
  return RPC_Response("getHora", true);
}

RPC_Response Minuto(const RPC_Data &data) {
  void processTime(const JsonVariantConst & data);

  int Minuto = (data);
  minutotb = Minuto;
  Serial.print("Hora = ");
  Serial.print(hora);
  Serial.print(" : ");
  Serial.println(minuto);

  // Just an response example
  return RPC_Response("getMinuto", true);
}

RPC_Response Reset(const RPC_Data &data) {
  void processTime(const JsonVariantConst & data);

  int RESET = (data);

  if (RESET == true ) {
    Serial.println(" ");
    Serial.println("Box desligado");
    Serial.println(" ");
    
    ResetBox = true;
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

void setup() {

  Serial.begin(115200);
  EEPROM.begin(512);

  pinMode(Button, INPUT_PULLUP);// Define o Botão como Entrada
  pinMode(Relay , OUTPUT); // Define o Rele como saída
  pinMode(Led, OUTPUT); // Define o Led como saída

  digitalWrite(Relay , HIGH);// Liga o rele do roteador
  digitalWrite(Led, LOW);

  Serial.println("");
  Serial.print("Reset no ESP = ");
  Serial.println(EEPROM.read(addressESP));
  Serial.println("");

  //wm.resetSettings();
  WiFiManager wm;
  wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar
  checkButtonPress();
  lastStatusSend = millis();
  lastPing = millis();
  lastResetBox = millis();
  PingBox  = true;
}

void loop() {
  //WiFiManager wm;
  pingbox();
  ConnectWiFi();
  pingnet();
  updatetimeNTP();
  EnviarThingsBoard();
  OffTime();
  resetBox();

}

void OffTime() {
  if (ResetBox == false) {
    timeClient.update();
    time_t currentEpoch = timeClient.getEpochTime();
    struct tm *timeinfo = localtime(&currentEpoch);

    // Verificar se é hora para reiniciar o roteador
    if (timeinfo->tm_hour == horatb && timeinfo->tm_min >= minutotb) {
      char timeString[6];
      sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
      tb.sendTelemetryString("Hora Reset", timeString);
      delay(50);
      ResetBox = true;
      valueResetBox++;
      EEPROM.write(addressBox, valueResetBox);
      EEPROM.commit();
      millisPisca = millis();
      lastResetBox = millis();
    }
  }
}

void  pingbox() {
  if ( PingBox  == true) {
    valuePingBox = EEPROM.read(addressPingBox);
    if (valuePingBox < MAX_RETRIES) {
      if ((millis() - lastPing) < WAITING_DELAY) {
        digitalWrite(Led, LOW);
      }
      else if ((millis() - lastPing) >= WAITING_DELAY) { // cada 30 segundos da um ping no roteados
        digitalWrite(Led, HIGH);
        if (Ping.ping(WiFi.gatewayIP()) > 0) {
          Serial.println("");
          Serial.println("Ping no Box OK");
          Serial.println("");
          PingBox = false;
          connectwifi = true;
          lastPing = millis();
        }
        else {
          digitalWrite(Led, LOW);
          Serial.println("");
          Serial.println("Falha Ping no Roteador!");
          Serial.println("");
          valuePingBox++;
          EEPROM.write(addressPingBox, valuePingBox);
          EEPROM.commit();
          lastPing = millis();
        }
      }
    }
    else {
      //Salvar na EEprom
      valueResetESP = EEPROM.read(addressESP);
      if (valueResetESP < MAX_RETRIES) {
        valueResetESP++;
        Serial.println("Reset Esp");
        EEPROM.write(addressESP, valueResetESP);
        EEPROM.commit();
        ESP.restart();
      }
      else {
        valueResetBox++;
        EEPROM.write(addressBox, valueResetBox);
        EEPROM.commit();
        PingBox = false;
        ResetBox = true;
        millisPisca = millis();
        lastResetBox = millis();
      }
    }
  }
}

void pingnet() {
  if ( PingNet  == true) {
    valuePingNet = EEPROM.read(addressPingNet);
    if (valuePingNet < MAX_RETRIES) {
      if ((millis() - lastPing) < WAITING_DELAY) {
        digitalWrite(Led, LOW);
      }
      else if ((millis() - lastPing) >= WAITING_DELAY) { // cada 30 segundos da um ping no roteados
        digitalWrite(Led, HIGH);
        if (Ping.ping("www.google.com") > 0) {
          Serial.println("");
          Serial.println("Ping no Net OK");
          Serial.println("");
          lastPing = millis();
          timeNTP = true;
        }
        else {
          digitalWrite(Led, LOW);
          Serial.println("");
          Serial.println("Falha Ping no Net!");
          Serial.println("");
          valuePingBox++;
          EEPROM.write(addressPingNet, valuePingNet);
          EEPROM.commit();
          lastPing = millis();
        }
      }
    }
    else {
      //Salvar na EEprom
      valueResetESP = EEPROM.read(addressESP);
      if (valueResetESP < MAX_RETRIES) {
        valueResetESP++;
        Serial.println("Reset Esp");
        EEPROM.write(addressESP, valueResetESP);
        EEPROM.commit();
        ESP.restart();
      }
      else {
        valueResetBox++;
        EEPROM.write(addressBox, valueResetBox);
        EEPROM.commit();
        PingNet = false;
        ResetBox = true;
        lastResetBox = millis();
        millisPisca = millis();
      }
    }
  }
}

void ConnectWiFi() {
  if ( connectwifi == true) {
    if (WiFi.status() == WL_CONNECTED) {
      PingNet = true;
      digitalWrite(Led, HIGH);
    }
    else {
      WiFi.reconnect();
      digitalWrite(Led, LOW);
      connectwifi = false;
    }
  }

}

void resetBox() { //--- Função que desliga o Box
  if ( ResetBox == true) {
    if ((millis() - lastResetBox) <= RETRY_DELAY) {
      digitalWrite(Relay , LOW);
      if ((millis() - millisPisca) <= (PISCA_DELAY / 2)) {
        digitalWrite(Led, HIGH);
        Serial.println("Liga o Led Azul");
      }
      if ((millis() - millisPisca) > (PISCA_DELAY / 2)) {
        digitalWrite(Led, LOW);
        Serial.println("Desliga o Led Azul");
      }
      if ((millis() - millisPisca) > PISCA_DELAY) {
        millisPisca = millis();
        Serial.println("Reset o Time");
      }
    }
    if ((millis() - lastResetBox) > RETRY_DELAY && (millis() - lastResetBox) < (RETRY_DELAY + (RETRY_DELAY / 2))) {
      liga_();
    }
    if ((millis() - lastResetBox) > (RETRY_DELAY + (RETRY_DELAY / 2))) {

      Serial.println("ConnectWiFi()");
      ResetBox = false;
      PingBox = true;
      lastStatusSend = millis();
    }
  }
}

void liga_() { //---Função que liga o Box
  Serial.println("Liga BOX");
  digitalWrite(Relay , HIGH);
  digitalWrite(Led, HIGH);
}

void updatetimeNTP() {  /// validar
  if ( timeNTP == true) {
    if (millis() - ultimatentativa >= intervalotentativa) {
      ultimatentativa = millis();

      // Tenta atualizar o horário
      if (timeClient.update()) {
        hora = timeClient.getHours();
        minuto = timeClient.getMinutes();
        Serial.printf("Horário atualizado: %02d:%02d\n", hora, minuto);
        tentativas = 0; // resetar as tentativas
      }
      else {
        tentativas++;
        // Serial.println(tentativas, maxtentativas);
        EEPROM.write(addressNtp, tentativas);
        EEPROM.commit();
        if (tentativas >= maxtentativas) {
          Serial.println("Falha ao obter horário do NTP. Tentativas esgotadas.");
          EnviarThingsBoard();
          tentativas = 0; // Resetar para tentar novamente mais tarde
        }
      }
    }
  }
}

void EnviarThingsBoard() {
  if (tb.connected()) {
    if (ConnectTB) {
      if ((millis() - lastStatusSend) >= THINGSBOARD_DELAY) {
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
        char timeString[6];
        sprintf(timeString, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
        tb.sendTelemetryString("Hora", timeString);
        tb.sendAttributeString("LocalIP", WiFi.localIP().toString().c_str());
        tb.sendAttributeString("MacAddress", WiFi.macAddress().c_str());
        tb.sendAttributeString("SSID", WiFi.SSID().c_str());
        lastStatusSend = millis();
        //LimpaEEPROM();
      }
    }
  }
  else {

  }

}

void LimpaEEPROM() {
  valuePingNet  = 0;
  valuePingBox  = 0;
  valueResetBox = 0;
  valueResetESP = 0;
  EEPROM.write(addressPingNet, valuePingNet);
  EEPROM.write(addressPingBox, valuePingBox);
  EEPROM.write(addressBox, valueResetESP);
  EEPROM.write(addressESP, valueResetBox);
  EEPROM.commit();

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
