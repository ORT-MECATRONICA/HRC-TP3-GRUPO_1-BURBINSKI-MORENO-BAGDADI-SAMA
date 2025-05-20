#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <I2C_eeprom.h>
#include "Wire.h"

// Este bloque se deja como está
I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC256);

// Credenciales de WiFi
const char* ssid = "ORT-IoT";
const char* password = "NuevaIOT$25";

// Inicializar el bot de Telegram
#define BOTtoken "7737430934:AAF3215UFmRcJwk1K10o2GaRWij5BwGHjUM"  // Token del bot
#define CHAT_ID "-4601150708"                                      // ID del chat donde se recibirán los mensajes

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Configuración del bot
unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000;

// Función para manejar los mensajes nuevos
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
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
      welcome += "/add nombre cantidad - Agregar un nuevo alimento\n";
      welcome += "/list - Listar todos los alimentos registrados\n";
      welcome += "/remove nombre cantidad - Restar una cantidad o eliminar un alimento\n";
      welcome += "/buscar nombre - Buscar la cantidad de un alimento específico\n";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();  // Inicialización I2C
  ee.begin();    // Inicialización del manejador de la memoria


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conectado a WiFi con IP:");
  Serial.println(WiFi.localIP());

#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif
}

void loop() {

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
