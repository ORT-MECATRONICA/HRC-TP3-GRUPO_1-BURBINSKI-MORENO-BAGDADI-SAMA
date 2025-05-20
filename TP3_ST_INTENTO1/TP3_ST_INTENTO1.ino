// GRUPO 3 - SAMA BAGDADI CHAMES DAGOTTO - EJERCICIO 2

#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <U8g2lib.h>
#include <WiFi.h>

#define PANTALLA1 1
#define PASO1_CAMBIO_A_P2 2
#define PASO2_CAMBIO_A_P2 3
#define PASO3_CAMBIO_A_P2 4
#define PANTALLA2 5
#define ESTADO_CONFIRMACION2 6
#define SUBIR_UMBRAL 7
#define BAJAR_UMBRAL 8

#define DHTPIN 23      // pin del dht11
#define DHTTYPE DHT11  // tipo de dht (hay otros)

DHT dht(DHTPIN, DHTTYPE);

//Botones
#define PIN_BOTON1 34
#define PIN_BOTON2 35
#define PULSADO LOW
#define N_PULSADO !PULSADO

const int CAMBIO_UMBRAL = 1;

int umbralDeTemperatura = 0;
bool cambioHecho = LOW;
float temperatura = 0;

const char* SSID = "ORT-IoT";
const char* PASSWORD = "NuevaIOT$25";
const char* NTP_SERVER = "pool.ntp.org";

int estadoActual = PANTALLA1;  // inicia en PANTALLA1

const long INTERVALO = 1000;
const long TIEMPO_MAX_ESPERA_BOTON = 5000;
unsigned long tiempoAhora = millis();
unsigned long tiempoUltimaImpresion = 0;
unsigned long tiempoUltimoBotonPresionado = 0;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);  //inicializa la pantallita


void setup() {
  Serial.begin(115200);

  pinMode(PIN_BOTON1, INPUT);
  pinMode(PIN_BOTON2, INPUT);

  dht.begin();   // inicializo el dht
  u8g2.begin();  //inicializo la pantallita

  WiFi.begin(SSID, PASSWORD);
  Serial.println("Conectando a wifi");
  tiempoUltimaImpresion = millis();
  while (WiFi.status() != WL_CONNECTED) {
    tiempoAhora = millis();
    if (tiempoAhora - tiempoUltimaImpresion >= INTERVALO) {
      tiempoUltimaImpresion = millis();
      Serial.println("Intentando conectar...");
    }
  }
  Serial.println("Conexión exitosa");

}


void loop() {
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
        sprintf(bufferUmbral, "%d", UmbralDeTemperatura);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(10, 10, "Temperatura");
        u8g2.drawStr(10, 25, bufferTemperatura);
        u8g2.drawStr(10, 40, "Umbral");
        u8g2.drawStr(10, 55, bufferUmbral);
        u8g2.sendBuffer();
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
        if (tiempoAhora - tiempoUltimoBotonPresionado >= TIEMPO_MAX_ESPERA_BOTON){
          estadoActual = PANTALLA1;
        }
      }
      break;

    case PASO2_CAMBIO_A_P2:
      if (lecturaBoton2 == N_PULSADO) {
        if (lecturaBoton1 == PULSADO) {
          estadoActual = PASO3_CAMBIO_A_P2;
        }
        if (tiempoAhora - tiempoUltimoBotonPresionado >= TIEMPO_MAX_ESPERA_BOTON){
          estadoActual = PANTALLA1;
        }
      }
      break;

    case PASO3_CAMBIO_A_P2:
      if (lecturaBoton1 == N_PULSADO) {
          estadoActual = PANTALLA2;
        }
      }
      break;



    case PANTALLA2:
      if (tiempoAhora - tiempoUltimaImpresion >= INTERVALO)  ///delay sin bloqueo
      {
        tiempoUltimaImpresion = millis();  /// importante actualizar el tiempo
        char bufferUmbral[5];

        sprintf(bufferUmbral, "%d", UmbralDeTemperatura);

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
        //Serial.println(cambioHecho);
      }
      if (lecturaBoton2 == PULSADO) {
        cambioHecho = HIGH;
        estadoActual = SUBIR_UMBRAL;
        //Serial.println(cambioHecho);
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
          UmbralDeTemperatura = UmbralDeTemperatura + CAMBIO_UMBRAL;
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
          UmbralDeTemperatura = UmbralDeTemperatura - CAMBIO_UMBRAL;
          cambioHecho = LOW;
        }
        estadoActual = PANTALLA2;
      }
      break;
  }
}