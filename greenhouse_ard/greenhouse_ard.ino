const int yellowLed = 4;
const int greenLed = 5;
const int redLed = 6;


void setup() {
  Serial2.begin(9600);
                  
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);


  // Initially, turn off all LEDs
  digitalWrite(yellowLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
}

void loop() {
  if (Serial2.available() > 0) {
    char command = Serial2.read();

    // Execute the received command from ESP32
    executeCommand(command);
  }
}

void executeCommand(char command) {

  bool red = command & 1;
  bool yellow = command & (1<<1);
  bool green = command & (1<<2);

  digitalWrite(redLed, red);
  digitalWrite(yellowLed, yellow);
  digitalWrite(greenLed, green);
}
