/**
 * Created by Adesola Samuel
 * 
 * Email:adesolasamuel2018@gmail.c0m
 * 
 * Github: https://github.com/adesolasamuel/ESP8266-NodeMCU-to-Google-Firebase
 * 
 * Copyright (c) 2022 samuel
 *
*/
#if defined(ESP32)
  #include <WiFi.h>
  #include <FirebaseESP32.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <FirebaseESP8266.h>
#endif
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

/************************* Includes required for FireBase *********************************/

//Provide the token generation process info.
#include <addons/TokenHelper.h>  

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define Temperature_PATH "/Boards/123456/Temperature"
#define Humidity_PATH "/Boards/123456/Humidity"
#define R1_PATH "/Boards/123456/R1"
#define R2_PATH "/Boards/123456/R2"
#define R3_PATH "/Boards/123456/R3"
#define R4_PATH "/Boards/123456/R4"

/************************* Macros For Pin Definition ******************************/
#define RELAY1           D8
#define RELAY2           D7
#define RELAY3           D6
#define RELAY4           D5
#define BUZZER           D0

/************************* DHT Sensor Setup *********************************/
#define DHTPIN            D4     // Digital pin connected to the DHT sensor
#define DHTTYPE           DHT11  // Type of DHT Sensor, DHT11 or DHT22
DHT dht(DHTPIN, DHTTYPE);
/************************* WiFi Access Point ************************************/
/* 1. Define the WiFi credentials */
#define WIFI_SSID "El3ezaby"
#define WIFI_PASSWORD "10000000@#"
//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAi0J-UZQTFt-Ws0B2myMPpbyiN_KE9yCE"//AIzaSyAdbz40mouZhn0DMhzn9V-OkjiCABvv4kA

/* 3. Define the RTDB URL */
#define DATABASE_URL "loginforms-c623a-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "a7med3ali080@gmail.com"
#define USER_PASSWORD "10000000@#"
/************************* LCD   object creation *********************************/
LiquidCrystal_I2C lcd(0x27, 16, 2);     //SCL 22 SDA 21 

/**************Define Firebase Data object *************************************/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis=0;
void ControlRelays(char*Relayvalus );
void setup()
{ 
  /**************************** Serial begin **************************************/
   Serial.begin(115200);
  /**************************WaterFlow Sensor Setup******************************/
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    pinMode(BUZZER, OUTPUT);

  /*************************** LCD Init ***************************************/
    Wire.begin();
    lcd.init();     // initialize LCD                 
    lcd.backlight();  // turn on Backlight
    lcd.setCursor(0, 0);
    lcd.print("Smart Home System");
  /************** Wifi Init and FireBase Connect******************************/
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }
    /********************* Printing wifi Data on Serial Port *******************/
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

  //Serial.printf("Firebase Client v%s\n\n", FIREBASE_STREAM_SESSION_H);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";
  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino
  //////////////////////////////////////////////////////////////////////////////////////////////
  //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  //otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////
  Firebase.begin(&config, &auth);
  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  digitalWrite(BUZZER, LOW);
  delay(500);

}
void loop()
 {
    /**************************************DHT11 Sensor Data*************************************************************/
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    h = h ;
    if (t>34)
    {
      digitalWrite(BUZZER, HIGH);
      delay(500); 
    }
    else 
    {
      digitalWrite(BUZZER, LOW);
      delay(500);
    }
      
    Serial.println("Humidity : " + (String)h + ".... Temperature= " + (String)t);
    lcd.setCursor(1, 1);
    lcd.print("H:" + (String)h + "%...T:" + (String)t + "°C");
    /**************************************** Dealing with Firebase *****************************************************/
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000) || (sendDataPrevMillis == 0)) {

        sendDataPrevMillis = millis();

        Serial.printf("Set Temperature... %s\n", Firebase.setFloat(fbdo, F(Temperature_PATH), t) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Set Humidity... %s\n", Firebase.setFloat(fbdo, F(Humidity_PATH), h) ? "ok" : fbdo.errorReason().c_str());

        /**********************************************Get string value of R1 from Firebase***********************************************/
        if (Firebase.getString(fbdo, R1_PATH)) {
            if (fbdo.stringData()) {
                Serial.print("R1 value  is :");
                
                String R1 = fbdo.stringData();
                R1.trim(); // Remove leading and trailing whitespace
                R1.replace("\"", ""); // Remove the quotes
                R1.replace("\\", ""); // Remove the backslashes
                int R1Value = R1.toInt(); // Co
                Serial.println(R1);
                if(R1Value==0)
                {
                  digitalWrite(RELAY1, LOW);  // Convert String to const char* using c_str() method
                  //Serial.print("R1 ON");

                }
                else if (R1Value==1)
                {          
                  digitalWrite(RELAY1, HIGH);
                  //Serial.print("R1 OFF");
                }
            } else {
                Serial.println("Failed to get string value from Firebase");
            }
        } else {
            Serial.println("Error fetching data from Firebase");
            Serial.println(fbdo.errorReason());
        }
        /**********************************************Get string value of R2 from Firebase***********************************************/
        if (Firebase.getString(fbdo, R2_PATH)) {
            if (fbdo.stringData()) {
                Serial.print("R2 value  is :");
                String R2 = fbdo.stringData();
                R2.trim(); // Remove leading and trailing whitespace
                R2.replace("\"", ""); // Remove the quotes
                R2.replace("\\", ""); // Remove the backslashes
                int R2Value = R2.toInt(); // Co
                Serial.println(R2);
                if(R2Value==0)
                  digitalWrite(RELAY2, LOW);  // Convert String to const char* using c_str() method
                else if (R2Value==1)
                  digitalWrite(RELAY2, HIGH);  // Convert String to const char* using c_str() method

            } else {
                Serial.println("Failed to get string value from Firebase");
            }
        } else {
            Serial.println("Error fetching data from Firebase");
            Serial.println(fbdo.errorReason());
        }

        /**********************************************Get string value of R3 from Firebase***********************************************/
        if (Firebase.getString(fbdo, R3_PATH)) {
            if (fbdo.stringData()) {
                Serial.print("R3 value  is :");
                String R3 = fbdo.stringData();
                R3.trim(); // Remove leading and trailing whitespace
                R3.replace("\"", ""); // Remove the quotes
                R3.replace("\\", ""); // Remove the backslashes
                int R3Value = R3.toInt(); // Co
                Serial.println(R3);
                if(R3Value==0)
                  digitalWrite(RELAY3, LOW);  // Convert String to const char* using c_str() method
                else if (R3Value==1)
                  digitalWrite(RELAY3, HIGH);
            } else {
                Serial.println("Failed to get string value from Firebase");
            }
        } else {
            Serial.println("Error fetching data from Firebase");
            Serial.println(fbdo.errorReason());
        }

        /**********************************************Get string value of R4 from Firebase***********************************************/
        if (Firebase.getString(fbdo, R4_PATH)) {
            if (fbdo.stringData()) {
                Serial.print("R4 value  is :");
                String R4 = fbdo.stringData();
                R4.trim(); // Remove leading and trailing whitespace
                R4.replace("\"", ""); // Remove the quotes
                R4.replace("\\", ""); // Remove the backslashes
                int R4Value = R4.toInt(); // Co
                Serial.println(R4);
                if(R4Value==0)
                  digitalWrite(RELAY4, LOW);  // Convert String to const char* using c_str() method
                else if (R4Value==1)
                  digitalWrite(RELAY4, HIGH);
            } else {
                Serial.println("Failed to get string value from Firebase");
            }
        } else {
            Serial.println("Error fetching data from Firebase");
            Serial.println(fbdo.errorReason());
        }
    }
}

