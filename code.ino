#include <Wire.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// --- Hardware Interfacing ---
const int MPU_ADDR = 0x68; 
const int RXD2 = 16;       
const int TXD2 = 17;       
const int BUZZER_PIN = 18; // Resurrected acoustic actuator

// --- System Instantiations ---
TinyGPSPlus gps;           
HardwareSerial GPSSerial(2); 

// --- Kinematic Metrics ---
int16_t ax, ay, az;
float roll, pitch;
unsigned long lastEventTime = 0;
const unsigned long COOLDOWN_MS = 1000; 

void setup() {
  Serial.begin(115200);     
  GPSSerial.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize I2C Topology (GPIO21=SDA, GPIO22=SCL)
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management register
  Wire.write(0);    // Resuscitate MPU6050
  Wire.endTransmission(true);

  triggerAcousticAlert(1, 200); // Genesis pulse
  Serial.println("System  : OMNISCIENT SENTINEL ACTIVE\n─────────────────────────────────"); 
}

void loop() {
  // 1. Ingest NMEA Streams (Asynchronous Hardware UART)
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }

  // 2. Interrogate Inertial Registers
  readMPU6050();
  
  // 3. Synthesize Spatial Orientation (Inverse Trigonometry)
  roll = atan2(ay, az) * 180.0 / PI;     
  pitch = atan2(-ax, az) * 180.0 / PI;   

  // 4. Heuristic Kinematic Classification
  if (millis() - lastEventTime > COOLDOWN_MS) {
    
    // Paradigm 1: Critical Spatial Inversion (Capsizing)
    if (abs(roll) > 60.0 || abs(pitch) > 60.0) { 
      Serial.println("EVENT   : SPATIAL INVERSION / ROLLOVER"); 
      Serial.print("ROLL    : "); Serial.println(roll);         
      Serial.print("PITCH   : "); Serial.println(pitch);        
      logGeospatialVectors();
      Serial.println("─────────────────────────────────");
      triggerAcousticAlert(5, 100); // 5 rapid cyclical pulses
      lastEventTime = millis();
    }
    // Paradigm 2: Kinetic Impulse (Z-Axis Shockwave)
    else if (abs(az) > 20000) { 
      Serial.println("EVENT   : KINETIC IMPULSE / POTHOLE");    
      logGeospatialVectors();
      Serial.println("─────────────────────────────────");
      triggerAcousticAlert(2, 200); // 2 short cyclical pulses
      lastEventTime = millis();
    }
  }
  
  delay(100); // Microsecond computational stabilization
}

// --- Subroutines ---

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Initiate sequential read at ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true); 
  
  // Amalgamate 16-bit signed vector payloads
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

void triggerAcousticAlert(int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delayMs);
  }
}

void logGeospatialVectors() {
  if (gps.location.isValid()) {
    Serial.print("LAT     : "); Serial.println(gps.location.lat(), 6); 
    Serial.print("LON     : "); Serial.println(gps.location.lng(), 6); 
  } else {
    Serial.println("GPS     : ACQUIRING ORBITAL LOCK..."); 
  }
}
