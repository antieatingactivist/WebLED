#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Adafruit_WS2801.h>

const int range = 170;


Adafruit_WS2801 pipe = Adafruit_WS2801(range, 15, 13);

const char* ssid = "Can't stop the signal, Mal";
const char* password = "youcanttaketheskyfromme";

ESP8266WebServer server(80);

const int led = 2;


const String postForms = "<html>\
  <head>\
    <title>Color Picker</title>\
    <style>\
      body { background-color: #111111; font-family: Arial, Helvetica, Sans-Serif; Color: #cccccc; }\
    </style>\
  </head>\
  <body>\
    <h1>Color Picker</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      Red <input type=\"text\" name=\"red\" value=\"\"><br>\
      Green <input type=\"text\" name=\"green\" value=\"\"><br>\
      Blue <input type=\"text\" name=\"blue\" value=\"\"><br><br>\
      Range <input type=\"text\" name=\"start\" value=\"\"> - <input type=\"text\" name=\"end\" value=\"\"><br>\
     <input type=\"submit\" value=\"Submit\">\
    </form>";



const String colorBoxStart = "<svg width=\"12\" height=\"30\">\
<rect width=\"20\" height=\"20\" style=\"fill:rgb(";

const String colorBoxEnd = ");stroke-width:3;stroke:rgb(0,0,0)\" />\
</svg>";



const String htmlEnd = " </body>\
</html>";

void handleRoot() {
  digitalWrite(led, 1);


int Data[512];
    for(int i=0; i<range*3; i++) {
      Data[i] = EEPROM.read(i);
      
    }
  String boxColor;
  String boxColor2;
    String boxColor3;
     for(int i=0; i<range; i++) { 
         int x = i*3;
        if(i < 50) boxColor = boxColor+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;
        if(i >= 50 && i < 100) boxColor2 = boxColor2+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;
        else boxColor3 = boxColor3+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;

     }
 
  
  


  server.send(200, "text/html", postForms+boxColor+boxColor2+boxColor3+htmlEnd);

  digitalWrite(led, 0);

}










void handleForm() {
  if (server.method() != HTTP_POST) {
    digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(led, 0);
  } else {
    digitalWrite(led, 1);
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      int red = server.arg(0).toInt();
      int green = server.arg(1).toInt();
      int blue = server.arg(2).toInt();
      int from = server.arg(3).toInt()-1;
      if (from > range) from = range;
      int to = server.arg(4).toInt();  
      if (to > range) to = range;      

             
         for(int i=from; i<to; i++) { 


             int x = (i)*3;
             EEPROM.write(x, red);
             EEPROM.write(x+1, green);
             EEPROM.write(x+2, blue);            
              pipe.setPixelColor(i, Color(red, green, blue));
          
             pipe.show();
   
         }


    
    
    }
    EEPROM.commit();
    server.send(200, "text/html", postForms);
    digitalWrite(led, 0);
  }

}


















void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}



void recoverOnBoot() {





            for(int i=0; i<range; i++) { 


             int x = i*3;

            int red = EEPROM.read(x);
            int green = EEPROM.read(x+1);             
            int blue = EEPROM.read(x+2);             
             



             pipe.setPixelColor(i, Color(red, green, blue));
  
             pipe.show();

         }

    

  
}

void setup(void) {
  pipe.begin();  

  
  EEPROM.begin(512);           
  recoverOnBoot(); 
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("pipe2")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

//  server.on("/postplain/", handlePlain);

  server.on("/postform/", handleForm);
  


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");



    Serial.println(EEPROM.read(0));
        Serial.println(EEPROM.read(1));
            Serial.println(EEPROM.read(2));

}



uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= b;
  c <<= 8;
  c |= g;
  return c;
}

void loop(void) {

  server.handleClient();

}
