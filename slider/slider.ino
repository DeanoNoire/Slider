#include <WiFi.h>
#include <WebServer.h>
#include <Stepper.h>

const char* ssid = "Dolly";
const char* password = "Busterbuster";
WebServer server(80);

int homed = 0;
int stepperPozice = 0;
const int stepperPocetKroku = 44;
const int stepperCelaDelka = 61600; // 1400 kroků = celá délka
int stepperSpeed = 725;//50; 
const int stepperHomingSpeed = 800;
const int endstopPin = 10;

Stepper stepper = Stepper(stepperPocetKroku, 11, 13, 12, 14);

void handleRoot() {
    String html = 
            "<html><body>"
        "<h1>ESP32 Web Server</h1>"
        "<button onclick=\"runFunction1()\">Test spin</button>"
        "<br>"
        "<button onclick=\"homing()\">Homing</button>"
        "<br><br><br>"
        "<input type='number' id='numberInput' placeholder='Minutes to full'>"
        "<button onclick=\"sendNumber()\">Potvrdit</button>"
       
        "<script>"
        "function runFunction1() {"
        "    var xhr = new XMLHttpRequest();"
        "    xhr.open('GET', '/runFunction1', true);"
        "    xhr.send();"
        "}"
        "function homing() {"
        "    var xhr = new XMLHttpRequest();"
        "    xhr.open('GET', '/homing', true);"
        "    xhr.send();"
        "}"
        "function sendNumber() {"
        "    var number = document.getElementById('numberInput').value;"
        "    var xhr = new XMLHttpRequest();"
        "    xhr.open('GET', '/sendNumber?value=' + number, true);"
        "    xhr.send();"
        "}"
        "</script>"
        "</body></html>";

                  
                  
                  ;
    server.send(200, "text/html", html);
}

void runFunction1() {
    // Code for function 1
    server.send(200, "text/plain", "Function 1 Executed");
    stepper.step(stepperPocetKroku);
    odpojMotor();
}

void sendNumber() {
    if (server.hasArg("value")) {
        int number = server.arg("value").toInt();
        jizda(number);
        server.send(200, "text/plain", "Number received: " + String(number));
    } else {
        server.send(400, "text/plain", "No number provided");
    }
}


void jizda(int minuty){
  if(homed == 0){
    homing();
    }
  stepperSpeed = calcMinutesToSpeed(minuty);
  stepper.setSpeed(stepperSpeed);
  Serial.print("Minuty:");
  Serial.println(minuty);
  Serial.print("Rychlost:");
  Serial.println(stepperSpeed);
  do {
    stepper.step(stepperPocetKroku);
    stepperPozice = stepperPozice + stepperPocetKroku;
    homed = 0;
    //Serial.print("Pozice:");
    //Serial.println(stepperPozice);
    //Serial.print("MInuty:");
    //Serial.println(minuty);
    } while (stepperPozice != stepperCelaDelka); 
    odpojMotor();
}


int calcMinutesToSpeed(int minuty){
 int vysledek = (265-minuty)/0.2;
  if(vysledek > 800){
    vysledek = 800;
    Serial.print("Vysledek:");
    Serial.println(vysledek);
    }
  vysledek=10;
  return vysledek;
  }

void homing(){
  Serial.print("HOMING");
  int endStopState;
    stepper.setSpeed(stepperHomingSpeed);
  do {      
      endStopState = digitalRead(endstopPin);
      stepper.step(-stepperPocetKroku);  
    } while (endStopState == LOW);
    Serial.println("HOMED");
    homed = 1;
    odpojMotor();
    stepper.setSpeed(stepperSpeed);
}

void odpojMotor(){
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
}

void setup() {
  Serial.begin(9600);
  pinMode(endstopPin, INPUT_PULLDOWN);
// APčko
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

// Hooky
  server.on("/", HTTP_GET, handleRoot);
  server.on("/runFunction1", HTTP_GET, runFunction1);
  server.on("/homing", HTTP_GET, homing);
  server.on("/sendNumber", HTTP_GET, sendNumber);

  server.begin();
  Serial.println("HTTP server started");

// Výchozí rychlost stepperu
  stepper.setSpeed(stepperSpeed);
}

void loop() {
  server.handleClient(); // Handle client requests
}
