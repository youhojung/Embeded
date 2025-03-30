let port;
let connectBtn;
let sliderRed, sliderYellow, sliderGreen;
let sliderBrightness;
let circleRColor = 'gray';
let circleYColor = 'gray';
let circleGColor = 'gray';
let periodRed = 2000;
let periodYellow = 500;
let periodGreen = 2000;
let buttonState = "Normal";
let Brightness = 0;
let Mode = "Normal";
let video;
let handPose;
let hands = [];
let thumbF, indexF, middleF, ringF, pinkyF;
let fingerState;
let preState;
// handPose 모델 불러오기
function preload() {
  handPose = ml5.handPose();
}
// 손 검출 시작하고 콜백하기
function gotHands(results) {
  hands = results;
}

function setup() {
  createCanvas(640, 600);
  background(220);
  // 비디오 기불러오기
  video = createCapture(VIDEO, { flipped: true });
  video.size(320, 180);
  video.hide();
  
  handPose.detectStart(video, gotHands);
  // 시리얼 포트 연결하기
  port = createSerial();
  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600);
  }
  // 아두이노 연결 버튼성생성
  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(375, 340);
  connectBtn.mousePressed(connectBtnClick);
  // 빨간 LED의 주기를 조절 가능한 슬라이더. 최소 1000, 최대 3000, 초기값 2000
  sliderRed = createSlider(1000, 3000, periodRed, 10);
  sliderRed.position(10, 190);
  sliderRed.size(500);
  sliderRed.mouseReleased(changeSliderRed); 
  // 노란 LED의 주기를 조절 가능한 슬라이더. 최소 250, 최대 750, 초기값 500
  sliderYellow = createSlider(250, 750, periodYellow, 10);
  sliderYellow.position(10, 220);
  sliderYellow.size(500);
  sliderYellow.mouseReleased(changeSliderYellow); 
  // 초록 LED의 주기를 조절 가능한 슬라이더. 최소 1000, 최대 3000, 초기값 2000
  sliderGreen = createSlider(1000, 3000, periodGreen, 10);
  sliderGreen.position(10, 250);
  sliderGreen.size(500);
  sliderGreen.mouseReleased(changeSliderGreen); 
  // LED의 밝기를 보여주는 슬라이더. 조절 불가능.
  sliderBrightness = createSlider(0, 255, 0, 1);
  sliderBrightness.position(10, 280);
  sliderBrightness.size(500);
  sliderBrightness.attribute('disabled', '');
}

function draw() {
  background(220);
  // 빨간색, 노란색, 초록색 LED의 주기와 현재 모드, 현재 밝기를 출력
  noStroke();
  fill(0);
  textSize(18);
  text("Red Period: " + periodRed, 10, 420);
  text("Yellow Period: " + periodYellow, 10, 440);
  text("Green Period: " + periodGreen, 10, 460);
  text("Button State: " + Mode, 10, 480);
  text("Brightness: " + Brightness, 10, 500);
  // LED의 상태를 나타내는 원을 색칠.
  stroke(0);
  fill(circleRColor);
  circle(100, 350, 50);
  fill(circleYColor);
  circle(200, 350, 50);
  fill(circleGColor);
  circle(300, 350, 50);
  // 시리얼 통신값 읽기
  let n = port.available(); 
  if (n > 0) {
    let str = port.readUntil("\n"); 
    if(str.includes("[")){
      newstr = str;
    }
    fill(0);
    // LED의 켜짐/꺼짐 신호를 받고, 그에 따라 색을 변화시킨다.
    if (str.includes("RED LED ON")) {
      circleRColor = 'red';
    } else if (str.includes("RED LED OFF")) {
      circleRColor = 'gray';
    }
    
    if (str.includes("YELLOW LED ON")) {
      circleYColor = 'yellow';
    } else if (str.includes("YELLOW LED OFF")) {
      circleYColor = 'gray';
    }
    
    if (str.includes("GREEN LED ON")) {
      circleGColor = 'green';
    } else if (str.includes("GREEN LED OFF")) {
      circleGColor = 'gray';
    }
    // 시리얼이 모드에 관한 통신이라면, 그 값을 Mode에 저장한다.
    if(str.includes("Emergency Mode")){
      Mode = "Emergency";
    }
    if(str.includes("BlinkAll Mode")){
      Mode = "BlinkAll";
    }
    if(str.includes("PowerOff Mode")){
      Mode = "PowerOff";
    }
    if(str.includes("Normal Mode")){
      Mode = "Normal";
    }
    // 시리얼 통신이 밝기에 관해있다면, Brightness에 저장한다.
    if (str.includes("Brightness: ")) {
      let brightnessStr = str.split("Brightness: ")[1].trim();
      if (!isNaN(brightnessStr) && brightnessStr.length > 0) {
        Brightness = int(brightnessStr); // 문자열을 정수로 변환 후 Brightness에 저장
      }
    }
  }
  // 비디오 불러오기
  image(video, 0, 0);
  for (let i = 0; i < hands.length; i++) {
    let hand = hands[i];
    // 손의 각 keypoint에 빨간 점 그리기
    for (let j = 0; j < hand.keypoints.length; j++) {
      let keypoint = hand.keypoints[j];
      fill(255, 0, 0);
      noStroke();
      circle(320 - keypoint.x, keypoint.y, 10);
    }
    // 필요한 keypoint 불러오기
    let tip1 = hand.thumb_tip, mcp1 = hand.thumb_mcp, cmc1 = hand.thumb_cmc;
    let tip2 = hand.index_finger_tip, pip2 = hand.index_finger_pip, mcp2 = hand.index_finger_mcp;
    let tip3 = hand.middle_finger_tip, pip3 = hand.middle_finger_pip, mcp3 = hand.middle_finger_mcp;
    let tip4 = hand.ring_finger_tip, pip4 = hand.ring_finger_pip, mcp4 = hand.ring_finger_mcp;
    let tip5 = hand.pinky_finger_tip, pip5 = hand.pinky_finger_pip, mcp5 = hand.pinky_finger_mcp;
    // 엄지를 제외한 손가락은 pip를 기준으로 꺾인다.
    // 따라서 (tip.y - pip.y) * (pip.y - mcp.y)의 값이 양수라면 펴진 것이고
    // (tip.y - pip.y) * (pip.y - mcp.y)의 값이 음수라면 구부러진 것이다.
    // 엄지는 다른 손가락과 구동 방식이 달라 y축이 아닌 x축에서 dramatic한 변화가 있다.
    // mcp와 cmc의 상대적 위치는 손을 구부리던지 펴던지 거의 변하지 않는다.
    // 그러나 tip와 mcp의 상대적 위치는 크게 변한다.
    // 따라서 (tip1.x - mcp1.x) * (mcp1.x - cmc1.x)가 양수면 펴져있을 것이고
    // 그렇지 않다면 구부러져 있을 것이다.
    thumbF = (tip1.x - mcp1.x) * (mcp1.x - cmc1.x) > 0 ? 1: 0;
    indexF = (tip2.y - pip2.y) * (pip2.y - mcp2.y) > 0 ? 1 : 0;
    middleF = (tip3.y - pip3.y) * (pip3.y - mcp3.y) > 0 ? 1 : 0;
    ringF = (tip4.y - pip4.y) * (pip4.y - mcp4.y) > 0 ? 1 : 0;
    pinkyF = (tip5.y - pip5.y) * (pip5.y - mcp5.y) > 0 ? 1 : 0;
    // 제대로 구동이 되는지 확인하기 위한 콘솔 로그 작성
    console.log(thumbF, indexF, middleF, ringF, pinkyF);
    // fingerState의 변화를 알아보기
    whatstate();
  }
}
// 버튼을 눌러 포트와 연결 혹은 연결 해제
function connectBtnClick() {
  if (!port.opened()) {
    port.open(9600);
  } else {
    port.close();
  }
}
// 빨간 LED의 슬라이더가 변했을 경우, 시리얼 통신을 보낸다.
function changeSliderRed() {
  periodRed = String(sliderRed.value());
  port.write("R" + periodRed + "\n");
}
// 노란 LED의 슬라이더가 변했을 경우, 시리얼 통신을 보낸다.
function changeSliderYellow() {
  periodYellow = String(sliderYellow.value());
  port.write("Y" + periodYellow + "\n");
}
//  LED의 슬라이더가 변했을 경우, 시리얼 통신을 보낸다.
function changeSliderGreen() {
  periodGreen = String(sliderGreen.value());
  port.write("G" + periodGreen + "\n");
}

function whatstate() { // 손가락의 상태에 따라 아두이노에 반환할 값을 정한다.
  // fingerState는 손가락의 상태를 나타내는 변수
  // fingerState는 각각 0, 1, 2, 3의 값을 가질 수 있다.
  // 검지와 중지만 펴져있다면, 0을 반환
  if (thumbF == 0 && indexF == 1 && middleF == 1 && ringF == 0 && pinkyF == 0) {
    fingerState = 0;
  }
  // 모든 손가락이 펴져있다면, 1을 반환
  if (thumbF == 1 && indexF == 1 && middleF == 1 && ringF == 1 && pinkyF == 1) {
    fingerState = 1;
  } 
  // 검지, 중지, 약지만 펴져있다면, 2를 반환
  if (thumbF == 0 && indexF == 1 && middleF == 1 && ringF == 1 && pinkyF == 0) {
    fingerState = 2;
  }
  // 모든 손가락이 구부러져 있다면, 3을 반환
  if (thumbF == 0 && indexF == 0 && middleF == 0 && ringF == 0 && pinkyF == 0) {
    fingerState = 3;
  }
  // 불필요한 통신을 줄이기 위해 fingerState가 달라질 때에만 시리얼 통신을 보낸다.
  if(preState != fingerState){
    port.write("Finger" + fingerState + "\n");
    preState = fingerState;
  }
}