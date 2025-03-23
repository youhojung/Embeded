let port;
let connectBtn;
let slider;
let circleRColor = 'gray';
let circleYColor = 'gray';
let circleGColor = 'gray';
let periodRed = 2000;
let periodYellow = 1000;
let periodGreen = 2000;
let newperiodRed = 2000;
let newperiodYel = 500;
let newperiodGre = 2000;
let Mode = "Normal";
let Brightness = 0;
let newstr = "";

function setup() {
  createCanvas(520, 500); //width, height
  background('220');

  port = createSerial(); // web serial control object

  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600);
  }

  // Web serial connect button setting
  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(60, 400);
  connectBtn.mousePressed(connectBtnClick);

  // Create a slider and place it at the top of the canvas.
  sliderR = createSlider(500, 5000, newperiodRed, 10);
  sliderR.position(10, 10);
  sliderR.size(500); // Set the width of the slider
  sliderR.mouseReleased(changeSlider); 
  
  sliderY = createSlider(500, 5000, newperiodYel, 10);
  sliderY.position(10, 25);
  sliderY.size(500); // Set the width of the slider
  sliderY.mouseReleased(changeSlider); 
  
  sliderG = createSlider(500, 5000, newperiodGre, 10);
  sliderG.position(10, 40);
  sliderG.size(500); // Set the width of the slider
  sliderG.mouseReleased(changeSlider); 

  textSize(18);
  fill(0);
}

function draw() {
  let n = port.available(); 
  if (n > 0) {
    let str = port.readUntil("\n"); 
    background(220);
    if(str.includes("[")){
      newstr = str;
    }
    
    fill(0);
    text("msg: " + newstr, 10, 200);
    
    
    // Check for LED ON/OFF messages
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
    
    if (str.includes("Brightness: ")) {
      let brightnessStr = str.split("Brightness: ")[1].trim();
      if (!isNaN(brightnessStr) && brightnessStr.length > 0) {
        Brightness = int(brightnessStr); // 문자열을 정수로 변환 후 Brightness에 저장
      }
    }
  }

  // Draw the circle
  fill(circleRColor);
  circle(100, 125, 50); // Centered circle with diameter 50
  fill (circleYColor);
  circle(175, 125, 50);
  fill (circleGColor);
  circle(250, 125, 50);
  // Change button label based on connection status
  if (!port.opened()) {
    connectBtn.html("Connect to Arduino");
  } else {
    connectBtn.html("Disconnect");
  }

  fill(0);
  text("Period Red : " + newperiodRed, 10, 250);
  fill(0);
  text("Period Yellow : " + newperiodYel, 10, 275);
  fill(0);
  text("Period Green : " + newperiodGre, 10, 300);
  fill(0);
  text("Mode : " + Mode, 10, 325);
  fill(0);
  text("Bright : " + Brightness, 10, 350);
}

function connectBtnClick() {
  if (!port.opened()) {
    port.open(9600);
  } else {
    port.close();
  }
}

function changeSlider() {
  newperiodRed = String(sliderR.value());
  newperiodYel = String(sliderY.value());
  newperiodGre = String(sliderG.value());
  if(periodRed != newperiodRed){
  periodRed = newperiodRed;
  port.write("R" + periodRed + "\n");
  }
  if(periodYellow != newperiodYel){
  periodYellow = newperiodYel;
  port.write("Y" + periodYellow + "\n");
  }
  if(periodGreen != newperiodGre){
  periodGreen = newperiodGre;
  port.write("G" + periodGreen + "\n");
  }
}
 