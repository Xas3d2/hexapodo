#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "ESP32_WebServer";
const char* password = "12345678";

WebServer server(80);
WebSocketsServer webSocket(81);

// Configurar UART para USC-32 (RX=18, TX=17)
#define USC32_RX 18
#define USC32_TX 17

// Página HTML con WebSockets y botones
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 USC-32 Control</title>
    <style>
        body { background-color: #282c36; font-family: Arial, sans-serif; text-align: center; color: white; margin: 0; padding: 20px; }
        h1 { font-size: 28px; margin-bottom: 20px; }
        .button-container {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            max-width: 600px;
            margin: auto;
        }
        .button {
            padding: 15px;
            font-size: 18px;
            font-weight: bold;
            color: white;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: 0.3s;
        }
        .red { background-color: #dc3545; }   
        .green { background-color: #28a745; } 
        .blue { background-color: #007bff; }  
        .yellow { background-color: #ffc107; }
        .purple { background-color: #6f42c1; }
        .orange { background-color: #fd7e14; }
        .cyan { background-color: #17a2b8; }
        .button:hover { opacity: 0.7; }
    </style>
    <script>
        var websock;
        function start() {
            websock = new WebSocket('ws://' + window.location.hostname + ':81/');
            websock.onopen = function(evt) { console.log("WebSocket conectado"); };
            websock.onmessage = function(evt) { console.log("Mensaje recibido: " + evt.data); };
        }
        function buttonclick(e) {
            websock.send(e.id);
        }
    </script>
</head>
<body onload="javascript:start();">
    <h1>ESP32 USC-32 Servo Control</h1>
    <div class="button-container">
        <button id="boton1" class="button red" onclick="buttonclick(this);">Mover Servo</button>
        <button id="boton2" class="button green" onclick="buttonclick(this);">Mover Servo</button>
        <button id="boton3" class="button green" onclick="buttonclick(this);">Mover Servo</button>
    </div>
</body>
</html>
)rawliteral";

// Manejo de peticiones HTTP
void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

// Enviar comando a USC-32
void sendServoCommand(int servo, int position, int speed) {
    String command = "#" + String(servo) + "P" + String(position) + " T" + String(speed) + "\r\n";
    
    Serial.print("Comando enviado a USC-32: ");
    Serial.println(command);
    
    Serial2.print(command); // Enviar comando al puerto serie USC-32
}

// Manejo de WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        String command = (char*) payload;
        Serial.println("Botón presionado: " + command);

        if (command == "boton1") {
            sendServoCommand(1, 1500, 1000); // Mueve el servo 0 a posición 1500 en 1 segundo
        }
        if (command == "boton2") {
            sendServoCommand(1, 0, 1000); // Mueve el servo 0 a posición 1500 en 1 segundo
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, USC32_RX, USC32_TX);

    // Configurar el ESP32 como punto de acceso
    WiFi.softAP(ssid, password);
    Serial.println("Punto de acceso creado.");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    // Configurar servidor HTTP y WebSockets
    server.on("/", handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    server.handleClient();
    webSocket.loop();
}
