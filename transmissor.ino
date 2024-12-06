#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Configurações de pinos do LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 915E6

// Configurações de pinos do display OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Configurações de pinos dos sensores
#define DHTPIN 12  // Pino onde o DHT11 está conectado para temperatura e umidade
#define LDRPIN 34  // Pino onde o LDR está conectado para luminosidade

// Tipo de sensor DHT
#define DHTTYPE DHT11

// Inicializa o DHT
DHT dht(DHTPIN, DHTTYPE);

// Inicializa o display OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Variáveis para armazenar os dados
int packet_id = 0;
float temperature = 0;
float humidity = 0;
int luminosity = 0;
String device_id = "SF07";
int counter = 0; 

void startOLED() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha na inicialização do display SSD1306");
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA SENDER");
  display.display();
  delay(2000);
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SF 10");
  display.setCursor(0, 12);
  display.print("Contador: ");
  display.println(counter);
  display.setCursor(0, 24);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.setCursor(0, 36);
  display.print("Hum: ");
  display.print(humidity);
  display.println(" %");
  display.setCursor(0, 48);
  display.print("Lum: ");
  display.println(luminosity);
  display.display();
}

void startLoRA() {
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Falha ao iniciar o LoRa!");
    while (true);
  }

  LoRa.setSpreadingFactor(7); // Spreading Factor SF = 7
  LoRa.enableCrc();
  LoRa.setCodingRate4(5);

    // Configuração da Bandwidth (BW)
  LoRa.setSignalBandwidth(125E3); // Bandwidth BW = 125 kHz

    // Configuração da potência de transmissão
  LoRa.setTxPower(20); // Potência = 20 dBm

  Serial.println("LoRa inicializado com sucesso com os seguintes parâmetros:");
  Serial.println("Spreading Factor: 12");
  Serial.println("Code Rate: 4/6");
  Serial.println("Bandwidth: 125 kHz");
  Serial.println("Potência de Transmissão: 20 dBm");
}

void getReadings() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Falha na leitura do DHT11!");
    temperature = 0;
    humidity = 0;
  }
  luminosity = analogRead(LDRPIN);
}

void sendReadings() {
  String LoRaMessage = "{\"device_name\":\"" + device_id + "\","
    + "\"temperature\":" + temperature + ","
    + "\"humidity\":" + humidity + ","
    + "\"luminosity\":" + 100 + ","
    + "\"sent\":" + (++counter) + ","
    + "\"long\":" + 121212 + ","
    + "\"lat\":" + 12121 + ",";
  

  LoRa.beginPacket();
  LoRa.println(LoRaMessage);
  LoRa.endPacket();

  Serial.println("Mensagem enviada: " + LoRaMessage);
  packet_id++;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  startOLED();
  startLoRA();
}

void loop() {
  getReadings();
  sendReadings();
  updateOLED();
  delay(10000);
}