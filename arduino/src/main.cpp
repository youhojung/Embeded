#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>
// LED 핀 번호
#define LED_R 11
#define LED_Y 10
#define LED_G 9
// 버튼 핀 번호
#define BUTTON1 7
#define BUTTON2 6
#define BUTTON3 5
// 가변저항 핀 번호
#define POT A0
int brightness = 0;    // LED 밝기 변수
int periodRed=2000;    // 빨간 LED 점멸 주기
int periodYel=500;   // 노란 LED 점멸 주기
int periodGre=2000;   // 초록 LED 점멸 주기
static bool BlinkG = false;     // 초록 LED 점멸 상태 관련 변수. true: LED가 켜져있음, false: LED가 꺼져있음
// 현재 Mode에 관한 변수. true면 활성화, false면 비활성화
volatile bool Normal = true;
volatile bool Emergency = false;
volatile bool BlinkAll = false;
volatile bool PowerOff = false;
// 버튼 입력 관련 변수. true면 버튼이 눌렸다는 뜻
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

Scheduler runner;   // TaskScheduler 객체 생성

void Red();
void Yel_1();
void Gre_1();
void Gre_2();
void Yel_2();
void CheckSerialInput();
void button1_ISR();
void button2_ISR();
void button3_ISR();
void emgc();
void BlAl();
void pwof();
void SendBrightness();
// TaskScheduler 객체 생성 및 Task 설정
Task LEDR(0, TASK_ONCE, &Red, &runner, true);
Task LEDY1(0, TASK_ONCE, &Yel_1, &runner, false);
Task LEDG1(0, TASK_ONCE, &Gre_1, &runner, false);
Task LEDG2(166, 6, &Gre_2, &runner, false);
Task LEDY2(0, TASK_ONCE, &Yel_2, &runner, false);
Task SerialCheck( 100, TASK_FOREVER, &CheckSerialInput, &runner, true);
Task BlinkAllTask(500, TASK_FOREVER, &BlAl, &runner, false);
Task BrightnessTask(100, TASK_FOREVER, &SendBrightness, &runner, true);

void setup() {
    // LED 핀을 출력으로 설정
    pinMode(LED_R, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);
    // 버튼 핀을 입력으로 설정
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);
    // 시리얼 통신 시작
    Serial.begin(9600);
    // 버튼 인터럽트 설정
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON1), button1_ISR, FALLING);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON2), button2_ISR, FALLING);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON3), button3_ISR, FALLING);

}

void loop() {
    // 가변저항은 0~1023의 값을 가지므로, 0~255로 변환
    brightness = analogRead(POT) / 4;
    runner.execute(); // TaskScheduler 실행
    if (button1Pressed) {
        // 버튼 1이 눌렸을 때의 동작
        // 버튼이 눌린 상태를 초기화
        button1Pressed = false;
        // LEDG2의 반복 횟수를 6으로 설정
        LEDG2.setIterations(6);
        if (Emergency == false) { // Emergency가 비활성화 상태일 때
            // Emergency 모드를 활성화하고, 나머지 모드를 비활성화
            Emergency = true;
            Normal = false;
            BlinkAll = false;
            PowerOff = false;
            // 시리얼 통신으로 Emergency 모드 진입 알림
            Serial.println("Emergency Mode");
            // LEDG2의 반복 횟수를 0으로 만들고, 모든 Task 비활성화
            LEDG2.setIterations(0);
            runner.disableAll();
            // 필요한 기능만 다시 활성화
            SerialCheck.enable();
            BrightnessTask.enable();
            emgc();
        } else { // Emergency가 활성화 상태일 때
            // Emergency 모드를 비활성화하고, Normal 모드를 활성화
            Emergency = false;
            Normal = true;
            // 시리얼 통신으로 Normal 모드 진입 알림
            Serial.println("Normal Mode");
            // 다시 기본 LED 상태로 돌아가기
            LEDR.restart();
        }
    }

    if (button2Pressed) { // 버튼 2가 눌렸을 때의 동작
        // 버튼이 눌린 상태를 초기화
        button2Pressed = false;
        if (BlinkAll == false) { // BlinkAll이 비활성화 상태일 때
            // BlinkAll 모드를 활성화하고, 나머지 모드를 비활성화
            Emergency = false;
            Normal = false;
            BlinkAll = true;
            PowerOff = false;
            // 시리얼 통신으로 BlinkAll 모드 진입 알림
            Serial.println("BlinkAll Mode");
            // LEDG2의 반복 횟수를 0으로 만들고, 모든 Task를 비활성화
            LEDG2.setIterations(0);
            runner.disableAll();
            // 필요한 기능만 다시 활성화
            SerialCheck.enable();
            BrightnessTask.enable();
            // BlinkAll 모드의 LED 상태 설정
            BlinkAllTask.restart();
        } else { // BlinkAll이 활성화 상태일 때
            // BlinkAll 모드를 비활성화하고, Normal 모드를 활성화
            PowerOff = false;
            Normal = true;
            // BlinkAll을 실행하는 Task를 비활성화하고, LEDG2의 반복 횟수를 6으로 설정
            BlinkAllTask.disable();
            LEDG2.setIterations(6);
            // 시리얼 통신으로 Normal 모드 진입 알림
            Serial.println("Normal Mode");
            // LED를 모두 끈다.
            analogWrite(LED_R, 0);
            Serial.println("[" + String(millis()) + "] RED LED OFF");
            analogWrite(LED_G, 0);
            Serial.println("[" + String(millis()) + "] GREEN LED OFF");
            analogWrite(LED_Y, 0);
            Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
            // 기본 LED 상태로 돌아가기
            LEDR.restart();
        }
    }

    if (button3Pressed) { // 버튼 3이 눌렸을 때의 동작
        // 버튼이 눌린 상태를 초기화
        button3Pressed = false;
        // LEDG2의 반복 횟수를 0으로 설정정
        LEDG2.setIterations(0);
        if (PowerOff == false) { // PowerOff가 비활성화 상태일 때
            // PowerOff 모드를 활성화하고, 나머지 모드를 비활성화
            Emergency = false;
            Normal = false;
            BlinkAll = false;
            PowerOff = true;
            // 시리얼 통신으로 PowerOff 모드 진입 알림
            Serial.println("PowerOff Mode");
            // 모든 Task를 비활성화
            runner.disableAll();
            // 필요한 기능만 다시 활성화
            SerialCheck.enable();
            BrightnessTask.enable();
            // PowerOff 모드의 LED 상태 설정
            pwof();
        } else { // PowerOff가 활성화 상태일 때
            // PowerOff 모드를 비활성화하고, Normal 모드를 활성화
            PowerOff = false;
            Normal = true;
            // 시리얼 통신으로 Normal 모드 진입 알림
            Serial.println("Normal Mode");
            // LEDG2의 반복 횟수를 6으로 설정
            LEDG2.setIterations(6);
            // 기본 LED 상태로 돌아가기
            LEDR.restart();
        }
    }

}
// 빨간 LED를 켜는 함수
void Red() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] RED LED ON");
    analogWrite(LED_R, brightness);
    LEDY1.restartDelayed(periodRed);
}
// 빨간 LED를 끄고, 노란 LED를 켜는 함수
void Yel_1() {
    Serial.println("[" + String(millis()) + "] RED LED OFF");
    analogWrite(LED_R, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    analogWrite(LED_Y, brightness);
    LEDG1.restartDelayed(periodYel);
}
// 노란 LED를 끄고, 초록 LED를 켜는 함수
void Gre_1() {
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED ON");
    analogWrite(LED_G, brightness);
    // 초록 LED 점멸 상태를 true로 설정
    // 초록 LED가 켜져있는 상태로 넘어가기 때문에, LED를 끄기 위해 true로 설정
    BlinkG = true;
    LEDG2.restartDelayed(periodGre);
}
// 초록 LED를 점멸시키는 함수. 
void Gre_2(){
    if(BlinkG == true){ // 초록 LED가 켜져있을 때
        // LED를 끄고, 점멸 상태를 false로 설정
        analogWrite(LED_G, 0);
        Serial.println("[" + String(millis()) + "] GREEN LED OFF");
        BlinkG = false;
    }
    else{ // 초록 LED가 꺼져있을 때
        // LED를 켜고, 점멸 상태를 true로 설정
        analogWrite(LED_G, brightness);
        Serial.println("[" + String(millis()) + "] GREEN LED ON");
        BlinkG = true;
    }
    if (LEDG2.isLastIteration()) { // 마지막 반복일 때
        LEDY2.restartDelayed(170); // LEDY2로 넘어가기 위해 170ms 대기
        // 166ms * 5 + 170ms = 1000ms이므로, 1초간 3번 깜빡이고 LEDY2로 넘어감
    }
}
// 초록 LED를 끄고, 노란 LED를 켜는 함수
void Yel_2() {
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_Y, brightness);
    Serial.println("[" + String(millis()) + "] YELLOW LED ON");
    // 다음 반복을 위해 BlinkG를 false로 설정
    BlinkG = false;
    LEDR.restartDelayed(periodYel);
}
// 시리얼 입력을 체크하는 함수
// 시리얼 입력이 있을 때마다 호출됨
void CheckSerialInput() {
    if (Serial.available() > 0) { // 시리얼 입력이 있을 때
        String input = Serial.readStringUntil('\n'); // '\n'까지 읽음
        input.trim(); // 공백 및 개행 문자 제거

        if (input.length() > 1) { // 최소한 1글자 이상 입력되었을 때
            if(input.startsWith("Finger")){ // "Finger"로 시작하는 경우
                int value = input.substring(6).toInt(); // "Finger" 이후의 숫자 부분 변환
                if(value == 0){ // 0일 때 Normal 모드로 전환
                    // 이미 누른 버튼과 동일한 버튼을 눌렀을때와 동일한 기능능 
                    Emergency = false;
                    BlinkAll = false;
                    PowerOff = false;
                    Normal = true;
                    LEDG2.setIterations(0);
                    runner.disableAll();
                    LEDG2.setIterations(6);
                    SerialCheck.enable();
                    BrightnessTask.enable();
                    Serial.println("Normal Mode");
                    analogWrite(LED_R, 0);
                    Serial.println("[" + String(millis()) + "] RED LED OFF");
                    analogWrite(LED_G, 0);
                    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
                    analogWrite(LED_Y, 0);
                    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
                    LEDR.restart();
                }
                else if(value == 1){ // Emergency 모드로 전환.
                    // Emergency 모드가 아닐 때 버튼 1을 눌렀을 때와 동일한 기능
                    Emergency = true;
                    Normal = false;
                    BlinkAll = false;
                    PowerOff = false;
                    LEDG2.setIterations(0);
                    runner.disableAll();
                    LEDG2.setIterations(6);
                    SerialCheck.enable();
                    BrightnessTask.enable();
                    Serial.println("Emergency Mode");
                    emgc();
                }
                else if(value == 2){ // BlinkAll 모드로 전환.
                    // BlinkAll 모드가 아닐 때 버튼 2를 눌렀을 때와 동일한 기능
                    Emergency = false;
                    Normal = false;
                    BlinkAll = true;
                    PowerOff = false;
                    LEDG2.setIterations(0);
                    runner.disableAll();
                    LEDG2.setIterations(6);
                    SerialCheck.enable();
                    BrightnessTask.enable();
                    Serial.println("BlinkAll Mode");
                    BlinkAllTask.restart();
                }
                else if(value == 3){ // PowerOff 모드로 전환.
                    // PowerOff 모드가 아닐 때 버튼 3을 눌렀을 때와 동일한 기능
                    Emergency = false;
                    Normal = false;
                    BlinkAll = false;
                    PowerOff = true;
                    runner.disableAll();
                    LEDG2.setIterations(0);
                    SerialCheck.enable();
                    BrightnessTask.enable();
                    Serial.println("PowerOff Mode");
                    pwof();
                }
            }
            else{ // "Finger"로 시작하지 않는 경우
                // 첫 번째 문자를 LED 종류로 판단하고, 나머지 부분을 숫자로 변환
                char type = input.charAt(0); // 첫 번째 문자 추출
                int value = input.substring(1).toInt(); // 숫자 부분 변환

                if (value > 0) { // 유효한 값인지 확인
                    if (type == 'R') { // 빨간 LED의 주기 설정
                        periodRed = value;
                        Serial.println("[" + String(millis()) + "] New periodRed: " + value);
                    } else if (type == 'Y') { // 노란 LED의 주기 설정
                        periodYel = value;
                        Serial.println("[" + String(millis()) + "] New periodYel: " + value);
                    } else if (type == 'G') { // 초록 LED의 주기 설정
                        periodGre = value;
                        Serial.println("[" + String(millis()) + "] New periodGre: " + value);
                    }
                }
            }
        }
    }
}

void button1_ISR(){ // 버튼 1 인터럽트 서비스 루틴
    button1Pressed = true;
}
void button2_ISR(){ // 버튼 2 인터럽트 서비스 루틴
    button2Pressed = true;
}
void button3_ISR(){ // 버튼 3 인터럽트 서비스 루틴
    button3Pressed = true;
}
// Emergency 모드에서 LED 상태 설정
void emgc(){
    Serial.println("Emergency Mode");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_R, brightness);
    Serial.println("[" + String(millis()) + "] RED LED ON");
}
// PowerOff 모드에서 LED 상태 설정
void pwof(){
    Serial.println("PowerOff Mode");
    analogWrite(LED_Y, 0);
    Serial.println("[" + String(millis()) + "] YELLOW LED OFF");
    analogWrite(LED_G, 0);
    Serial.println("[" + String(millis()) + "] GREEN LED OFF");
    analogWrite(LED_R, 0);
    Serial.println("[" + String(millis()) + "] RED LED OFF");
}
// BlinkAll 모드에서 LED 상태 설정
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
// LED 밝기를 시리얼로 전송하는 함수
void SendBrightness() {
    Serial.println("Brightness: " + String(brightness));
}