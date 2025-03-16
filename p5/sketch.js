let port;
let connectBtn;
let sliderRed, sliderYellow, sliderGreen;
let circleColorRed = 'gray';
let circleColorYellow = 'gray';
let circleColorGreen = 'gray';
let periodRed = 2000; // default period for red LED
let periodYellow = 500; // default period for yellow LED
let periodGreen = 2000; // default period for green LED
let msgRed = "";
let msgYellow = "";
let msgGreen = "";
let buttonState = "NORMAL";

function setup() {
  createCanvas(520, 400); //width, height
  background('220');

  port = createSerial(); // web serial control object

  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600);
  }

  // Web serial connect button setting
  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(350, 60);
  connectBtn.mousePressed(connectBtnClick);

  // Create sliders and place them at the top of the canvas.
  sliderRed = createSlider(0, 4000, periodRed, 10);
  sliderRed.position(10, 10);
  sliderRed.size(500); // Set the width of the slider
  sliderRed.mouseReleased(changeSliderRed); 

  sliderYellow = createSlider(0, 1000, periodYellow, 10);
  sliderYellow.position(10, 40);
  sliderYellow.size(500); // Set the width of the slider
  sliderYellow.mouseReleased(changeSliderYellow); 

  sliderGreen = createSlider(0, 4000, periodGreen, 10);
  sliderGreen.position(10, 70);
  sliderGreen.size(500); // Set the width of the slider
  sliderGreen.mouseReleased(changeSliderGreen); 

  textSize(18);
  fill(0);
}

function draw() {
  let n = port.available(); 
  if (n > 0) {
    let str = port.readUntil("\n"); 
    background(220);
    fill(0);
    
    // Check for LED ON/OFF messages
    if (str.includes("RED LED ON")) {
      circleColorRed = 'red';
      msgRed = str;
    } else if (str.includes("RED LED OFF")) {
      circleColorRed = 'gray';
      msgRed = str;
    }
    
    if (str.includes("YELLOW LED ON")) {
      circleColorYellow = 'yellow';
      msgYellow = str;
    } else if (str.includes("YELLOW LED OFF")) {
      circleColorYellow = 'gray';
      msgYellow = str;
    }
    
    if (str.includes("GREEN LED ON")) {
      circleColorGreen = 'green';
      msgGreen = str;
    } else if (str.includes("GREEN LED OFF")) {
      circleColorGreen = 'gray';
      msgGreen = str;
    }

    // Check for button state messages
    if (str.includes("NORMAL")) {
      buttonState = "NORMAL";
    } else if (str.includes("EMERGENCY")) {
      buttonState = "EMERGENCY";
    } else if (str.includes("BLINK ALL")) {
      buttonState = "BLINK ALL";
    } else if (str.includes("ON OFF")) {
      buttonState = "ON OFF";
    }
  }

  // Draw the circles
  fill(circleColorRed);
  circle(100, 150, 50); // Red LED circle
  fill(circleColorYellow);
  circle(200, 150, 50); // Yellow LED circle
  fill(circleColorGreen);
  circle(300, 150, 50); // Green LED circle

  // Change button label based on connection status
  if (!port.opened()) {
    connectBtn.html("Connect to Arduino");
  } else {
    connectBtn.html("Disconnect");
  }

  fill(0);
  text("Red Period: " + periodRed, 10, 250);
  text("Yellow Period: " + periodYellow, 10, 270);
  text("Green Period: " + periodGreen, 10, 290);
  text("Red Msg: " + msgRed, 10, 310);
  text("Yellow Msg: " + msgYellow, 10, 330);
  text("Green Msg: " + msgGreen, 10, 350);
  text("Button State: " + buttonState, 10, 370);
}

function connectBtnClick() {
  if (!port.opened()) {
    port.open(9600);
  } else {
    port.close();
  }
}

function changeSliderRed() {
  periodRed = String(sliderRed.value());
  port.write("R" + periodRed + "\n");
}

function changeSliderYellow() {
  periodYellow = String(sliderYellow.value());
  port.write("Y" + periodYellow + "\n");
}

function changeSliderGreen() {
  periodGreen = String(sliderGreen.value());
  port.write("G" + periodGreen + "\n");
}