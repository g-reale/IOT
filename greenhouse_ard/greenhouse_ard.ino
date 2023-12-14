const int yellowLed = 4;
const int greenLed = 5;
const int redLed = 6;

void setup() {
  Serial.begin(9600);

  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  // Initially, turn off all LEDs
  digitalWrite(yellowLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Execute the received command from ESP32
    executeCommand(command);
  }
}

void executeCommand(char command) {
  switch (command) {
    case 'A':
      digitalWrite(redLed, HIGH);
      break;
    
    case 'B':
      digitalWrite(redLed, LOW);
      break;
    
    case 'C':
      digitalWrite(yellowLed, HIGH);
      break;
    
    case 'D':
      digitalWrite(yellowLed, LOW);
      break;
    
    case 'E':
      digitalWrite(greenLed, HIGH);
      break;
    
    case 'F':
      digitalWrite(greenLed, LOW);
      break;

    default:
      break;
  }
}
