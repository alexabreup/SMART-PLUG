#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESPping.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

// Parametros do wifi manager
  WiFiManager wm;

#define RLY_1  14              
#define RLY_2      
#define RLY_3        
#define Led    13

const int MAX_PING_RETRIES = 4;
const int MAX_RECONNECT_RETRIES = 3;
const int RECONNECT_DELAY = 3;
const int RETRY_DELAY = 60000;
const int WAITING_DELAY = 10000;
const int SERVER_DELAY = 60000; //= 300000;

unsigned long millisTarefa1 = millis();
unsigned long millisTarefa2 = millis();
unsigned long millisTarefa3 = millis();
unsigned long millisPica = millis();

int addressPingNet = 0;
int addressPingRot = 1;
int addressROT     = 2;
int addressESP     = 3;

int valuePingNet;
int valuePingRot;
int valueROT;
int valueESP;

bool Send = false;
bool ROT = false;

const char* serverAddress = "192.168.1.162:80/Projeto/save_data.php"; // Endereço do script PHP

//const char* host = "192.168.18.5/"; // Endereço IP do seu servidor local
//const int port = 80; // Porta do servidor

void setup() {
   Serial.begin(115200);
  EEPROM.begin(512);
  delay(100);
   
  pinMode(RLY_1, OUTPUT);// Define o Rele como saída
  pinMode(Led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
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

  Serial.print("Ping Net  - ");Serial.print(valuePingNet , DEC);  Serial.println();
  Serial.print("Ping Rot  - ");Serial.print(valuePingRot , DEC);  Serial.println();
  Serial.print("Reset ROT - ");Serial.print(valueROT , DEC);  Serial.println();
  Serial.print("Reset ESP - ");Serial.print(valueESP , DEC);  Serial.println();
 
  //wm.resetSettings();
  WiFiManager wm;

  //wm.addParameter(&custom_output);
  wm.setTimeout(60); //Configura o tempo do wifi manager, ate reiniciar

  //Constrói o nome da rede com o endereço MAC
  String ssid = "SmartPlug - ";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for(int i = 0; i < 6; ++i){
    if(i > 0) ssid += ".";
    ssid += String(mac[i],HEX);
  }
  
  //Abertura do wifi manager
  bool connect_to_me = wm.autoConnect(ssid.c_str(), "3l3tr0m1d1@123");

  if (!connect_to_me) {
    // Resetar ou colocar em modo de espera profundo (deep sleep)
    valueESP++;
    EEPROM.write(addressESP,valueESP);
    EEPROM.commit();
    ESP.restart();
  }
  else{
       valueESP++;
       EEPROM.write(addressESP,valueESP);
       EEPROM.commit();
  }
 
 // Envia os dados para o servidor
  EnviarParaServer();
  
  delay(40);
  millisTarefa2 = millis();

}

void loop() {
  WiFiManager wm;
  digitalWrite(Led,HIGH);
  int attempt = 0;
  millisTarefa1 = millis();
  // Ping local IP
  while(attempt < MAX_PING_RETRIES){
         if((millis() - millisTarefa1) < WAITING_DELAY){
            Serial.print(".");
         }
         if((millis() - millisTarefa1) > WAITING_DELAY){
            if(Ping.ping(WiFi.gatewayIP()) > 0){
               Serial.println("");
               Serial.println("Ping no Roteador OK");
               ROT = true;
               break;
            }
            else{
               Serial.println("");
               Serial.println("Ping no Roteador Error !");
               attempt++;
               valuePingRot++;
               EEPROM.write(addressPingRot,valuePingRot);
               EEPROM.commit();
               if(attempt >= MAX_PING_RETRIES){
                  digitalWrite(Led, LOW);
                  valueROT++;
                  EEPROM.write(addressROT,valueROT);
                  EEPROM.commit();
                  desliga_();
                  delay(6000);
                  liga_();
                  ROT = false; 
               }
               millisTarefa1 = millis();
            }    
         }
   }

   attempt = 0;
   millisTarefa1 = millis();
   
   // Ping Host
   while(attempt < MAX_PING_RETRIES && ROT == true){
        if((millis() - millisTarefa1) < WAITING_DELAY){
           Serial.print(".");
        }
        if((millis() - millisTarefa1) > WAITING_DELAY){
           if(Ping.ping("www.google.com") > 0){
             Serial.println("");
             Serial.println("Ping na Internet OK");
             //Serial.print("MacAddress  - ");Serial.print(WiFi.macAddress());  Serial.println();
             Serial.print("Ping Net  - ");Serial.print(valuePingNet);  Serial.println();
             Serial.print("Ping Rot  - ");Serial.print(valuePingRot);  Serial.println();
             Serial.print("Reset ROT - ");Serial.print(valueROT);  Serial.println();
             Serial.print("Reset ESP - ");Serial.print(valueESP);  Serial.println();
             EnviarParaServer();
             break;
           }
           else{
             Serial.println("");
             Serial.println("Ping na Internet Error ");
             attempt++;
             valuePingNet++;
             EEPROM.write(addressPingNet,valuePingNet);
            EEPROM.commit();
             millisTarefa1 = millis();
             if(attempt >= MAX_PING_RETRIES){
              digitalWrite(Led, LOW);
              valueESP++;
              EEPROM.write(addressESP,valueESP);
             EEPROM.commit();
              ESP.restart();
             }
           }
        }
   }
}

void EnviarParaServer() {
  if((millis() - millisTarefa2) > SERVER_DELAY) {

    uint8_t mac[6];
    WiFi.macAddress(mac);

    char macAddress[18];
    sprintf(macAddress,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
     Serial.print("MacAddress  - ");Serial.print(macAddress);  Serial.println();
    // Cria uma instância do cliente HTTP
    HTTPClient http;
    WiFiClient client;

    // Define o endereço do servidor local
    http.begin(client, "http://192.168.18.5/Projeto/save_data.php");

    // Adiciona os headers necessários
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Prepara os dados a serem enviados
    String postData = "MacAddress=" + String(macAddress) +
                      "&PingNet=" + String(valuePingNet) +
                      "&PingRot=" + String(valuePingRot) +
                      "&ResetROT=" + String(valueROT) +
                      "&ResetESP=" + String(valueESP);

    // Envia os dados para o servidor
    int httpResponseCode = http.POST(postData);

    // Verifica se a requisição foi bem-sucedida
    if(httpResponseCode > 0) {
      Serial.printf("Resposta do servidor: %d\n", httpResponseCode);
      millisTarefa2 = millis();
      LimpaEEPROM();
    } 
    else {
      Serial.printf("Falha na solicitação HTTP: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Libera os recursos
    http.end();

    delay(5000); // Espere 5 segundos antes de enviar novamente
    millisTarefa2 = millis();
    //LimpaEEPROM();
  }
}

void LimpaEEPROM(){
  valuePingNet = 0;
  valuePingRot = 0;
  valueROT = 0;
  valueESP = 0;
  
  EEPROM.write(addressPingNet, valuePingNet);
  EEPROM.write(addressPingRot, valuePingRot);
  EEPROM.write(addressROT, valueROT);
  EEPROM.write(addressESP, valueESP);
  EEPROM.commit();
}

void desliga_() {//--- Função que desliga o roteador
    digitalWrite(RLY_1, LOW);
    digitalWrite(Led, HIGH);
}

void liga_() { //---Função que liga o roteador
  digitalWrite(RLY_1, HIGH);             
  digitalWrite(Led, LOW); 
}
