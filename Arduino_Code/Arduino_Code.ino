#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

int buttons[4] = {2, 3, 4, 5};  
int leds[4] = {8, 9, 10, 11};   
boolean button[4] = {0, 0, 0, 0};  

#define buzzer 6                    
#define levelsInGame 20

int bt_simonSaid[100]; 
int led_simonSaid[100]; 

boolean lost;
int game_play, level, stage, score, highestScore;

// Difficulty levels
enum Difficulty {NORMAL,HARD};
Difficulty selectedDifficulty;

int normalInitialDelay = 350;  // Initial delay for normal pattern display in milliseconds
int hardInitialDelay = 240;    // Initial delay for hard pattern display in milliseconds
int delayReduction = 5;       // Amount to reduce delay with each level

void setup() {
  Serial.begin(9600);

  // Initialize digital pins as outputs for LEDs in the buttons.
  for (int i = 0; i <= 3; i++) {
    pinMode(buttons[i], INPUT_PULLUP); // Set the button pins as inputs
    pinMode(leds[i], OUTPUT);          // Set the LED pins as outputs
  }

  pinMode(buzzer, OUTPUT);

  lcd.init(); // Initialize the LCD
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   Welcome To   ");
  lcd.setCursor(0, 1);
  lcd.print("> Memory  Game <");
  delay(2000); 
  lcd.clear();

  randomSeed(analogRead(0)); // Make our random numbers more random to enhance the unpredictibility 

  // Initialize scores
  score = 0;
  highestScore = 0;
  // Select difficulty level
  selectDifficulty();
}

void selectDifficulty() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Difficulty:");
  lcd.setCursor(0, 1);
  lcd.print("R:Normal  B:Hard");

  // Wait for button press to select difficulty
  while (true) {
    for (int i = 0; i < 4; i++) {  // Iterate over all 4 buttons
      button[i] = digitalRead(buttons[i]);
      if (button[i] == LOW) {
        if (i == 0) {
          selectedDifficulty = NORMAL;
        } else if (i == 3) {
          selectedDifficulty = HARD;
        } else {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Invalid Button");    //stay invalid untill user presses the correct button 
          delay(1000);
          return;
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selected:");
        lcd.setCursor(0, 1);
        lcd.print((selectedDifficulty == NORMAL) ? "Normal" : "Hard");
        delay(2000);
        return;
      }
    }
  }
}

void loop() {
  // the loop function runs over and over again forever
  switch (stage) {
  //Game Initialization
  case 0:               
    lcd.setCursor(0, 0);
    lcd.print((selectedDifficulty == NORMAL) ? "Press Red Button" : "Press BlueButton");
    lcd.setCursor(0, 1);
    lcd.print(" for Start Game ");
    
    while (true) {
      for (int i = 0; i < 4; i++) {
        button[i] = digitalRead(buttons[i]);
        if (button[i] == LOW) {
          if ((selectedDifficulty == NORMAL && i == 0) || (selectedDifficulty == HARD && i == 3)) {
            level = 1, stage = 1, game_play = 1;
            return;
          } else {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Invalid Button");
            delay(1000);
            break;
          }
        }
      }
    }
  break;

  //Pattern Memorization
  case 1:
    lcd.clear();
    // print level on screen
    lcd.setCursor(4, 0);
    lcd.print("Level: ");
    lcd.print((level / 10) % 10); //format when you want to ensure that they always occupy two positions (01,02,03...)
    lcd.print(level % 10);

    lcd.setCursor(0, 1);
    lcd.print(" -- Memorize -- ");
    delay(1500);
    led_simonSaid[level] = random(8, 12); // populate the array with random 'colours'
    // Inside the loop for pattern display with difficulty curve
    for (int i = 1; i <= level; i++) {
      digitalWrite(led_simonSaid[i], HIGH); // turn on the button light
      playBuzzer(led_simonSaid[i] - 7);
      digitalWrite(led_simonSaid[i], LOW);
      // Use different initial delays for normal and hard difficulty
      int currentInitialDelay = (selectedDifficulty == NORMAL) ? normalInitialDelay : hardInitialDelay;
      delay(currentInitialDelay - (level * delayReduction)); // Adjusted delay for difficulty
    }
    delay(500);
    stage = 2;
  break;

  //Player's Turn
  case 2:
    stage = 3;
    lcd.setCursor(0, 1);
    lcd.print("   -- Play --   ");
  break;

  //Tracking Player's Input
  case 3:
    for (int i = 0; i <= 3; i++) {
      button[i] = digitalRead(buttons[i]);
      if (button[i] == LOW) {
        bt_simonSaid[game_play] = leds[i];
        digitalWrite(leds[i], HIGH);
        playBuzzer(i + 1); // make the sound of the button pressed - right or wrong
        while (button[i] == LOW) {
          button[i] = digitalRead(buttons[i]);
        }
        delay(50);
        digitalWrite(leds[i], LOW);
        game_play++;
        if (game_play - 1 == level) {
          game_play = 1;
          stage = 4;
          break;
        }
      }
    }
    delay(10);
  break;

  //Verification
  case 4:
    lcd.setCursor(0, 1);
    lcd.print("  Verification  ");
    delay(1000);
    for (int i = 1; i <= level; i++) {
      if (led_simonSaid[i] != bt_simonSaid[i]) {
        lost = 1;
        break;
      }
    }
    if (lost == 1)
      stage = 5;
    else {
      stage = 6;
      score++; // Increment the score when the player successfully completes a level
    }
  break;

  //Lost Case , Show the message and display score
  case 5:
    lcd.setCursor(0, 1);
    lcd.print(" !! You Lost !! ");
    tone(buzzer, 350); // Play game over low tone on the buzzer
    for (int i = 0; i <= 3; i++) {
      digitalWrite(leds[i], HIGH);
    }
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("!! GAME  OVER !!");
    noTone(buzzer);
    delay(1000);
    for (int i = 0; i <= 3; i++) {
      digitalWrite(leds[i], LOW);
    }

    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your Score: ");
    lcd.print(score);

    if (score > highestScore) {
      highestScore = score;
      lcd.setCursor(0, 1);
      lcd.print("New High Score!");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("High Score: ");
      lcd.print(highestScore);
    }
    delay(3500); 

    // Reset scores and restart the game
    score = 0;
    level = 1;
    stage = 0;
    lost = 0;
    // Select difficulty for the next game
    selectDifficulty();
  break;

  //Win Case , show you win and then goes to next level (back to case1) untill the game is completed
  case 6:
    lcd.setCursor(0, 1);
    lcd.print(" ** You  Win ** ");
    delay(1000);
    if (level == levelsInGame) {
      lcd.setCursor(0, 0);
      lcd.print("Congratulation");
      lcd.setCursor(0, 1);
      lcd.print(" Level Complete");
      delay(1000);
      lcd.clear();
      level = 1;
    } else {
      if (level < levelsInGame)
        level++;
    }

    stage = 1;
  break;

  default:
  break;
  }
}

void playBuzzer(int x) {
  tone(buzzer, 650 + (x * 100));
  delay(300);
  noTone(buzzer);
}
