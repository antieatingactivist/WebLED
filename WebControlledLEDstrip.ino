#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Adafruit_WS2801.h>

const int range = 170;        //number of LEDs addressed
const int dataPin = 15;       //ws2801 data pin
const int clockPin = 13;      //ws2801 clock pin
const char* bonjourName = "light-strip";  //bonjour name - http://xxxx.local 


Adafruit_WS2801 pipe = Adafruit_WS2801(range, dataPin, clockPin);


const char* ssid = "Can't stop the signal, Mal";
const char* password = "youcanttaketheskyfromme";

ESP8266WebServer server(80);    //start web server on port 80

const int led = 2;              //onboard LED

//------------------------ Web Interface Strings ------------------------------------

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


//---------------------------------------------------------------------------------------

void handleRoot() {
  digitalWrite(led, 1);  //built in LED on


  int Data[512];     //create data buffer
    for(int i=0; i<range*3; i++)  //read eeprom into data buffer
    {
      Data[i] = EEPROM.read(i);
    }

  String boxColor;            //--
  String boxColor2;           //---- Declare strings to assemble LED web browser visualization. Need 3 separate strings to display all LEDs
  String boxColor3;           //--
     for(int i=0; i<range; i++) { 
         int x = i*3;               //separate red, green, and blue pixels.
        if(i < 50) boxColor = boxColor+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;                     //--
        if(i >= 50 && i < 100) boxColor2 = boxColor2+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;       //----assemble pixel data into human readable html/css
        else boxColor3 = boxColor3+colorBoxStart+String(Data[x])+","+String(Data[x+1])+","+String(Data[x+2])+colorBoxEnd;                         //--

     }
 
  
  


  server.send(200, "text/html", postForms+boxColor+boxColor2+boxColor3+htmlEnd);  //--serve web page to client

  digitalWrite(led, 0); //built in LED off

}










void handleForm() {
  if (server.method() != HTTP_POST) {
    digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");   //handle server error
    digitalWrite(led, 0);
  } 
  
  
  
  else {
    digitalWrite(led, 1);
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {     //read and parse web form
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      int red = server.arg(0).toInt();      //converts red value (0-255) to integer
      int green = server.arg(1).toInt();   //converts green value (0-255) to integer
      int blue = server.arg(2).toInt();   //converts blue value (0-255) to integer
      int from = server.arg(3).toInt()-1;   //start pixel
      if (from > range) from = range;       //handles invalid range
      int to = server.arg(4).toInt();       //end pixel
      if (to > range) to = range;           //handles invalid range

             
         for(int i=from; i<to; i++) {     //prepares current LED color config for saving to eeprom


             int x = (i)*3;   //arranges red, green, blue values to be written serially
             EEPROM.write(x, red);
             EEPROM.write(x+1, green);
             EEPROM.write(x+2, blue);            
             pipe.setPixelColor(i, Color(red, green, blue));    //write data to addressable LED strip
             pipe.show();                                       //
   
         }


    
    
    }
    EEPROM.commit();
    server.send(200, "text/html", postForms);
    digitalWrite(led, 0);
  }

}



//------------handles bad server request

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

//-------------------------


//----------read eeprom and write to LED strip on boot


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

//-------------------------------

void setup() {
  pipe.begin();                   //initialize LED strip
  EEPROM.begin(512);              //start EEPROM
  recoverOnBoot();                //pull previous config from eeprom and write to LED strip
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.begin(ssid, password);     //start WiFi service
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

  if (MDNS.begin(bonjourName)) {            //start Bonjour service
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);               //http://xxxxx.local/
  server.on("/postform/", handleForm);      //http://xxxxx.local/postform/
  server.onNotFound(handleNotFound);        //bad directory

  server.begin();
  Serial.println("HTTP server started");


}



//--combine separate RGB values into 24 bit integer

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

//------

void loop() {

  server.handleClient();

}
