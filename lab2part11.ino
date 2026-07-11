#include <Arduino_HS300x.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_APDS9960.h>


bool firstSample = true;

float prevHumidity = 0;
float prevTemperature = 0;
float prevMag = 0;

int prevR = 0;
int prevG = 0;
int prevB = 0;
int prevClear = 0;



void setup() {

  Serial.begin(115200);
  delay(1500);

  if (!HS300x.begin()) {
    Serial.println("Failed to initialize HS3003.");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU.");
    while (1);
  }

  if (!APDS.begin()) {
    Serial.println("Failed to initialize APDS9960.");
    while (1);
  }

  Serial.println("Event detector started.");
}

void loop() {

  //HS3003 

  float rh = HS300x.readHumidity();
  float temp = HS300x.readTemperature();

  //Magnetometer 

  float mx, my, mz;
  float mag = 0;

  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mx, my, mz);

  
    mag = sqrt(mx * mx + my * my + mz * mz);
  }


  int r = 0, g = 0, b = 0, clear = 0;

  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, clear);
  }

  // Indicators 

  int humid_jump = 0;
  int temp_rise = 0;
  int mag_shift = 0;
  int light_or_color_change = 0;

  if (!firstSample) {

    humid_jump = (fabs(rh - prevHumidity) > 5.0);

    temp_rise = ((temp - prevTemperature) > 2.0);

    mag_shift = (fabs(mag - prevMag) > 20.0);

    light_or_color_change =
      (abs(clear - prevClear) > 20 ||
       abs(r - prevR) > 20 ||
       abs(g - prevG) > 20 ||
       abs(b - prevB) > 20);

  }
  else {
    firstSample = false;
  }

  // Save Current Values 

  prevHumidity = rh;
  prevTemperature = temp;
  prevMag = mag;

  prevR = r;
  prevG = g;
  prevB = b;
  prevClear = clear;

  // Classification 

  String state = "BASELINE_NORMAL";

  if (humid_jump || temp_rise) {
    state = "BREATH_OR_WARM_AIR_EVENT";
  }
  else if (mag_shift) {
    state = "MAGNETIC_DISTURBANCE_EVENT";
  }
  else if (light_or_color_change) {
    state = "LIGHT_OR_COLOR_CHANGE_EVENT";
  }

  // Required Output 

  Serial.print("raw,");
  Serial.print("rh=");
  Serial.print(rh, 1);
  Serial.print(",temp=");
  Serial.print(temp, 1);
  Serial.print(",mag=");
  Serial.print(mag, 2);
  Serial.print(",r=");
  Serial.print(r);
  Serial.print(",g=");
  Serial.print(g);
  Serial.print(",b=");
  Serial.print(b);
  Serial.print(",clear=");
  Serial.println(clear);

  Serial.print("flags,");
  Serial.print("humid_jump=");
  Serial.print(humid_jump);
  Serial.print(",temp_rise=");
  Serial.print(temp_rise);
  Serial.print(",mag_shift=");
  Serial.print(mag_shift);
  Serial.print(",light_or_color_change=");
  Serial.println(light_or_color_change);

  Serial.print("state,");
  Serial.println(state);

  Serial.println();

  delay(100);
}