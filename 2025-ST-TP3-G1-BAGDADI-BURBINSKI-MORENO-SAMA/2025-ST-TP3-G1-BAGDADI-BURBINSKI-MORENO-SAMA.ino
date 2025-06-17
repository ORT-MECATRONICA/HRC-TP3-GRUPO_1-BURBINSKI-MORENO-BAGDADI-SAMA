// GRUPO 1 - BAGDADI BURBINSKI, MORENO, SAMA - 5B

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <I2C_eeprom.h>
#include "Wire.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <U8g2lib.h>
#include <WiFi.h>

const char* ssid = "MECA-IoT";
const char* password = "IoT$2025";

#define PANTALLA1 1
#define PASO1_CAMBIO_A_P2 2
#define PASO2_CAMBIO_A_P2 3
#define PASO3_CAMBIO_A_P2 4
#define PANTALLA2 5
#define ESTADO_CONFIRMACION2 6
#define SUBIR_UMBRAL 7
#define BAJAR_UMBRAL 8

#define DHTPIN 23
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC256);
//***************************************************************************************

#define BOTtoken "7816717541:AAGefH9MSn0YQuwKzNlP30JBH1dP3DxZKQM"
#define CHAT_ID "6750729046"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000;



//***************************************************************************************

TaskHandle_t LoopPantallas;
TaskHandle_t LoopBotTelegram;

#define PIN_BOTON1 34
#define PIN_BOTON2 35
#define PULSADO LOW
#define N_PULSADO !PULSADO
#define PIN_LED 25

const int CAMBIO_UMBRAL = 1;

float umbralTemperatura = 23.0;
bool cambioHecho = LOW;
float temperatura = 0;
bool alertaEnviada = false;

int estadoActual = PANTALLA1;

const long INTERVALO = 1000;
const long TIEMPO_MAX_ESPERA_BOTON = 5000;
unsigned long tiempoAhora = millis();
unsigned long tiempoUltimaImpresion = 0;
unsigned long tiempoUltimoBotonPresionado = 0;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//***************************************************************************************


void setup() {
  Serial.begin(115200);

  Wire.begin();
  ee.begin();

  pinMode(PIN_BOTON1, INPUT);
  pinMode(PIN_BOTON2, INPUT);
  pinMode(PIN_LED, OUTPUT);

  dht.begin();
  u8g2.begin();

  //***************************************************************************************

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  //***************************************************************************************

#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif

  xTaskCreatePinnedToCore(LoopPantallascode, "Ejecutar máquina de estados", 10000, NULL, 1, &LoopPantallas, 0);
  xTaskCreatePinnedToCore(LoopBotTelegramcode, "Correr Bot de Telegram", 10000, NULL, 1, &LoopBotTelegram, 1);
}


// ########### - PANTALLAS - ###############################################################################


void LoopPantallascode(void* pvParameters) {

  for (;;) {
    tiempoAhora = millis();
    temperatura = dht.readTemperature();

    bool lecturaBoton1 = digitalRead(PIN_BOTON1);
    bool lecturaBoton2 = digitalRead(PIN_BOTON2);

    switch (estadoActual) {
      case PANTALLA1:
        if (tiempoAhora - tiempoUltimaImpresion >= INTERVALO) {
          tiempoUltimaImpresion = millis();
          char bufferTemperatura[5];
          char bufferUmbral[5];
          sprintf(bufferTemperatura, "%.2f", temperatura);
          sprintf(bufferUmbral, "%.1f", umbralTemperatura);

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tf);
          u8g2.drawStr(10, 10, "Temperatura");
          u8g2.drawStr(10, 25, bufferTemperatura);
          u8g2.drawStr(10, 40, "Umbral");
          u8g2.drawStr(10, 55, bufferUmbral);
          u8g2.sendBuffer();
        }

        if (temperatura > umbralTemperatura) {
          digitalWrite(PIN_LED, HIGH);
        } else {
          digitalWrite(PIN_LED, LOW);
        }

        if (lecturaBoton1 == PULSADO) {
          tiempoUltimoBotonPresionado = millis();
          estadoActual = PASO1_CAMBIO_A_P2;
        }
        break;

      case PASO1_CAMBIO_A_P2:
        if (lecturaBoton1 == N_PULSADO) {
          if (lecturaBoton2 == PULSADO) {
            estadoActual = PASO2_CAMBIO_A_P2;
          }
          if (tiempoAhora - tiempoUltimoBotonPresionado >= TIEMPO_MAX_ESPERA_BOTON) {
            estadoActual = PANTALLA1;
          }
        }
        break;

      case PASO2_CAMBIO_A_P2:
        if (lecturaBoton2 == N_PULSADO) {
          if (lecturaBoton1 == PULSADO) {
            estadoActual = PASO3_CAMBIO_A_P2;
          }
          if (tiempoAhora - tiempoUltimoBotonPresionado >= TIEMPO_MAX_ESPERA_BOTON) {
            estadoActual = PANTALLA1;
          }
        }
        break;

      case PASO3_CAMBIO_A_P2:
        if (lecturaBoton1 == N_PULSADO) {
          estadoActual = PANTALLA2;
        }
        break;

      case PANTALLA2:
        if (tiempoAhora - tiempoUltimaImpresion >= INTERVALO) {
          tiempoUltimaImpresion = millis();
          char bufferUmbral[5];
          sprintf(bufferUmbral, "%.1f", umbralTemperatura);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tf);
          u8g2.drawStr(5, 5, "Umbral:");
          u8g2.drawStr(5, 25, bufferUmbral);
          u8g2.sendBuffer();
        }

        if (lecturaBoton1 == PULSADO && lecturaBoton2 == PULSADO) {
          estadoActual = ESTADO_CONFIRMACION2;
        }

        if (lecturaBoton1 == PULSADO) {
          cambioHecho = HIGH;
          estadoActual = BAJAR_UMBRAL;
        }
        if (lecturaBoton2 == PULSADO) {
          cambioHecho = HIGH;
          estadoActual = SUBIR_UMBRAL;
        }
        break;

      case ESTADO_CONFIRMACION2:
        if (lecturaBoton1 == N_PULSADO && lecturaBoton2 == N_PULSADO) {
          estadoActual = PANTALLA1;
        }
        break;

      case SUBIR_UMBRAL:
        if (lecturaBoton1 == PULSADO) {
          estadoActual = ESTADO_CONFIRMACION2;
        }
        if (lecturaBoton2 == N_PULSADO) {
          if (cambioHecho == HIGH) {
            umbralTemperatura += CAMBIO_UMBRAL;
            cambioHecho = LOW;
          }
          estadoActual = PANTALLA2;
        }
        break;

      case BAJAR_UMBRAL:
        if (lecturaBoton2 == PULSADO) {
          estadoActual = ESTADO_CONFIRMACION2;
        }
        if (lecturaBoton1 == N_PULSADO) {
          if (cambioHecho == HIGH) {
            umbralTemperatura -= CAMBIO_UMBRAL;
            cambioHecho = LOW;
          }
          estadoActual = PANTALLA2;
        }
        break;
    }
  }
}

//***************************************************************************************


void LoopBotTelegramcode(void* pvParameters) {
  for (;;) {
    if (millis() - lastTimeBotRan > botRequestDelay) {
      int numMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numMessages) {
        numMessages = bot.getUpdates(bot.last_message_received + 1);

        handleMessages(numMessages);
      }
      lastTimeBotRan = millis();

      //.....
    }
    if (temperatura > umbralTemperatura && alertaEnviada == false) {
      String warningTemp = "La temperatura actual superó el umbral (";
      warningTemp += umbralTemperatura;
      warningTemp += ")";
      bot.sendMessage(CHAT_ID, warningTemp, "");
      alertaEnviada = true;
    }

    if (temperatura <= umbralTemperatura) {
      alertaEnviada = false;
    }
  }
  delay(100);
}


void handleMessages(int numMessages) {
  for (int i = 0; i < numMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Bienvenido, " + from_name + ".\n";
      welcome += "Usa los siguientes comandos para interactuar:\n";
      welcome += "/enviarTemperatura - Recibir la tempertaura actual\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/enviarTemperatura") {
      String sendTemp = "La temperatura actual es: ";
      sendTemp += temperatura;
      bot.sendMessage(chat_id, sendTemp, "");
    }
  }
}

void loop() {}
