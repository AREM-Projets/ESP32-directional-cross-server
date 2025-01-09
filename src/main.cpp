#include <WiFi.h>

// Original Author : Lïam LOTTE - EI22
// Release Date 09/01/2025

/*---------------------------------------------------------------------------------------------------*/
// Wi-Fi credentials and mode selection
#define ACCESS_POINT 1 // Set 1 for station mode, 0 for access point mode
const char *ssid = "your_own_wiki";
const char *password = "your_own_password";
const char *ssid_access_point = "robot1";

// Static IP configuration for the Access Point
IPAddress local_IP(192, 168, 4, 1); // The static IP address of the Access Point
IPAddress gateway(192, 168, 4, 1);  // The gateway IP is usually the same as the AP's IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask

// Define RX and TX pins for UART 2
#define RXD2 16
#define TXD2 17

// Web server port
WiFiServer server(80);

/*---------------------------------------------------------------------------------------------------*/

// HTML webpage stored in program memory
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Cross Direction</title>
    <style>
        body {
            background-color: #ECF1F1;
            font-family: Arial, sans-serif;
            text-align: center;
        }
        .directional-buttons {
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .row {
            display: flex;
            justify-content: center;
        }
        .button {
            width: 60px;
            height: 60px;
            margin: 10px;
            font-size: 20px;
            color: white;
            background-color: #00979D;
            border: none;
            border-radius: 10px;
            cursor: pointer;
        }
        .button:hover {
            background-color: #007b82;
        }
    </style>
    <script>
        function sendDirection(direction) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/direction?dir=" + direction, true);
            xhr.send();
        }
    </script>
</head>
<body>
    <h1>Control Direction</h1>
    <div class="directional-buttons">
        <div class="row">
            <button class="button" onclick="sendDirection('up')"></button>
        </div>
        <div class="row">
            <button class="button" onclick="sendDirection('left')"></button>
            <button class="button" onclick="sendDirection('center')"></button>
            <button class="button" onclick="sendDirection('right')"></button>
        </div>
        <div class="row">
            <button class="button" onclick="sendDirection('down')"></button>
        </div>
    </div>
</body>
</html>
)rawliteral";

/*---------------------------------------------------------------------------------------------------*/

// Function to handle Wi-Fi connection
void setupWiFi() {
    // Set the static IP for the Access Point
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure Access Point with static IP.");
    }

    if (ACCESS_POINT) {
        Serial.println("Connecting to Wi-Fi as Station...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWiFi connected.");
        Serial.print("Station IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Starting Access Point...");
        if (WiFi.softAP(ssid_access_point)) {
            Serial.println("Access Point started successfully.");
            Serial.print("AP IP address: ");
            Serial.println(WiFi.softAPIP());
        } else {
            Serial.println("Failed to start Access Point.");
        }
    }
}

// Function to handle direction
void handleDirection(const String &dir) {
    char directionChar = 'c'; // Default to 'c'

    if (dir == "up") directionChar = 'u';
    else if (dir == "down") directionChar = 'd';
    else if (dir == "left") directionChar = 'l';
    else if (dir == "right") directionChar = 'r';

    Serial.print("Direction: ");
    Serial.println(directionChar);

    // Send direction to UART2
    Serial2.write(directionChar);
}

// Function to handle client requests
void handleClient(WiFiClient &client) {
    String request = "";
    while (client.connected() && client.available()) {
        char c = client.read();
        request += c;
    }

    // Parse direction request
    if (request.indexOf("GET /direction?dir=") >= 0) {
        int start = request.indexOf("GET /direction?dir=") + 19;
        int end = request.indexOf(" ", start);
        String direction = request.substring(start, end);
        handleDirection(direction);
    }

    // Send webpage
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println(webpage);

    client.stop();
    Serial.println("Client disconnected.");
}

/*---------------------------------------------------------------------------------------------------*/

// Setup function
void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    delay(5000);

    setupWiFi();
    server.begin();
    Serial.println("Web server started.");
}

// Main loop
void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New Client Connected.");
        handleClient(client);
    }
}
