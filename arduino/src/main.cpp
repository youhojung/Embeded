#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

#define LED_R 11  // LED가 연결된 핀
#define LED_Y 10  // LED가 연결된 핀
#define LED_G 9  // LED가 연결된 핀
#define BUTTON1 7
#define BUTTON2 6
#define BUTTON3 5
#define POT A0

int brightness = 0;    // LED 밝기 변수
int periodRed=2000;
int periodYel=500;
int periodGre=2000;
static bool BlinkG = false;
volatile bool Normal = true;
volatile bool Emergency = false;
volatile bool BlinkAll = false;
volatile bool PowerOff = false;
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

Scheduler runner;   // TaskScheduler 객체 생성

void Red();       // LED 켜기 함수
void Yel_1();
void Gre_1();
void Gre_2();
void Yel_2();
void CheckSerialInput(); // Serial 입력 확인 함수
void button1_ISR();
void button2_ISR();
void button3_ISR();
void emgc();
void BlAl();
void pwof();
void SendBrightness();

Task LEDR(0, TASK_ONCE, &Red, &runner, true);
Task LEDY1(0, TASK_ONCE, &Yel_1, &runner, false);
Task LEDG1(0, TASK_ONCE, &Gre_1, &runner, false);
Task LEDG2(166, 6, &Gre_2, &runner, false);
Task LEDY2(0, TASK_ONCE, &Yel_2, &runner, false);
Task SerialCheck( 100, TASK_FOREVER, &CheckSerialInput, &runner, true);
Task BlinkAllTask(500, TASK_FOREVER, &BlAl, &runner, false);
Task BrightnessTask(100, TASK_FOREVER, &SendBrightness, &runner, true);

void setup() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);
    Serial.begin(9600);

    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON1), button1_ISR, FALLING);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON2), button2_ISR, FALLING);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON3), button3_ISR, FALLING);

}

void loop() {
    brightness = analogRead(POT) / 4;
    runner.execute(); // TaskScheduler 실행
    if (button1Pressed) {
        button1Pressed = false;
        LEDG2.setIterations(6);
        if (Emergency == false) {
            Emergency = true;
            Normal = false;
            BlinkAll = false;
            PowerOff = false;
            runner.disableAll();
            SerialCheck.enable();
            BrightnessTask.enable();
            emgc();
        } else {
            Emergency = false;
            Normal = true;
            Serial.println("Normal Mode");
            LEDR.restart();
        }
    }

    if (button2Pressed) {
        button2Pressed = false;
        if (BlinkAll == false) {
            Emergency = false;
            Normal = false;
            BlinkAll = true;
            PowerOff = false;
            runner.disableAll();
            LEDG2.setIterations(0);
            SerialCheck.enable();
            BrightnessTask.enable();
            BlinkAllTask.restart();
        } else {
            PowerOff = false;
            Normal = true;
            BlinkAllTask.disable();
            LEDG2.setIterations(6);
            Serial.println("Normal Mode");
            analogWrite(LED_R, 0);
            Serial.println("[" + String(millis()) + "] RED LED OFF");
            analogWrite(LED_G, 0);
            Serial.println("[" + String(millis()) + "] GREEN LED OFF");
            analogWrite(LED_Y, 0);
            Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
            LEDR.restart();
        }
    }

    if (button3Pressed) {
        button3Pressed = false;
        LEDG2.setIterations(6);
        if (PowerOff == false) {
            Emergency = false;
            Normal = false;
            BlinkAll = false;
            PowerOff = true;
            runner.disableAll();
            SerialCheck.enable();
            BrightnessTask.enable();
            pwof();
        } else {
            PowerOff = false;
            Normal = true;
            Serial.println("Normal Mode");
            LEDR.restart();
        }
    }

}

void Red() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] RED LED ON");
    analogWrite(LED_R, brightness);
    LEDY1.restartDelayed(periodRed);
}

void Yel_1() {
    Serial.println("[" + String(millis()) + "] RED LED OFF");
    analogWrite(LED_R, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    analogWrite(LED_Y, brightness);
    LEDG1.restartDelayed(periodYel);
}

void Gre_1() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED ON");
    analogWrite(LED_G, brightness);
    BlinkG = true;
    LEDG2.restartDelayed(periodGre);
}

void Gre_2(){
    if(BlinkG == true){
        analogWrite(LED_G, 0);
        Serial.println("[" + String(millis()) + "] GREEN LED OFF");
        BlinkG = false;
    }
    else{
        analogWrite(LED_G, brightness);
        Serial.println("[" + String(millis()) + "] GREEN LED ON");
        BlinkG = true;
    }
    if (LEDG2.isLastIteration()) {
        LEDY2.restartDelayed(170); // 노란 LED 켜기
    }
}

void Yel_2() {
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_Y, brightness);
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    BlinkG = false;
    LEDR.restartDelayed(periodYel);
}

void CheckSerialInput() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim(); // 공백 및 개행 문자 제거

        if (input.length() > 1) { // 최소한 R/Y/G + 숫자 한 자리 이상 필요
            char type = input.charAt(0); // 첫 번째 문자 추출
            int value = input.substring(1).toInt(); // 숫자 부분 변환

            if (value > 0) { // 유효한 값인지 확인
                if (type == 'R') {
                    periodRed = value;
                    Serial.println("[" + String(millis()) + "] New periodRed: " + value);
                } else if (type == 'Y') {
                    periodYel = value;
                    Serial.println("[" + String(millis()) + "] New periodYel: " + value);
                } else if (type == 'G') {
                    periodGre = value;
                    Serial.println("[" + String(millis()) + "] New periodGre: " + value);
                }
            }
        }
    }
}

void button1_ISR(){
    button1Pressed = true;
}
void button2_ISR(){
    button2Pressed = true;
}
void button3_ISR(){
    button3Pressed = true;
}

void emgc(){
    Serial.println("Emergency Mode");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_R, brightness);
    Serial.println("[" + String(millis()) + "] RED LED ON");
}

void pwof(){
    Serial.println("PowerOff Mode");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_R, 0);
    Serial.println("[" + String(millis()) + "] RED LED OFF");
}

void BlAl(){
    Serial.println("BlinkAll Mode");
    static bool HL = false;
    if(HL == false){
        analogWrite(LED_Y, brightness);
        Serial.println("[" + String(millis()) + "] YELLOW LED ON");
        analogWrite(LED_G, brightness);
        Serial.println("[" + String(millis()) + "] GREEN LED ON");
        analogWrite(LED_R, brightness);
        Serial.println("[" + String(millis()) + "] RED LED ON");
        HL = true;
    }
    else{
        analogWrite(LED_Y, 0);
        Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
        analogWrite(LED_G, 0);
        Serial.println("[" + String(millis()) + "] GREEN LED OFF");
        analogWrite(LED_R, 0);
        Serial.println("[" + String(millis()) + "] RED LED OFF");
        HL = false;
    }
}

void SendBrightness() {
    Serial.println("Brightness: " + String(brightness));
}