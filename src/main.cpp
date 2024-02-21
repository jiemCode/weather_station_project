// FJ5792QPATERL4Q2EZVNQCZD

// +15169287064 phone_number
// ACcfe94d0de2dee53812dfdb153782433b account_sid
// 0843ab9662d13ecdd9d53cafff92ad38 auth_token

#include <Arduino.h>


// Import required libraries
// #include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MQ135.h>
#include <ESP_Mail_Client.h>

boolean T_status = true;
boolean H_status = true;
boolean A_status = true;

// Replace with your network credentials
const char* ssid = "Reach";
const char* password = "pass117#";

#define DHTPIN 5     // Digital pin connected to the DHT sensor
#define PIN_MQ135 35

#define temp_threshold 37
#define hum_threshold 30
#define aq_threshold 50

float temperature_v = 0;
float humidity_v = 0;
float air_quality_v = 0;


// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11


DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// -----------------------------------------
#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "dmax6176@gmail.com"

#define sender_password "pwpwimilvqigjteh"

#define Recipient_email "diop.maguette@uam.edu.sn"

#define Recipient_name ""

SMTPSession smtp;


// ---------------------------



String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readMQ135() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  MQ135 mq135_sensor = MQ135(PIN_MQ135);
  float mq = mq135_sensor.getPPM(); 
  if (isnan(mq)) {
    Serial.println("Failed to read from MQ135 sensor!");
    return "--";
  }
  else {
    Serial.println(mq);
    return String(mq);
  }
}


// Replaces placeholder with DHT values
String processor(const String& var){
  if (var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "AIR_QUALITY"){
    return readMQ135();
  }
  return String();
}

void send_mail_alert(String msg_subject, String msg_content){

  ESP_Mail_Session session;

  session.server.host_name = SMTP_server ;

  session.server.port = SMTP_Port;

  session.login.email = sender_email;

  session.login.password = sender_password;

  session.login.user_domain = "";

  /* Declare the message class */

  SMTP_Message message;

  message.sender.name = "";

  message.sender.email = sender_email;

  message.subject = msg_subject;

  message.addRecipient(Recipient_name,Recipient_email);

   //Send HTML message

  String htmlMsg = msg_content;

  message.html.content = htmlMsg.c_str();

  message.text.charSet = "us-ascii";

  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;


  if (!smtp.connect(&session))

    return;
  
  if (!MailClient.sendMail(&smtp, &message))

    Serial.println("Error sending Email, " + smtp.errorReason());
 
}

void setup(){

  // Serial port for debugging purposes
  Serial.begin(115200);
  while (!Serial){}

  if(!SPIFFS.begin(true))
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }



  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  // Print ESP32 Local IP Address
  Serial.print("IP Address : ");
  Serial.print(WiFi.localIP());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  server.serveStatic("/", SPIFFS, "/");
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/air_quality", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readMQ135().c_str());
  });

  // Start server
  server.begin();
  // Serial.println("Follow http://" + WiFi.localIP());

  Serial.begin(115200);
  Serial.println("");

  // smtp.debug(1);

 
}
 
void loop(){
  temperature_v = dht.readTemperature();

  if (T_status==true && !isnan(temperature_v) && temperature_v > temp_threshold){
    send_mail_alert("Alerte T", "Temperature elevee !");
    T_status = false;
  }
  humidity_v=dht.readHumidity();
  if (H_status==true && !isnan(humidity_v) && humidity_v > hum_threshold){
    send_mail_alert("Alerte H", "Basse !");
    H_status=false;
  }
  MQ135 air_quality_f = MQ135(PIN_MQ135);
  air_quality_v = air_quality_f.getPPM(); 
  if (A_status==true && !isnan(air_quality_v) && air_quality_v > aq_threshold){
    send_mail_alert("Alerte AQ", "Mauvaise qualite de l'air!");
    A_status=false;
  }
  delay(500000);
  T_status = true;
}