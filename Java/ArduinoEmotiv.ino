int green = 2;
int yellow = 3;
int red = 4;
int blue = 5;
int in;

int greenHL = LOW;
int yellowHL = LOW;
int redHL = LOW;
int blueHL = LOW;

void setup() {
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    in = Serial.read();
    Serial.println(in);
    switch(in) {
      case 50: 
        if (greenHL == LOW) {
          greenHL = HIGH;
          digitalWrite(green, greenHL);
        } else {
          greenHL = LOW;
          digitalWrite(green, greenHL);
        }
        break;
      
      case 51:
      if (yellowHL == LOW) {
          yellowHL = HIGH;
          digitalWrite(yellow, yellowHL);
        } else {
          yellowHL = LOW;
          digitalWrite(yellow, yellowHL);
        }
        break;
  
        case 52:
      if (redHL == LOW) {
          redHL = HIGH;
          digitalWrite(red, redHL);
        } else {
          redHL = LOW;
          digitalWrite(red, redHL);
        }
        break;
  
        case 53:
      if (blueHL == LOW) {
          blueHL = HIGH;
          digitalWrite(blue, blueHL);
        } else {
          blueHL = LOW;
          digitalWrite(blue, blueHL);
        }
        break;
    }
  }
}
