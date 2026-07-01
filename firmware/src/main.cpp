#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SPI.h>


const int leftButtonPin = 2;
const int rightButtonPin = 1;
const int backButtonPin = 4;
const int frontButtonPin = 3;
const int batteryPin = A0;
const int encoderAPin = 5;
const int encoderBPin = 6;
 const int ledRedPin = 26;
 const int ledGreenPin = 30;
 const int sensorCSPin = 10;
 const int sensorRstPin = 9;
 const int sensorMotPin = 8;

int lastA = HIGH;


uint8_t const desc_hid_report[] = { TUD_HID_REPORT_DESC_MOUSE() };
Adafruit_USBD_HID usb_hid;

const float LOW_BATTERY_V = 3.4;
const float DIVIDER_RATIO = 2.0;

float readBatteryVoltage() {
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead (batteryPin);
        delay(2);  
    }
    float raw = sum  / 16.0f;
    float vout = raw * (3.3/4095.0);
    return vout * DIVIDER_RATIO;
}


void setColor(bool r, bool g) {
    digitalWrite(ledRedPin, r ? LOW : HIGH);
    digitalWrite(ledGreenPin, g ? LOW : HIGH);
}


void showBatteryColor(float vbatt) {
    if (vbatt > 3.9) setColor(false, true);
    else if (vbatt > 3.6) setColor (true, true);
    else if (vbatt > 3.4) setColor (true, false);
    else setColor(false, false);
}
void pawSelect() {digitalWrite(sensorCSPin, LOW); }
void pawDeselect() {digitalWrite(sensorCSPin, HIGH); }
void pawWriteReg(uint8_t reg, uint8_t val) {
    pawSelect();
    SPI.transfer(reg | 0x80);
    SPI.transfer(val);
    pawDeselect();
}
void readPAW3395Motion(int &dx, int &dy) {
    dx = 0;
    dy = 0;
}



uint8_t readMouseButtons() {
 uint8_t buttons = 0;
 if  (digitalRead(leftButtonPin) == LOW) buttons |= MOUSE_BUTTON_LEFT;
 if  (digitalRead(rightButtonPin) == LOW)buttons |= MOUSE_BUTTON_RIGHT;
 if  (digitalRead(backButtonPin) == LOW)buttons |= MOUSE_BUTTON_BACKWARD;
 if  (digitalRead(frontButtonPin) == LOW)buttons |= MOUSE_BUTTON_FORWARD;
 return buttons;
}

uint8_t pawReadReg(uint8_t reg) {
    pawSelect();
    SPI.transfer(reg & 0x7f);
    delayMicroseconds(1);
    uint8_t val = SPI.transfer(0x00);
    pawDeselect();
    return val;
}


void setup() {


//*************microswitches****************
 pinMode(leftButtonPin, INPUT_PULLUP);
 pinMode(rightButtonPin, INPUT_PULLUP);
 pinMode(backButtonPin, INPUT_PULLUP);
 pinMode(frontButtonPin, INPUT_PULLUP);
 pinMode(batteryPin, INPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(sensorCSPin, OUTPUT);
  pinMode(sensorRstPin, OUTPUT);
  pinMode(sensorMotPin, INPUT_PULLUP);

//for sensor
digitalWrite(sensorCSPin, HIGH);
digitalWrite(sensorRstPin, HIGH);
SPI.begin();
 
usb_hid.setPollInterval(2);
usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
usb_hid.begin();

//for TTC encoder
Serial.begin (115200);

while (!TinyUSBDevice.mounted()) delay(1);

//*************TTC encoder******************

pinMode(encoderAPin, INPUT_PULLUP);
pinMode(encoderBPin, INPUT_PULLUP);
lastA = digitalRead(encoderAPin);

//***************build in LED***************
digitalWrite(ledRedPin, HIGH);
digitalWrite(ledGreenPin, HIGH);

}




void loop() {

if (!TinyUSBDevice.mounted()) return;
 //*************battery*********************
float vbatt = readBatteryVoltage();
showBatteryColor(vbatt);

if (vbatt <LOW_BATTERY_V) {
 usb_hid.mouseReport(0, 0, 0, 0, 0, 0);
 delay(500);
 return;
}

//****************sensor********************
int dx = 0;
int dy = 0;
readPAW3395Motion(dx, dy);
usb_hid.mouseReport(0, readMouseButtons(), dx, dy, 0, 0);

//*************TTC encoder******************
 int a = digitalRead(encoderAPin);
 int b = digitalRead(encoderBPin);

 if (a != lastA) {
    if (a == LOW) {
        if (b ==HIGH) {
          usb_hid.mouseScroll(0, 1, 0);
        }else{
            usb_hid.mouseScroll(0, -1, 0);
        }
    }
    lastA = a;
  }
}
