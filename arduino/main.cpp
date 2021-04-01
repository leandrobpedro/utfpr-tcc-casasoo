/**
 * Project  : Casasoo
 * File     : main.cpp
 * Version  : 2.1.1
 * Date     : 2015.10.04
 * Author   : leandrobpedro@gmail.com
 */

#include <Ethernet.h>
#include <math.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>

#define ACS712  A2  // Current sensor.
#define LM35    A3  // Temperature sensor.

// Quiescent output voltage.
int quiescentVoltage;

// Servo
Servo servo;

// Ethernet configuration.
byte mac[] =
    {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Mac address.
IPAddress ip(10, 0, 0, 210);              // Ip address.
IPAddress mask(255, 255, 255, 0);         // Subnet mask.
IPAddress gateway(10, 0, 0, 254);         // Gateway address.
IPAddress dnsserver(10, 0, 0, 1);         // Dns server ip.

// Create a server that listens for incoming connections.
EthernetServer server(80);

float get_temp(int pin) {
  float temp = 0;
  int sample = 5;

  for (int i = 0; i < sample; i++) {
    temp += analogRead(pin);
    delay(1);
  }

  return (float) temp / sample;
}

int get_quiescent_voltage(int pin) {
  int sample = 1000;
  long quiescentVoltage = 0;

  // Read samples to stabilise value.
  for (int i = 0; i < sample; i++) {
    quiescentVoltage += abs(analogRead(pin));
    delay(1);
  }

  quiescentVoltage /= sample;

  return (int) quiescentVoltage;
}

boolean read_current(int pin) {
  float sensor = 0, aux = 0;
  int sample = 5, sensibility = 5;

  // Read samples to stabilise value.
  for (int i = 0; i < sample; i++) {
    aux = abs(analogRead(pin) - quiescentVoltage);
    if (aux > sensibility) {
      sensor += aux;
    }
    delay(1);
  }

  return (sensor / sample) > 1 ? true : false;
}

boolean xhr(EthernetClient client, const String request) {
  String cmd =
      request.substring(request.indexOf("?cmd=") + 5, request.indexOf("&"));
  String value =
      request.substring(request.indexOf("&value=") + 7, request.indexOf("\n"));

  if (cmd.equals("toggle")) {
    pinMode(value.toInt(), OUTPUT);
    digitalWrite(value.toInt(), !digitalRead(value.toInt()));
  } else if (cmd.equals("servo")) {
    servo.write(value.toInt());
  } else if (cmd.equals("refresh")) {
    client.println();                   // Blank line.
    client.print("?servo=");            // Servo.
    client.print(servo.read());         // Servo current position.
    client.print("&light=");            // Light.
    client.print(read_current(ACS712)); // Light state.
    client.print("&temp=");             // Temperature.
    client.print(get_temp(LM35));       // Temperature value.
  } else {
    return false;
  }

  return true;
}

void listen_ethernet_clients () {
  EthernetClient client = server.available(); // Client connected.
  const int bufsiz = 99;
  char request[bufsiz], c;
  int index = 0;
  int isXhRequest = 0;

  if (client) {
    if (client.connected()) {
      // Get client request.
      while (client.available()) {
        c = client.read();

        if (c != '\n' && c != '\r') {
          request[index] = c;
          index++;
        } else {
          break;
        }
      }
    }

    request[index] = NULL;

    // Format client request.
    if (strstr(request, "GET /") != 0) {
      (strstr(request, " HTTP"))[0] = 0;

      // Remove "GET /" (5 chars)
      if (strstr(request, "GET /")) {
        memmove(request, request + 5, 1 + strlen(request) + 5);
      }

      if (!strlen(request)) {
        // TODO: check if request is empty
         strcpy(request, "index.htm");
      }
    }

//    Serial.println(request);

    // Send a standard http response header.
    client.println("HTTP/1.1 200 OK");

    // MIME Types - Multipurpose Internet Mail Extensions.
    if (strstr(request, ".htm")) {                // .htm
      client.println("Content-Type: text/html");
    } else if (strstr(request, ".css")) {         // .css
      client.println("Content-Type: text/css");
    } else if (strstr(request, ".js")) {          // .js
      client.println("Content-Type: application/javascript");
    } else if (strstr(request, ".png")) {         // .png
      client.println("Content-Type: image/png");
    } else if (strstr(request, ".jp")) {          // .jpg, .jpeg
      client.println("Content-Type: image/jpeg");
    } else if (strstr(request, "?cmd")) {         // Ajax request.
      client.println("Content-Type: text");
      isXhRequest = xhr(client, request);
    } else {
      client.println("Content-Type: text");
    }

    // If it's a file.
    if (!isXhRequest) {

      client.println();

      // Open file.
      File webfile = SD.open(request);

      if (webfile) {
        while(webfile.available()) {
          client.write(webfile.read());
        }
        webfile.close();
      }
    }

    // Give some time to browser receive data.
    delay(1);
    client.stop();
  }
}

void setup () {
  Serial.begin(9600);

  // For external power supply.
  // The voltage applied to the AREF pin (0 to 5V) is used as the reference.
  analogReference(EXTERNAL);

  // Set pins as output.
  pinMode(ACS712, INPUT);
  pinMode(LM35, INPUT);

  // Determine quiescent output voltage.
  quiescentVoltage = get_quiescent_voltage(ACS712);

  Serial.print("Quiescent voltage: ");
  Serial.println(quiescentVoltage);

  // Initialize the ethernet device.
  // Ethernet shield attached to pins 10, 11, 12, 13.
  Ethernet.begin(mac, ip, dnsserver, gateway, mask);

  // Initialize server.
  server.begin();

  // Servo motor.
  servo.attach(9);  // Pin.
  servo.write(90);  // Initial position.

  // initialize SD card.
  Serial.println("Initializing SD card...");
  if (!SD.begin(4)) {  // cs pin at 4
    Serial.println("ERROR - SD card initialization failed!");
    return;
  } else {
    Serial.println("SUCCESS - SD card initialized.");
  }

  // Check for index.htm file.
  char *c = new char[12];
  strcpy(c, "index.htm");

  if (!SD.exists(c)) {
    // Can't find index file.
    Serial.println("ERROR - Can't find index.htm file!");
    return;
  } else {
    Serial.println("SUCCESS - Found index.htm file.");
  }

  // Print out the IP address.
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
}

void loop () {
  // Listen for incoming Ethernet connections.
  listen_ethernet_clients();
}
