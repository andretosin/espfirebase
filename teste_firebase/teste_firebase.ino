#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Constants
#define WIFI_SSID "Casa Sala 2.4G"
#define WIFI_PASSWORD "7621287700"
#define API_KEY "AIzaSyCrUhGtkVIyjrHAWxjEw0BCM84663sSRhw"
#define DATABASE_URL "https://esp32-botao-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int counter = 0;
bool signupOK = false;

int outputPin = 23;
int buttonPin = 4;
bool lastReading = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
bool toggleState = false;
bool state = false;
bool lastState = false;



void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  setupWiFi();

  pinMode(outputPin, OUTPUT);


  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

//if (Firebase.ready() && signupOK){
//    if (Firebase.RTDB.setInt(&fbdo, "counter", counter))
//    Serial.println("Gravou " + String(counter));
// }

void readButtonState() {
int reading = digitalRead(buttonPin);
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == 1 && lastReading == 0) {
      toggleState = !toggleState;
      if (Firebase.RTDB.setInt(&fbdo, "state", toggleState)) {
        Serial.println(">>" + String(toggleState));
      } else {
        Serial.println("Erro: " + fbdo.errorReason());
      }
    } 
    if (reading == 0 && lastReading == 1) {
      lastDebounceTime = millis();
    } 
  lastReading = reading;
  }
}

void readValueFromFirebase() {
  if (Firebase.RTDB.getBool(&fbdo, "state")) {
    if (fbdo.dataType() == "boolean") {
      state = fbdo.boolData();
      if (state != lastState) {
        digitalWrite(outputPin, state);
        lastState = state;
      }
    }  
  } else {
    Serial.println(fbdo.errorReason());
  }
}

void loop(){
  readButtonState();
  readValueFromFirebase();
}