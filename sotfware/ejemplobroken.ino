#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "Flia Vasquez"; //modificar dependidendo de donde estes
const char *password = "maritzazo";

// MQTT Broker
const char *mqtt_broker = "broker.hivemq.com";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
const char *topic = "robotica";

// Pins para los LEDs
const int pinLedVerde = D1;
const int pinLedRojo = D2;

WiFiClient pablito;  //cambiar y poner el mismo que el de abajo
PubSubClient client(pablito);   

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Configuración de pines
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);

  // Conexión a MQTT Broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = "esp8266-client-3";
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado al MQTT Broker");
      client.subscribe(topic);
    } else {
      Serial.print("Fallo en la conexión MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void loop() {
  client.loop();
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensaje recibido en el topic: ");
  Serial.println(topic);

  // Convertir el payload a String
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensaje: ");
  Serial.println(message);

  // Procesar el mensaje recibido
  if (message == "1") {
    digitalWrite(pinLedVerde, HIGH);   // Encender LED verde
    digitalWrite(pinLedRojo, LOW);     // Apagar LED rojo
  } else if (message == "0") {
    digitalWrite(pinLedVerde, LOW);    // Apagar LED verde
    digitalWrite(pinLedRojo, HIGH);    // Encender LED rojo
  }
}