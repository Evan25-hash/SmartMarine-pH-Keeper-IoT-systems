#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <time.h>


#define PHPin A0
#define sensorPin A4
#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6ZT2DzBQY"
#define BLYNK_TEMPLATE_NAME "SmartMarine pH Keeper"
#define BLYNK_AUTH_TOKEN "jau8-TFrnFm41uxsSaSFTpwKDfVmLIyL"

#include <BlynkSimpleEsp32.h>

char auth[] = "jau8-TFrnFm41uxsSaSFTpwKDfVmLIyL"; 
char ssid[] = "patka";
char pass[] = "10022004"; // Jika tidak menggunakan password kosongkan ""

// Deklarasi objek servo
Servo myservo;
Servo myservo2;

// Pin servo terhubung ke pin 4 pada ESP32
int servoPin = 4;
int servoPin2 = 16;
int currentPosition = 0; // Menyimpan posisi servo saat ini
int currentPosition2 = 0; // Menyimpan posisi servo saat ini untuk servo 2

int buf[10];
unsigned long previousMillis = 0;
unsigned long lastServoMoveMillis = 0;
const long interval = 900000; // 15 menit dalam milidetik (15 * 60 * 1000)
const long readInterval = 1000; // 1 detik dalam milidetik (1 * 1000)
const long timePrintInterval = 30000; // Cetak waktu setiap 30 detik

// Batasan untuk menentukan status kekeruhan air (dalam milivolt)
const float clearThreshold = 300.0; // Jernih jika < 500 mV
const float murkyThreshold = 150.0; // Kusam jika >= 500 mV dan < 1500 mV

// Fungsi untuk menghitung nilai pH dari tegangan
float ph(float voltage) {
  return 7 + ((2.5 - voltage) / 0.18);
}

// Fungsi untuk menghitung keanggotaan fuzzy untuk kekeruhan rendah
float fuzzyMembershipLow(float x) {
  if (x <= murkyThreshold) {
    return 1.0;
  } else if (x >= clearThreshold) {
    return 0.0;
  } else {
    return (clearThreshold - x) / (clearThreshold - murkyThreshold);
  }
}

// Fungsi untuk menghitung keanggotaan fuzzy untuk kekeruhan sedang
float fuzzyMembershipMedium(float x) {
  if (x <= murkyThreshold || x >= clearThreshold) {
    return 0.0;
  } else if (x > murkyThreshold && x < clearThreshold) {
    return (x - murkyThreshold) / (clearThreshold - murkyThreshold);
  } else {
    return (clearThreshold - x) / (clearThreshold - murkyThreshold);
  }
}

// Fungsi untuk menghitung keanggotaan fuzzy untuk kekeruhan tinggi
float fuzzyMembershipHigh(float x) {
  if (x <= clearThreshold) {
    return 0.0;
  } else if (x >= 3000.0) {
    return 1.0;
  } else {
    return (x - clearThreshold) / (3000.0 - clearThreshold);
  }
}

// Fungsi untuk menentukan status kekeruhan berdasarkan keanggotaan fuzzy
String fuzzyStatus(float low, float medium, float high) {
  if (low >= medium && low >= high) {
    return "Kotor";
  } else if (medium >= low && medium >= high) {
    return "Kusam";
  } else {
    return "Jernih";
  }
}

void setup() {
  Serial.begin(9600);

  // Inisialisasi pin untuk sensor pH
  pinMode(PHPin, INPUT);

  // Inisialisasi servo
  myservo.attach(servoPin);
  myservo2.attach(servoPin2);

  // Mengatur posisi awal servo ke 0 derajat
  myservo.write(currentPosition);
  myservo2.write(currentPosition2);

  // Tambahkan pesan untuk menunjukkan bahwa setup selesai
  Serial.println("Setup selesai");

  // Inisialisasi koneksi WiFi dan Blynk
  Blynk.begin(auth, ssid, pass);

  // Inisialisasi waktu
  configTime(7 * 3600, 0, "pool.ntp.org");

  // Menunggu sinkronisasi waktu
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWaktu tersinkronisasi");
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal mendapatkan waktu");
    return;
  }
  Serial.printf("Waktu saat ini: %02d:%02d:%02d WIB\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void loop() {
  unsigned long currentMillis = millis();

  // Membaca nilai pH dan turbidity setiap detik
  if (currentMillis - previousMillis >= readInterval) {
    previousMillis = currentMillis;

    // Membaca nilai pH
    for (int i = 0; i < 10; i++) {
      buf[i] = analogRead(PHPin);
      delay(10); // Mengurangi delay untuk membaca lebih cepat
    }

    float avgValue = 0;
    for (int i = 0; i < 10; i++)
      avgValue += buf[i];

    float pHVol = (float)avgValue * 5.0 / 4096 / 10;
    float pHValue = -3.109 * pHVol + 19.54; // hasi   l regresi linear pH

    Serial.print("pH: ");
    Serial.println(pHValue);

    // Membaca nilai turbidity
    int sensorValue = analogRead(sensorPin);
    float voltage = sensorValue * (3300.0 / 4096.0);

    // Hitung keanggotaan fuzzy
    float lowMembership = fuzzyMembershipLow(voltage);
    float mediumMembership = fuzzyMembershipMedium(voltage);
    float highMembership = fuzzyMembershipHigh(voltage);

    // Menentukan status kekeruhan berdasarkan keanggotaan fuzzy
    String status = fuzzyStatus(lowMembership, mediumMembership, highMembership);

    // Print nilai sensor, tegangan (dalam milivolt), dan status kekeruhan ke serial monitor
    Serial.print("Nilai sensor: ");
    Serial.print(sensorValue);
    Serial.print("\t Tegangan (mV): ");
    Serial.print(voltage);
    Serial.print("\t Status: ");
    Serial.println(status);

    // Kirim data ke Blynk
    Blynk.virtualWrite(V0, pHValue);
    Blynk.virtualWrite(V1, sensorValue);
    Blynk.virtualWrite(V2, status);

    // Memeriksa nilai pH dan menggerakkan servo jika perlu
    if (pHValue < 5.1 && (currentMillis - lastServoMoveMillis >= interval)) {
      currentPosition = 45; // Mengatur posisi servo ke 45 derajat
      myservo.write(currentPosition);
      Serial.println("Servo dibuka ke 45 derajat");

      delay(1000); // Menunggu sejenak sebelum mengembalikan servo ke posisi semula

      currentPosition = 0; // Mengatur posisi servo kembali ke 0 derajat
      myservo.write(currentPosition);
      Serial.println("Servo ditutup ke 0 derajat");

      // Mencatat waktu terakhir servo bergerak
      lastServoMoveMillis = currentMillis;
    }
  }

  // Mendapatkan waktu saat ini
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal mendapatkan waktu");
    return;
  }

  // Cetak waktu setiap 30 detik
  static unsigned long lastTimePrintMillis = 0;
  if (currentMillis - lastTimePrintMillis >= timePrintInterval) {
    printLocalTime();
    lastTimePrintMillis = currentMillis;
  }

  // Memeriksa waktu saat ini dan menggerakkan servo pada jam 8:00 dan 16:00
  if ((timeinfo.tm_hour == 8 || timeinfo.tm_hour == 16) && timeinfo.tm_min == 0 && timeinfo.tm_sec < 10) {
    currentPosition2 = 45; // Mengatur posisi servo ke 45 derajat
    myservo2.write(currentPosition2);
    Serial.println("Servo 2 dibuka ke 45 derajat");

    delay(10000); // Menunggu 10 detik sebelum mengembalikan servo ke posisi semula

    currentPosition2 = 0; // Mengatur posisi servo kembali ke 0 derajat
    myservo2.write(currentPosition2);
    Serial.println("Servo 2 ditutup ke 0 derajat");
  }

  // Memeriksa apakah ada data yang tersedia di serial
  if (Serial.available() > 0) {
    // Membaca data dari serial
    String command = Serial.readStringUntil('\n');
    
    // Memproses perintah dari serial
    if (command == "makan") {
      currentPosition2 = 45; // Mengatur posisi servo ke 45 derajat
      myservo2.write(currentPosition2);
      Serial.println("Makan Wak");
      
      delay(1000); // Menunggu 1 detik sebelum mengembalikan servo ke posisi semula
      
      currentPosition2 = 0; // Mengatur posisi servo kembali ke 0 derajat
      myservo2.write(currentPosition2);
      Serial.println("Mantap");
      Blynk.virtualWrite(V3, currentPosition2);
    } else {
      Serial.println("Perintah tidak dikenal");
    }
  }

  Blynk.run();
}