/*
  Pico W Web Interface Demo
  picow-web-control-demo.ino
  Web Interface & WiFi Connection
  Control the onboard LED with Pico W
 
  Adapted from ESP32 example by Rui Santos - https://randomnerdtutorials.com
 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Load Wi-Fi and ESP Mail Client library
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include "Credentials.h"

const int sensor_pin = 28;
const int button_pin = 15; 

SMTPSession smtp;

Session_Config config;

void smtpCallback(SMTP_Status status);

void setup(){

  pinMode(button_pin, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println();
  

  // Set network reconnection option
  MailClient.networkReconnect(true);

  // Enable debug
  smtp.debug(1);

  smtp.callback(smtpCallback);

  // Set the session config
  // Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  // Set the NTP Config time
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;
}

void loop(){

  Serial.println(get_moisture_reading());
  //Serial.println(analogRead(sensor_pin));
  delay(1000);

  /*if (digitalRead(button_pin) == LOW){
    digitalWrite(LED_BUILTIN, HIGH);
    connect_to_wifi();
    send_email(get_moisture_reading());
    //send_email("LIGMA BALLS!!!!");
    WiFi.disconnect();
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }*/
}

String get_moisture_reading(){

  float sensor_value = 0; // Hold the value of the sensor
  
  sensor_value = 100 - (float(analogRead(sensor_pin) - wettest_value) / float(dryest_value - wettest_value) * 100);

  

  return String(int(sensor_value));
}

void connect_to_wifi(){
  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);
 
  // Start WiFi with supplied parameters
  WiFi.begin(SSID, NETWORK_PASSWORD);
 
  // Print periods on monitor while establishing connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
 
  // Connection established
  Serial.println("");
  Serial.print("Pico W is connected to WiFi network ");
  Serial.println(WiFi.SSID());
 
  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());
}

void send_email(String moisture_reading){
  // Declare the message class
  SMTP_Message message;

  // Set the message headers
  message.sender.name = F("Pepper Monitor");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Pi Pico Test Email");
  message.addRecipient(F("Pepper Grower"), RECIPIENT_EMAIL);

  // Send raw text message
  String textMsg = String("Moisture: " + moisture_reading + "%");
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // Connect to the server
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, status code: %d, Error Code: %d, Reason %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
} 

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
