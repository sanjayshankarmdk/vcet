#include <WiFi.h>
#include <PubSubClient.h>

// Motor Control Pins (L298N)
#define ENA 12   // LEFT motor PWM
#define IN1 13   // LEFT motor +
#define IN2 14   // LEFT motor -
#define ENB 25   // RIGHT motor PWM
#define IN3 26   // RIGHT motor +
#define IN4 27   // RIGHT motor -
#define ena 5 
#define in1 18 
#define in2 19 
#define enb 21 
#define in3 22 
#define in4 23 
int motorSpeed = 200;  // PWM speed (0-255)

// Network Settings
const char* ssid = "Tce";
const char* password = "Enaketherla";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  // Initialize motor control pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  stopMotors(); // Start with motors off

  connectWiFi();
  setupMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop(); // This calls callback() automatically
}

// Message received callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message;
  for (int i=0;i<length;i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // ACTUAL MOTOR CONTROL LOGIC
  if (message.indexOf("Zone 1") != -1) {
    moveLeft();
    Serial.println("ACTION: Moving LEFT");
  }
  else if (message.indexOf("Zone 2") != -1) {
    stopMotors();
    Serial.println("ACTION: Stopping");
  }
  else if (message.indexOf("Zone 3") != -1) {
    moveRight();
    Serial.println("ACTION: Moving RIGHT");
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // THIS IS CRUCIAL
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("ball_detection/zone");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// Motor control functions
void moveLeft() {
  analogWrite(ENA, motorSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENB, motorSpeed);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ena, motorSpeed);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enb, motorSpeed);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void moveRight() {
  analogWrite(ENA, motorSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENB, motorSpeed);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ena, motorSpeed);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enb, motorSpeed);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
