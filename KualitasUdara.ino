#define BLYNK_TEMPLATE_ID "TMPL6qPCMb3Pd"
#define BLYNK_TEMPLATE_NAME "KUALITAS UDARA"
#define BLYNK_AUTH_TOKEN "DSzE_2sEQiwFTIcgyZrZjdUqevK3enls" // Ganti dengan token autentikasi Blynk Anda

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>

// Deklarasi pin dan konstanta
#define DHTPIN 13
#define DHTTYPE DHT11
#define PM_LED_PIN 25
#define PM_VOUT_PIN 33
#define MQ135_PIN 32
#define BUZZER_PIN 19
#define FAN_PIN 18

// Inisialisasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C untuk LCD bisa berbeda, sesuaikan dengan alamat I2C yang sesuai

// Inisialisasi sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// Inisialisasi threshold untuk MQ135
const int gasThreshold = 700;  // Threshold diubah menjadi 700
const int FANThreshold = 1;

// Blynk credentials
char ssid[] = "Home wifi"; // Ganti dengan SSID WiFi Anda
char pass[] = "teguh112233"; // Ganti dengan password WiFi Anda

// Variabel untuk menyimpan status tombol kipas
bool fanStatus = false;

BLYNK_WRITE(V5) {
  fanStatus = param.asInt(); // Membaca nilai dari tombol Blynk (0 atau 1)
  digitalWrite(FAN_PIN, fanStatus ? HIGH : LOW); // Mengatur status kipas berdasarkan nilai tombol
}

void setup() {
  // Memulai komunikasi serial
  Serial.begin(115200);

  // Inisialisasi pin untuk kontrol LED dan Buzzer
  pinMode(PM_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  // Memulai LCD
  lcd.init();
  lcd.backlight();
  
  // Menampilkan pesan awal di LCD
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  // Inisialisasi WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 1);
    lcd.print("WiFi...      "); // Menambahkan beberapa spasi untuk menghapus pesan sebelumnya
  }

  // Menampilkan pesan sukses koneksi WiFi di LCD
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print("Connecting to Blynk...");
  
  // Memulai koneksi ke server Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Memulai sensor DHT11
  dht.begin();

  Serial.println("Sensor PM2.5 siap!");
}

float readPM25() {
  const int numSamples = 10;
  float total = 600;

  for (int i = 0; i < numSamples; i++) {
    // Aktifkan LED untuk pengukuran
    digitalWrite(PM_LED_PIN, LOW);
    delayMicroseconds(280);

    // Baca nilai analog dari VOUT pin
    int sensorValue = analogRead(PM_VOUT_PIN);
    delayMicroseconds(40);

    // Matikan LED
    digitalWrite(PM_LED_PIN, HIGH);
    delayMicroseconds(9680);

    // Konversi nilai analog ke konsentrasi PM2.5
    float voltage = sensorValue * (3.3 / 4095.0);  // Asumsi 12-bit ADC dan 3.3V
    float dustDensity = 0.17 * voltage - 0.1;  // Formula konversi berdasarkan datasheet

    total += dustDensity;
    delay(10);  // Tunggu sebentar sebelum pembacaan berikutnya
  }

  return total / numSamples;
}

const char* serverName = "http://192.168.1.4:5000/add"; // Ganti dengan alamat IP server Flask Anda

void sendSensorData() {
  // Membaca data dari sensor DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Membaca nilai analog dari sensor MQ135
  int mq135Value = analogRead(MQ135_PIN);

  // Membaca data dari sensor PM2.5
  float pm25Value = readPM25();

  // Konversi nilai MQ135 menjadi konsentrasi gas
  float no2Value = (mq135Value / 4095.0) * 400; // NO2: 30-400
  float nh3Value = (mq135Value / 4095.0) * 1800; // NH3: 190-1800
  float coValue = (mq135Value / 4095.0) * 34; // CO: 1.0-34.0

  // Membuat JSON payload
  String payload = "{\"temperature\":";
  payload += t;
  payload += ",\"humidity\":";
  payload += h;
  payload += ",\"pm25\":";
  payload += pm25Value;
  payload += ",\"no2\":";
  payload += no2Value;
  payload += ",\"nh3\":";
  payload += nh3Value;
  payload += ",\"co\":";
  payload += coValue;
  payload += "}";

  // Menampilkan data pada LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t); lcd.print(" H:"); lcd.print(h);
  lcd.setCursor(0, 1);
  lcd.print("PM:"); lcd.print(pm25Value); lcd.print(" MQ:"); lcd.print(mq135Value);

  // Menampilkan data pada Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C, Humidity: ");
  Serial.print(h);
  Serial.print(" %, PM2.5: ");
  Serial.print(pm25Value);
  Serial.print(" ug/m3, NO2: ");
  Serial.print(no2Value);
  Serial.print(" ppb, NH3: ");
  Serial.print(nh3Value);
  Serial.print(" ppm, CO: ");
  Serial.println(coValue);

  // Kirim data ke Blynk server
  Blynk.virtualWrite(V1, t);  // Widget V1 untuk menampilkan suhu
  Blynk.virtualWrite(V2, h);  // Widget V2 untuk menampilkan kelembaban
  Blynk.virtualWrite(V3, pm25Value);  // Widget V3 untuk menampilkan nilai PM2.5
  Blynk.virtualWrite(V4, no2Value);  // Widget V4 untuk menampilkan nilai NO2
  Blynk.virtualWrite(V5, nh3Value);  // Widget V5 untuk menampilkan nilai NH3
  Blynk.virtualWrite(V6, coValue);  // Widget V6 untuk menampilkan nilai CO

  // Mengaktifkan buzzer jika gas berbahaya terdeteksi
  if (mq135Value > gasThreshold) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Kirim data ke server Flask
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void loop() {
  Blynk.run();
  if (Blynk.connected()) {
    lcd.setCursor(0, 0);
    
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Blynk Disconnected");
    Serial.println("Blynk Disconnected");
  }
  sendSensorData();
  delay(2000);  // Menunda 2 detik sebelum pembacaan berikutnya
}
