
#include <Servo.h> // including servo library.
#include <Encoder.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> //Include File System Headers
#include <ArduinoJson.h>

/* I/O Pin Connections
  # Rotary Encoder KY-040
    GND -- GND
    +   -- 3.3V
    DT  -- D2
    CLK -- D1
  # Servo HiTEC HS-422
    GND -- GND     || Optionally  servo can be powered 
    +   -- 3.3V    || by an external power source such
    DT  -- D3      || as a 6V battery.  
 */

int inputAngle;

long oldPosition = -999;

const char *htmlfile = "/index.html";

//WiFi Connection configuration
const char *ssid = "****";
const char *password = "****";

Encoder myEnc(D1, D2);       // Setup rotary-encoder.
Servo servo_1;               // Giving name to servo.
ESP8266WebServer server(80); // Setup Web Server.

const size_t manualresponcebufferSize = JSON_OBJECT_SIZE(1);

void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  server.send(302, "text/plane", "");
}

void autoTestData()
{
  StaticJsonBuffer<2000> jsonBuffer;
  JsonArray &array = jsonBuffer.createArray();
  int initialRead;
  for (int i = 0; i < 181; i += 6)
  {
    JsonObject &nested = array.createNestedObject();
    Serial.print("Auto Angle: ");
    Serial.println(i);
    servo_1.write(i);
    nested["s"] = int(i);
    delay(999);
    long newPosition = myEnc.read();
    if (i == 0)
      initialRead = newPosition;
    if (newPosition != oldPosition)
    {
      oldPosition = newPosition;
      Serial.print("Auto Encoder Val: ");
      Serial.println(newPosition);
      nested["e"] = (newPosition - initialRead) * 6;
    }
    else
    {
      nested["e"] = (oldPosition - initialRead) * 6;
    }
  }
  servo_1.write(0);
  String jsonString;
  array.printTo(jsonString);
  server.send(200, "application/json", jsonString);
}

void manualTestData()
{
  DynamicJsonBuffer jsonBuffer(manualresponcebufferSize);

  JsonObject &root = jsonBuffer.createObject();

  root.printTo(Serial);
  if (server.arg("servoAngle") == "")
  { //Parameter not found
    server.send(404);
  }
  else
  {                                               //Parameter found
    String angle = "" + server.arg("servoAngle"); //Gets the value of the query parameter
    Serial.print("Manual Angle: ");
    Serial.println(angle.toInt());
    servo_1.write(0);
    delay(999);
    long statePosition = myEnc.read();
    servo_1.write(angle.toInt());
    delay(999);
    long changePosition = myEnc.read();
    root["e"] = (changePosition - statePosition) * 6;
    Serial.print("Manual Encoder Val: ");
    Serial.println((changePosition - statePosition) * 6);
    servo_1.write(0);
  }
  String jsonString;
  root.printTo(jsonString);
  server.send(200, "application/json", jsonString);
}

void handleWebRequests()
{
  if (loadFromSpiffs(server.uri()))
    return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  servo_1.attach(0); // Attaching Servo to D3

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  //Connect to wifi Network
  WiFi.begin(ssid, password); //Connect to your WiFi router
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //IP address assigned to your ESP

  //Initialize Webserver
  server.on("/", handleRoot);
  server.on("/autoTestData", autoTestData); //Reads ADC function is called from out index.html
  server.on("/manualTestData", manualTestData);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.begin();
}

void loop()
{
  server.handleClient();
}

bool loadFromSpiffs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.htm";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }

  dataFile.close();
  return true;
}