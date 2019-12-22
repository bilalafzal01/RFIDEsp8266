#define SS_PIN D4  
#define RST_PIN D2  

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid     = "Bilal Afzal";
const char* password = "12345678";

ESP8266WebServer server(80);

// Current time
unsigned long currentTime = millis();
unsigned long newTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

String content= "";

#define ERROR_PIN D0
#define SUCCESS_PIN D1
#define CONN_PIN D8

MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() 
{
  Serial.begin(115200);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

   pinMode(CONN_PIN, OUTPUT);
   pinMode(SUCCESS_PIN, OUTPUT);
   pinMode(ERROR_PIN, OUTPUT);
   
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  digitalWrite(CONN_PIN, HIGH);
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

String prepareHtmlPage(String content,unsigned long newTime)
{
  String htmlPage =
            String("<!DOCTYPE HTML>") +
            "<html style=\"background: black\"><head><meta charset=\"UTF-8\">" +
            "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">";
       htmlPage +="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
       htmlPage +="<script>\n";
       htmlPage +="setInterval(loadDoc,200);\n";
       htmlPage +="function loadDoc() {\n";
       htmlPage +="var xhttp = new XMLHttpRequest();\n";
       htmlPage +="xhttp.onreadystatechange = function() {\n";
       htmlPage +="if (this.readyState == 4 && this.status == 200) {\n";
       htmlPage +="document.getElementById(\"data\").innerHTML =this.responseText}\n";
       htmlPage +="};\n";
       htmlPage +="xhttp.open(\"GET\", \"/\", true);\n";
       htmlPage +="xhttp.send();\n";
       htmlPage +="}\n";
       htmlPage +="</script>\n";
       htmlPage +="</head>" +
       htmlPage +="<div style=\"color:white; text-align:center border\"id=\"data\"><h3  style=\"font-family:monaco;\"><ul>Card id:  " + content + "<br>";
       if(content.substring(1) == "10 12 23 A4"){
       digitalWrite(SUCCESS_PIN, HIGH);
       digitalWrite(ERROR_PIN, LOW);
        htmlPage += "<h2>Access Granted!</h2>\n";
        htmlPage += "<h2>"+String(newTime)+"</h2>\n";
       }else{
        htmlPage += "<h2>Access Denied!</h2>\n";        
       digitalWrite(SUCCESS_PIN, LOW);
       digitalWrite(ERROR_PIN, HIGH);
       }
       htmlPage +="\n";
       htmlPage +="</ul></h3></div></html>";
  return htmlPage;
}

void loop() 
{
 
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor

  
  Serial.println();
  Serial.print(" UID tag :");
  content= "";  
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();

  server.handleClient();
//  if (content.substring(1) == "10 12 23 A4") //change UID of the card that you want to give access
//  {
//    Serial.println(" Access Granted ");
//    Serial.println(" Welcome Mr.Circuit ");
//    delay(1000);
//    Serial.println(" Have FUN ");
//    Serial.println();
////    statuss = 1;
//  }
//  
//  else   {
//    Serial.println(" Access Denied ");
//    delay(3000);
//  }
}

void handle_OnConnect() {
  Serial.println("[Client handled]");
  newTime = millis();
  server.send(200, "text/html", prepareHtmlPage(content, newTime));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
