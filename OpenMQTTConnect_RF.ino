// INCLUIR BIBLIOTECA
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Defina suas credenciais de Wi-Fi e MQTT
const char* ssid = "ppmxhjms";
const char* password = "3OFde3Jdvakn";
const char* mqtt_server = "m15.cloudmqtt.com";  // Substitua pelo endereço do seu broker MQTT
const int mqtt_port = 1883;                   // Porta padrão do MQTT
const char* mqtt_user = "Lion";            // Se necessário
const char* mqtt_password = "123@Mudar";          // Se necessário

// Cria instâncias para WiFi e cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Defina o pino para o receptor RF
const int receiverPin = 2;    // Pino onde o receptor está conectado
const unsigned long pulseMin = 500;  // Tempo mínimo para um pulso em microsegundos
const unsigned long pulseMax = 2000; // Tempo máximo para um pulso em microsegundos

// Função de conexão com o Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado. Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para reconectar ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao broker MQTT");
      // Inscreva-se em tópicos se necessário
      // client.subscribe("seu/topico");
    } else {
      Serial.print("Falha na conexão. Código de erro: ");
      Serial.print(client.state());
      Serial.println(" Tentar novamente em 5 segundos");
      delay(5000);
    }
  }
}

// Função de callback MQTT para processar mensagens recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Configuração inicial
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(receiverPin, INPUT);
}

// Função principal
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (digitalRead(receiverPin) == LOW) {
    // Início de um pulso baixo
    unsigned long startTime = micros();
    
    // Espera até o pulso terminar
    while (digitalRead(receiverPin) == LOW) {
      // Não faz nada, só espera
    }
    
    unsigned long pulseLength = micros() - startTime;
    
    // Verifica se o pulso é um pulso de "0" ou "1"
    if (pulseLength > pulseMin && pulseLength < pulseMax) {
      Serial.print("Pulse length: ");
      Serial.println(pulseLength);

      // Envie o comprimento do pulso para o broker MQTT no tópico "rf_data"
      String payload = String(pulseLength);
      client.publish("rf_data", payload.c_str());
    }
  }
}
