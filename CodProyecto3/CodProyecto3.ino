#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> 
#include <Hash.h>
#include <FS.h>

#include<Servo.h>

const char *ssid = "id";
const char *password = "password";
Servo servo;
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

#define echoPin 13 // Echo Pin
#define trigPin 12 // Trigger Pin

 #define sensorCapEcho 0
 #define sensorCapTrig 4
int distanceSens, distanceCap; 
 long porLleno;

 int16_t thisRead=0;
 int16_t lastRead=0;
 uint16_t counter=0;

void setup(void){
  delay(1000);
  
  Serial.begin(115200);
 //................................
 pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);
pinMode(sensorCapTrig, OUTPUT);
pinMode(sensorCapEcho, INPUT);
 servo.attach(16);
  servo.write(0);
  
 //.............................
  WiFi.begin(ssid, password);
  Serial.println("INICIANDO CONEXIÓN A MODEM");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress myIP = WiFi.localIP();
   Serial.println("");
  Serial.print("DIRECCION IP: ");
  Serial.println(myIP);


  SPIFFS.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.begin();
  Serial.println("SERVIDOR WEB INICIADO");
}
long ultimovalor=0;
void loop(void) {
  webSocket.loop();
  server.handleClient();  
  
  
 distanceSens=calculateDistance1();
distanceCap=calculateDistance2();
porLleno=(distanceCap/18.0)*100.0;
Serial.print(distanceSens);
Serial.print("  D Cap:  ");
Serial.print(distanceCap);
Serial.print("  %Cap  ");
Serial.println(porLleno);
if(distanceSens<= 20){
  servo.write(90);
   delay(3000);
   servo.write(0);
  delay(200);
}
 if (porLleno < 0) porLleno = 0;
 if (porLleno != ultimovalor) {
      String msj = String(porLleno);
      webSocket.broadcastTXT(msj);
      Serial.println(msj);
    }
    ultimovalor = porLleno;
  delay(200);
}

//FUNCION PROPIA DEL OBJETO WEBSOCKET
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch(type) {

   
    case WStype_DISCONNECTED: {
      Serial.printf("USUARIO #%u - DESCONECTADO!\n", num);
      break;
    }
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("NUEVA CONEXIÓN. IP: %d.%d.%d.%d NOMBRE: %s ID USUARIO: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);  
 
      String message = String(lastRead);
      webSocket.broadcastTXT(message);
      break;
    }
    case WStype_TEXT: {
      String dato = "";
      for (int i = 0; i < lenght; i++) {
        dato.concat((char)payload[i]);
      }
       Serial.print(dato);
       Serial.println("");
      uint8_t valor = dato.toInt();
      if(valor==1){
        servo.write(90);
      }else{
        servo.write(0);
      }
      
      break;
    }
    
  }

}

// A function we use to get the content type for our HTTP responses
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}


int calculateDistance1()
     { long duration1; int distance1;
  
          digitalWrite(trigPin, LOW); 
          delayMicroseconds(2);
          digitalWrite(trigPin, HIGH); 
          delayMicroseconds(10);
          digitalWrite(trigPin, LOW);
          duration1 = pulseIn(echoPin, HIGH); 
          distance1= duration1/58.2;
          return distance1;
     }

int calculateDistance2()
     { long duration2; int distance2;
  
          digitalWrite(sensorCapTrig, LOW); 
          delayMicroseconds(2);
        
          digitalWrite(sensorCapTrig, HIGH); 
          delayMicroseconds(10);
          digitalWrite(sensorCapTrig, LOW);
          duration2 = pulseIn(sensorCapEcho, HIGH); 
          distance2= duration2/58.2;
          return distance2;
     }
