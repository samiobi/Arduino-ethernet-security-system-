#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

// Ethernet setup
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(xxx, xxx, xxx, xx);
EthernetServer server(80);

// Pins
const int pir1Pin = 2;
const int pir2Pin = 3;
const int relayPin = 4;
const int buzzerPin = 5;
const int servoPin = 6;

Servo myServo;

bool pir1State = false;
bool pir2State = false;
int servoAngle = 0;

void setup() {
  pinMode(pir1Pin, INPUT);
  pinMode(pir2Pin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  myServo.attach(servoPin);
  myServo.write(0);

  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  pir1State = digitalRead(pir1Pin);
  pir2State = digitalRead(pir2Pin);

  bool alertTriggered = false;

  if (pir1State) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    myServo.write(180);
    servoAngle = 180;
    alertTriggered = true;
  }

  if (pir2State) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    myServo.write(90);
    servoAngle = 90;
    alertTriggered = true;
  }

  if (!alertTriggered) {
    digitalWrite(relayPin, LOW);
    digitalWrite(buzzerPin, LOW);
    myServo.write(0);
    servoAngle = 0;
  }

  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          // Web page
          client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Security Monitor</title>");
          client.println("<meta http-equiv='refresh' content='2'>"); // ⬅ Auto-refresh every 2 seconds
          client.println("<style>");
          client.println("body { font-family: Arial; background-color: lightblue; margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; }");
          client.println("h1 { margin-bottom: 30px; font-size: 28px; color: #333; }");
          client.println(".container { display: flex; gap: 50px; align-items: center; justify-content: center; }");
          client.println(".circle { width: 120px; height: 120px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 16px; color: white; font-weight: bold; }");
          client.println(".active { background-color: red; }");
          client.println(".normal { background-color: green; }");
          client.println(".angle { font-size: 20px; font-weight: bold; padding: 10px 20px; background: white; border-radius: 10px; border: 2px solid #000; }");
          client.println("</style></head><body>");

          // Visible title
          client.println("<h1>Security Monitoring System</h1>");

          client.println("<div class='container'>");

          // PIR 1 Circle
          client.print("<div class='circle ");
          client.print(pir1State ? "active" : "normal");
          client.print("'>PIR 1<br>");
          client.print(pir1State ? "Motion" : "No Motion");
          client.println("</div>");

          // Servo Angle
          client.print("<div class='angle'>Servo: ");
          client.print(servoAngle);
          client.println("°</div>");

          // PIR 2 Circle
          client.print("<div class='circle ");
          client.print(pir2State ? "active" : "normal");
          client.print("'>PIR 2<br>");
          client.print(pir2State ? "Motion" : "No Motion");
          client.println("</div>");

          client.println("</div></body></html>");
          break;
        }
        if (c == '\n') currentLineIsBlank = true;
        else if (c != '\r') currentLineIsBlank = false;
      }
    }
    delay(1);
    client.stop();
  }
}
