#include <Arduino.h>
#include <TaskScheduler.h>

#define LED_PIN_RED 13  // 빨간 LED가 연결된 핀
#define LED_PIN_YELLOW 12  // 노란 LED가 연결된 핀
#define LED_PIN_GREEN 8  // 초록 LED가 연결된 핀
#define BUTTON_PIN_1 7  // 첫 번째 버튼 핀
#define BUTTON_PIN_2 6  // 두 번째 버튼 핀
#define BUTTON_PIN_3 5  // 세 번째 버튼 핀

Scheduler runner;   // TaskScheduler 객체 생성

void LedOnRed();       // 빨간 LED 켜기 함수
void LedOffRed();      // 빨간 LED 끄기 함수
void LedOnYellow();    // 노란 LED 켜기 함수
void LedOffYellow();   // 노란 LED 끄기 함수
void LedOnGreen();     // 초록 LED 켜기 함수
void LedOffGreen();    // 초록 LED 끄기 함수
void BlinkGreen();     // 초록 LED 깜빡이기 함수
void CheckSerialInput(); // Serial 입력 확인 함수
void CheckButtonInput(); // 버튼 입력 확인 함수
void StartRed();       // 빨간 LED 주기를 시작하는 함수
void LedOnYellow2();   // 노란 LED2 켜기 함수
void LedOffYellow2();  // 노란 LED2 끄기 함수
void EmergencyMode();  // Emergency 모드 함수
void BlinkAllMode();   // Blink All 모드 함수
void OnOffMode();      // On Off 모드 함수

int periodRed = 2000; // 빨간 LED 주기
int periodYellow = 500; // 노란 LED 주기
int periodGreen = 2000; // 초록 LED 주기

enum State { NORMAL, EMERGENCY, BLINK_ALL, ON_OFF };
volatile State currentState = NORMAL;

Task taskRed(periodRed, TASK_ONCE, &LedOnRed,  &runner, true); 
Task taskYellow(periodYellow, TASK_ONCE, &LedOnYellow,  &runner, false); 
Task taskGreen(periodGreen, TASK_ONCE, &LedOnGreen,  &runner, false); 
Task taskBlinkGreen(333, 6, &BlinkGreen, &runner, false); // 1초에 3번 깜빡임
Task taskYellow2(periodYellow, TASK_ONCE, &LedOnYellow2,  &runner, false);
Task taskOffYellow2(0, TASK_ONCE, &LedOffYellow2, &runner, false);
Task taskOffRed(0, TASK_ONCE, &LedOffRed, &runner, false);
Task taskOffYellow(0, TASK_ONCE, &LedOffYellow, &runner, false);
Task taskOffGreen(0, TASK_ONCE, &LedOffGreen, &runner, false);
Task taskStartRed(0, TASK_ONCE, &StartRed, &runner, false); // 빨간 LED 켜기
Task taskEmergency(0, TASK_FOREVER, &EmergencyMode, &runner, false);
Task taskBlinkAll(500, TASK_FOREVER, &BlinkAllMode, &runner, false);
Task taskOnOff(0, TASK_FOREVER, &OnOffMode, &runner, false);
Task task3(100, TASK_FOREVER, &CheckSerialInput, &runner, true);
Task taskCheckButton(100, TASK_FOREVER, &CheckButtonInput, &runner, true); // 버튼 입력 확인 Task 추가

void setup() {
    pinMode(LED_PIN_RED, OUTPUT);
    pinMode(LED_PIN_YELLOW, OUTPUT);
    pinMode(LED_PIN_GREEN, OUTPUT);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);
    pinMode(BUTTON_PIN_3, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop() {
    runner.execute(); // TaskScheduler 실행
}

void LedOnRed() {
    Serial.println("[" + String(millis()) + "] RED LED ON");
    digitalWrite(LED_PIN_RED, HIGH);
    taskOffRed.restartDelayed(periodRed);
    taskYellow.restartDelayed(periodRed); // 노란 LED 켜기
}

void LedOffRed() {
    Serial.println("[" + String(millis()) + "] RED LED OFF");
    digitalWrite(LED_PIN_RED, LOW);
}

void LedOnYellow() {
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    digitalWrite(LED_PIN_YELLOW, HIGH);
    taskOffYellow.restartDelayed(periodYellow);
    taskGreen.restartDelayed(periodYellow); // 초록 LED 켜기
}

void LedOffYellow() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    digitalWrite(LED_PIN_YELLOW, LOW);
}

void LedOnGreen() {
    Serial.println("[" + String(millis()) + "] GREEN LED ON");
    digitalWrite(LED_PIN_GREEN, HIGH);
    taskOffGreen.restartDelayed(periodGreen);
    taskBlinkGreen.restartDelayed(periodGreen); // 초록 LED 깜빡이기 시작
}

void LedOffGreen() {
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    digitalWrite(LED_PIN_GREEN, LOW);
}

void BlinkGreen() {
    static bool isOn = false;
    if (isOn) {
        Serial.println("[" + String(millis()) + "] GREEN LED OFF");
        digitalWrite(LED_PIN_GREEN, LOW);
    } else {
        Serial.println("[" + String(millis()) + "] GREEN LED ON");
        digitalWrite(LED_PIN_GREEN, HIGH);
    }
    isOn = !isOn;
    if (taskBlinkGreen.isLastIteration()) {
        digitalWrite(LED_PIN_GREEN, LOW);
        taskYellow2.restartDelayed(0); // 노란 LED 켜기
    }
}

void LedOnYellow2() {
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    digitalWrite(LED_PIN_YELLOW, HIGH);
    taskOffYellow2.restartDelayed(periodYellow);
}

void LedOffYellow2() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    digitalWrite(LED_PIN_YELLOW, LOW);
    taskRed.restartDelayed(0); // 빨간 LED 켜기
}

void StartRed() {
    taskRed.restart();
}

void EmergencyMode() {
    Serial.println("EMERGENCY MODE");
    digitalWrite(LED_PIN_RED, HIGH);
    digitalWrite(LED_PIN_YELLOW, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
    Serial.println("RED LED ON");
    Serial.println("YELLOW LED OFF");
    Serial.println("GREEN LED OFF");
}

void BlinkAllMode() {
    static bool isOn = false;
    if (isOn) {
        Serial.println("BLINK ALL OFF");
        digitalWrite(LED_PIN_RED, LOW);
        digitalWrite(LED_PIN_YELLOW, LOW);
        digitalWrite(LED_PIN_GREEN, LOW);
        Serial.println("RED LED OFF");
        Serial.println("YELLOW LED OFF");
        Serial.println("GREEN LED OFF");
    } else {
        Serial.println("BLINK ALL ON");
        digitalWrite(LED_PIN_RED, HIGH);
        digitalWrite(LED_PIN_YELLOW, HIGH);
        digitalWrite(LED_PIN_GREEN, HIGH);
        Serial.println("RED LED ON");
        Serial.println("YELLOW LED ON");
        Serial.println("GREEN LED ON");
    }
    isOn = !isOn;
}

void OnOffMode() {
    Serial.println("ON OFF MODE");
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
    Serial.println("RED LED OFF");
    Serial.println("YELLOW LED OFF");
    Serial.println("GREEN LED OFF");
}

void CheckSerialInput() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        int newPeriod = input.substring(1).toInt();
        if (newPeriod > 0) {
            if (input.startsWith("R")) {
                periodRed = newPeriod;
                taskRed.setInterval(periodRed);
                Serial.println("[" + String(millis()) + "] New RED task period: " + newPeriod);
            } else if (input.startsWith("Y")) {
                periodYellow = newPeriod;
                taskYellow.setInterval(periodYellow);
                taskYellow2.setInterval(periodYellow);
                Serial.println("[" + String(millis()) + "] New YELLOW task period: " + newPeriod);
            } else if (input.startsWith("G")) {
                periodGreen = newPeriod;
                taskGreen.setInterval(periodGreen);
                Serial.println("[" + String(millis()) + "] New GREEN task period: " + newPeriod);
            }
        }
    }
}

void CheckButtonInput() {
    static unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 200;

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (digitalRead(BUTTON_PIN_1) == LOW) {
            if (currentState == EMERGENCY) {
                currentState = NORMAL;
                Serial.println("RETURN TO NORMAL");
                digitalWrite(LED_PIN_RED, LOW);
                digitalWrite(LED_PIN_YELLOW, LOW);
                digitalWrite(LED_PIN_GREEN, LOW);
                taskEmergency.disable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskRed.enable();
                taskRed.restart();
            } else {
                currentState = EMERGENCY;
                Serial.println("EMERGENCY");
                taskEmergency.enable();
                taskBlinkAll.disable();
                taskOnOff.disable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskEmergency.restart();
            }
            lastDebounceTime = millis();
        } else if (digitalRead(BUTTON_PIN_2) == LOW) {
            if (currentState == BLINK_ALL) {
                currentState = NORMAL;
                Serial.println("RETURN TO NORMAL");
                digitalWrite(LED_PIN_RED, LOW);
                digitalWrite(LED_PIN_YELLOW, LOW);
                digitalWrite(LED_PIN_GREEN, LOW);
                taskBlinkAll.disable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskRed.enable();
                taskRed.restart();
            } else {
                currentState = BLINK_ALL;
                Serial.println("BLINK ALL");
                taskEmergency.disable();
                taskBlinkAll.enable();
                taskOnOff.disable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskBlinkAll.restart();
            }
            lastDebounceTime = millis();
        } else if (digitalRead(BUTTON_PIN_3) == LOW) {
            if (currentState == ON_OFF) {
                currentState = NORMAL;
                Serial.println("RETURN TO NORMAL");
                digitalWrite(LED_PIN_RED, LOW);
                digitalWrite(LED_PIN_YELLOW, LOW);
                digitalWrite(LED_PIN_GREEN, LOW);
                taskOnOff.disable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskRed.enable();
                taskRed.restart();
            } else {
                currentState = ON_OFF;
                Serial.println("ON OFF");
                taskEmergency.disable();
                taskBlinkAll.disable();
                taskOnOff.enable();
                taskRed.disable();
                taskYellow.disable();
                taskGreen.disable();
                taskBlinkGreen.disable();
                taskYellow2.disable();
                taskOffYellow2.disable();
                taskOffRed.disable();
                taskOffYellow.disable();
                taskOffGreen.disable();
                taskStartRed.disable();
                taskOnOff.restart();
            }
            lastDebounceTime = millis();
        }
    }
}