TaskHandle_t BOT_TELEGRAM;
TaskHandle_t MAQ_ESTADOS;

const int led1 = 34;
const int led2 = 35;

void setup () {
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  xTaskCreatePinnedToCore(
             CORRER_BOT, /* Task function. */
             "botCom",   /* name of task. */
             10000,     /* Stack size of task */
             NULL,      /* parameter of the task */
             1,         /* priority of the task */
             &BOT_TELEGRAM,    /* Task handle to keep track of created task */
             0);   

   xTaskCreatePinnedToCore(
             FUNC,  /* Task function. */
             "maqEstados",    /* name of task. */
             10000,      /* Stack size of task */
             NULL,       /* parameter of the task */
             1,          /* priority of the task */
             &MAQ_ESTADOS,     /* Task handle to keep track of created task */
             1);         /* pin task to core 0 */
}

void CORRER_BOT( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    digitalWrite(led1, HIGH);
    delay(1000);
    digitalWrite(led1, LOW);
    delay(1000);
  }
}

void FUNC( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    digitalWrite(led2, HIGH);
    delay(700);
    digitalWrite(led2, LOW);
    delay(700);
  }
}

void loop() { }


