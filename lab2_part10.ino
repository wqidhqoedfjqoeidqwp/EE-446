#include <Arduino_BMI270_BMM150.h>
#include <Arduino_APDS9960.h>
#include <PDM.h>

// Microphone

short sampleBuffer[256];
volatile int samplesRead = 0;

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}

// Motion 

float prevX = 0;
float prevY = 0;
float prevZ = 0;
bool firstSample = true;


void setup() {

  Serial.begin(115200);
  delay(1500);

  // IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU.");
    while (1);
  }

  // APDS9960
  if (!APDS.begin()) {
    Serial.println("Failed to initialize APDS9960.");
    while (1);
  }

  // Microphone
  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM microphone.");
    while (1);
  }

  Serial.println("Sensor fusion started.");
}


void loop() {

  //Microphone 

  int mic = 0;

  if (samplesRead) {
    long sum = 0;

    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }

    mic = sum / samplesRead;
    samplesRead = 0;
  }

  //Light 

  int red, green, blue, clear;

  if (APDS.colorAvailable()) {
    APDS.readColor(red, green, blue, clear);
  }

  //Proximity 

  int prox = 255;

  if (APDS.proximityAvailable()) {
    prox = APDS.readProximity();
  }

  // Motion

  float x, y, z;
  float motion = 0;

  if (IMU.accelerationAvailable()) {

    IMU.readAcceleration(x, y, z);

    if (!firstSample) {

      float dx = x - prevX;
      float dy = y - prevY;
      float dz = z - prevZ;

      motion = fabs(dx) + fabs(dy) + fabs(dz);

    } else {

      firstSample = false;

    }

    prevX = x;
    prevY = y;
    prevZ = z;
  }

  //Binary Decisions 

  int sound = (mic > 50);

  int dark = (clear < 10 || red < 10 || green < 10 || blue < 10);

  int near = (prox < 200);

  int moving = (motion > 0.03);

  // Classification 

  String state = "UNKNOWN";

  if (!sound && !dark && !moving && !near) {
    state = "QUIET_BRIGHT_STEADY_FAR";
  }
  else if (sound && !dark && !moving && !near) {
    state = "NOISY_BRIGHT_STEADY_FAR";
  }
  else if (!sound && dark && !moving && near) {
    state = "QUIET_DARK_STEADY_NEAR";
  }
  else if (sound && !dark && moving && near) {
    state = "NOISY_BRIGHT_MOVING_NEAR";
  }


  Serial.print("raw,");
  Serial.print("mic=");
  Serial.print(mic);
  Serial.print(",clear=");
  Serial.print(clear);
  Serial.print(",motion=");
  Serial.print(motion, 3);
  Serial.print(",prox=");
  Serial.println(prox);

  Serial.print("flags,");
  Serial.print("sound=");
  Serial.print(sound);
  Serial.print(",dark=");
  Serial.print(dark);
  Serial.print(",moving=");
  Serial.print(moving);
  Serial.print(",near=");
  Serial.println(near);

  Serial.print("state,");
  Serial.println(state);

  delay(100);
}
