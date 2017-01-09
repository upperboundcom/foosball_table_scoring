
/*
 * foosbal_table_scoring
 * 
 * A scoring system for a foosball table or any other similar two-player tabletop game.
 * 
 * (C) 2017 Upperbound.com. All rights reserved.
 */
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "Player.h"
#include "Goal.h"

// Software version
#define VERSION "0.1"
// Possible system states
#define STATE_STARTUP 0
#define STATE_CANCEL 1
#define STATE_ROUNDOVER 2
#define STATE_GAMEOVER 3
#define STATE_INGAME 4
#define STATE_CONFIRM_CANCEL_POINT 8
#define STATE_GOAL_SCORED 9

// Some special LCD characters
#define RIGHT_ARROW 0x7E
#define LEFT_ARROW 0x7F

LiquidCrystal lcd(26, 28, 5, 4, 3, 38);

int backLight = 30;    // LCD (pin A) backlight. 
int maxPoints = 5; // Maximum # of points needed to win a round
int maxRounds =0 ; // Maximum # of rounds in a game (will be 3 or 5 depending on user selection)

// UI Pushbutton arduino pins for the control panel
int leftButtonPin = 32;
int rightButtonPin =33;
int currentState = STATE_STARTUP;
int previousState = -999;
int yesCancelState, noCancelState, okState; // State assignment variables for possible button presses

int maxPlayerNameLen = 20; // Max # of characters for a player name
int leftButtonState, rightButtonState; // Holds the states of the pushbuttons

int buttonDelay = 150; //Default button press delay in ms

int currentGameNum,  roundsToWin;
int currentRoundNum=1;
int gameMode;

int lightLevel; // For the photoresistors
int highLightLevel = 0;
int lowLightLevel = 1023;


Player player1;
Player player2;

Goal goal1(46,A9,370);
Goal goal2(47,A8,270);

// Goal light threshold (photo sensitivity adjustment using a trimpot) pin. We can have one of these for each goal or just one for both.

int goalLightThresholdPin = A7;

int goalLightThreshold;

// A pointer to whoever last scored. 
// Using a pointer here to avoid copying Player objects around unnecessarily and taking up valuable SRAM
Player* playerLastScored; 

// Pointers to last round winner and game winner, respectively
Player* roundWinnerPlayer;
Player* gameWinnerPlayer;

// For the buzzer

int buzzerPin = 45;

char *gameName="Foosball Table";

void setup()
{
  Serial.begin(9600);

  // Control panel pin modes
  pinMode(backLight, OUTPUT);
  pinMode(leftButtonPin, INPUT);
  pinMode(rightButtonPin, INPUT);

  digitalWrite(backLight, HIGH); // turn LCD backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(20,4);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  showAnimatedStartup();
  
  pinMode(buzzerPin, OUTPUT);
  Serial.print(gameName);
  Serial.print(" v");
  Serial.println(VERSION);
  goal1.label = "Goal1";
  goal2.label= "Goal2";
}

void loop()
{
  
    // Handle the different system states when it changes
    
    if (currentState != previousState) {

      Serial.print("Previous state was ");
      Serial.println(previousState);
      Serial.print("Game state changed to ");
      Serial.println(currentState);
      
      Serial.println("player 1:");
      player1.dumpValues();
      Serial.println("player 2:");
      player2.dumpValues();
      
      switch (currentState) {
        case STATE_STARTUP:
          handleStateStartup();
          break;  
        case STATE_INGAME:
          handleStateInGame();
          break;
        case STATE_ROUNDOVER:
          handleStateRoundOver();
          break;
        case STATE_GAMEOVER:
          handleStateGameOver();
          break;
        case STATE_CONFIRM_CANCEL_POINT:
          handleStateCancelPointConfirmation();
          break;
        case STATE_GOAL_SCORED:
          handleStateGoalScored();
          break;
        case STATE_CANCEL:
          handleStateCancel();
          break;
        default:
          // todo
          break;
      }
    
    }
    Serial.println("State is the same!");
    
}
// Shows start screen
void showStartScreen() {

  Serial.println("Showing start screen");
  lcd.clear();                 
  lcd.setCursor(0,0);          
  lcd.print(gameName);
  lcd.print(" v");
  lcd.print(VERSION); 
  lcd.setCursor(2,1);          
  lcd.print("Select Game Type");
  lcd.setCursor(0,3);         
  lcd.write(LEFT_ARROW);
  lcd.print(" 2/3   Best   3/5 "); 
  lcd.write(RIGHT_ARROW);
}

// Shows cancellation confirmation screen
void showCancelConfirmationScreen() {
  Serial.println("Showing cancel screen");
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Are you sure you");
  lcd.setCursor(3,1);
  lcd.print("want to cancel?");  

  lcd.setCursor(0,3);
  lcd.write(LEFT_ARROW);
  lcd.print(" Yes           No ");
  lcd.write(RIGHT_ARROW);
}

// Shows player welcome screen after badge-in
// Truncate playerName to 12 chars.
void showPlayerWelcomeScreen(String playerName, int nextPlayerNum) {

  Serial.println("Showing player welcome screen");
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Welcome ");
  lcd.print(playerName.substring(0,12));

  lcd.setCursor(0,1);
  lcd.print("Player ");
  lcd.print(nextPlayerNum);
  lcd.print(" badge in.");

  lcd.setCursor(0,3);
  lcd.write(LEFT_ARROW);
  lcd.print (" Back      Cancel "); 
  lcd.write(RIGHT_ARROW);
}

// Animated startup sequence

void showAnimatedStartup() {
  int delayMS=20;

  lcd.clear();
  for (int i=0; i<20; i++) {
    lcd.setCursor(i,0);
    lcd.write(0x05);
    
    delay(delayMS);
    lcd.setCursor(i,3);
    lcd.write(0x05);
    delay(delayMS);
    lcd.setCursor(i,3);


  }

}

// Show game-in progress screen
// Player names have to be truncated to 7 charactesr due to the size limitations of the LCD screen

void showGameInProgressScreen(int player1Score, int player2Score, int currentRound) {
  Serial.println("Showing game in progress screen");
  int maxNameSize = 7;
  lcd.clear();
  // Player name and current round row
  lcd.setCursor(0,0);

  lcd.print(player1.name.substring(0,maxNameSize));


  // round information
  lcd.setCursor(8,0);
  lcd.print(currentRound);
  lcd.print("/");
  lcd.print(maxRounds);

  
  // Now need to right-justify the truncated player 2 name

  String truncated = player2.name.substring(0,maxNameSize);
  lcd.setCursor(20 - truncated.length(),0);
  lcd.print(truncated);
  // Scoring row
  lcd.setCursor(0,1);
  lcd.print(player1Score);
  lcd.print("       Score      ");
  lcd.print(player2Score);

  // Rounds won row
  lcd.setCursor(0,2);
  lcd.print(player1.roundsWon);
  lcd.print("    Rounds Won");
  lcd.setCursor(19,2);
  lcd.print(player2.roundsWon);
  
  
  // Button row
  lcd.setCursor(0,3);
  lcd.write(LEFT_ARROW);
  lcd.print(" Cancel Game");
}

// Show Round over screen

void showRoundOverScreen(String playerName) {
  Serial.println("Showing round over screen");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(playerName);
  lcd.setCursor(0,1);
  lcd.print("wins this round!");
  lcd.setCursor(0,3);
  lcd.write(LEFT_ARROW);
  lcd.print(" Cancel      Next ");
  lcd.write(RIGHT_ARROW);
}

// Show game over screen. Game over man! Game over!
void showGameOverScreen(String playerName) {
  Serial.println("Showing game over screen");
  lcd.clear();
  flashBacklight();
  lcd.setCursor(0,0);
  lcd.print(playerName);
  lcd.print(" wins!");

  lcd.setCursor(0,3);

  lcd.print("              Done ");
  lcd.write(RIGHT_ARROW);
}

// flashes LCD backlight
void flashBacklight() {

  int delayMS=300; 

  digitalWrite(backLight, LOW);
  delay(delayMS);
  digitalWrite(backLight, HIGH);
  delay(delayMS);
  digitalWrite(backLight, LOW);
  delay(delayMS);
  digitalWrite(backLight, HIGH);  
}

// Startup state handler
void handleStateStartup() {
  
  player1.clear();
  player2.clear();  
  showStartScreen();
  previousState = currentState;   
    // Wait for input
    while (1) {
      leftButtonState = digitalRead(leftButtonPin);
      rightButtonState = digitalRead(rightButtonPin);
            
      delay(buttonDelay);
      if (leftButtonState == LOW) {
        // Best 2/3 selected
        maxRounds =3;
        roundsToWin = 2;
        break;
      }
      
      if (rightButtonState == LOW) {
        // Best 3/5 selected
        maxRounds = 5;
        roundsToWin=3;
        break;
      }      

    }

    player1.name="Player1";
    player2.name="Player2";
    Serial.print("Selected best ");
    Serial.print(roundsToWin);
    Serial.print("/");
    Serial.println(maxRounds);
    currentState = STATE_INGAME;
    return;
    
}


// Cancel screen handler
void handleStateCancel() {
  showCancelConfirmationScreen();
  
  // wait for input
  while(1) {
    leftButtonState = digitalRead(leftButtonPin);
    rightButtonState = digitalRead(rightButtonPin);
  
    delay(buttonDelay);
    if (leftButtonState == LOW) {
      // Yes to cancel
      previousState = currentState;
      currentState = yesCancelState;
      return;
    }
    if (rightButtonState == LOW) {
      // No to cancel
      previousState = currentState;
      currentState = noCancelState;
      return;
    }
  }
  
}



// Handler for the current game

void handleStateInGame() {
    
    Serial.print("Player 2 is ");
    Serial.println(player2.name);
    
    activateGoals();
    
    previousState = currentState;
    showGameInProgressScreen(player1.score, player2.score,currentRoundNum);
    // Wait for inputs. Either button presses or a player scores
    while(1) {
      leftButtonState = digitalRead(leftButtonPin);

      delay(buttonDelay);
      goalLightThreshold = analogRead(goalLightThresholdPin);
      
      goal1.setLightThreshold(goalLightThreshold);
      goal2.setLightThreshold(goalLightThreshold);

      Serial.print("Goal light threshold: ");
      Serial.println (goalLightThreshold);
      
      bool player1Scored = goal1.read();
      bool player2Scored = goal2.read();

      // If either light level is low enough, someone scored, so switch states      
      
       if (player1Scored || player2Scored) {
          // See who scored
        

        if (player1Scored) {
           //Serial.println("Player 1 GOAL!");
           playerLastScored = &player1;
        }
          
        if (player2Scored) {
           //Serial.println("Player 2 GOAL!");

           playerLastScored = &player2;

         }
         Serial.print(playerLastScored->name);
         Serial.println(" scored!");
         
         currentState = STATE_GOAL_SCORED;
         deactivateGoals();
         
         return;         
      }
      
       if (leftButtonState == LOW) {
        // Cancel game was pressed
        currentState = STATE_CANCEL;
        
        yesCancelState = STATE_STARTUP;
        noCancelState = STATE_INGAME;
        deactivateGoals();
        return;
      }
      
    }
    
}

// Handler for the round over state
void handleStateRoundOver() {

  showRoundOverScreen(roundWinnerPlayer->name);
  
  //Wait for input
  
  while(1) {
      
      leftButtonState = digitalRead(leftButtonPin);
      rightButtonState = digitalRead(rightButtonPin);
      delay(buttonDelay);
      
     if (leftButtonState == LOW) {
       previousState = currentState;
       // Cancel game was pressed
        currentState = STATE_CANCEL;
        
        yesCancelState = STATE_STARTUP;
        noCancelState = STATE_ROUNDOVER;
        return;
     }
     if (rightButtonState == LOW) {
       // Moving on to the next round
       previousState = currentState;
       resetScores();
       currentRoundNum++;
       currentState = STATE_INGAME;
       return;       
     }
     
  }
  
}

// Display who scored
void showGoalScreen() {
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print(playerLastScored->name);
 lcd.print(" scores!");

 lcd.setCursor(0,3);
 lcd.write(LEFT_ARROW);
 lcd.print(" Cancel Point  OK ");
 lcd.write(RIGHT_ARROW);
 return;
  
}

// Sets player scores to 0
void resetScores() {
  player1.score=0;
  player2.score=0;
}


// handler for the game over state. 
// Game over man, game over!
void handleStateGameOver() {

  showGameOverScreen(gameWinnerPlayer->name);
  // Wait for input
  
  while(1) {
    rightButtonState = digitalRead(rightButtonPin);
    delay(buttonDelay);
    
    if (rightButtonState == LOW) {
      // Done was pressed
      previousState  = currentState;
      currentState = STATE_STARTUP;
      resetGame();
      return;
    }
    
  }
  
}

// Error state, for when something goes wrong

void handleStateError(String message) {
 
  Serial.print("Error: ");
  Serial.println(message);
  
  lcd.clear();
  
  lcd.setCursor(0,0);
  
  lcd.print("Error: ");
  lcd.print(message);
  
  lcd.setCursor(0,2);
  lcd.print("              OK ");
  lcd.write(RIGHT_ARROW);
  
  // wait for input
  while(1) {
    rightButtonState = digitalRead(rightButtonPin);
    delay(buttonDelay);
  } 
}

// Shows the confirm concel point screen
void showConfirmCancelPointScreen() {

  Serial.println("Showing confirm cancel point screen");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You sure you want to");
  lcd.setCursor(0,1);
  lcd.print("cancel the point?");

  lcd.setCursor(0,3);
  lcd.write(LEFT_ARROW);
  lcd.print(" Yes           No ");
  lcd.write(RIGHT_ARROW);
  
}

// Resets scoring and game variables
void resetGame() {
  //resetScores();
  maxRounds=0;
  roundsToWin=0;
  currentRoundNum=1;
  player1.clear();
  player2.clear();
}

//handler for the cancel point confirmation screen
void handleStateCancelPointConfirmation() {
  showConfirmCancelPointScreen();
  previousState = currentState;
  
  // wait for input
  while(1) {
      
    leftButtonState = digitalRead(leftButtonPin);
    rightButtonState = digitalRead(rightButtonPin);
    delay(buttonDelay);
    
    if (leftButtonState == LOW) {
      // Yes was pressed  
   
      currentState = yesCancelState;
      return;
    }
    
    if (rightButtonState == LOW) {
      // No was pressed. Just return to the in-game state
      currentState = noCancelState;
      return; 
    }
  }
}

// handler for the goal scored state
void handleStateGoalScored() {
  
  tone(buzzerPin, 300, 100);
  delay(200);
  tone(buzzerPin, 300, 100);
  delay(100);
  tone(buzzerPin, 500, 250);  
  
  showGoalScreen();
  
  int msInThisState = 0; // ms spent in this state
  int maxTimeLimitMS = 6000; // Time limit in ms to wait until state automatically moves one specified by a right button press. 
  
  previousState = currentState;

   
  // wait for input
  while (1) {

    lcd.setCursor(18,2); // Put timer number above "OK" text
    lcd.print((maxTimeLimitMS - msInThisState)/1000);
    
    leftButtonState = digitalRead(leftButtonPin);
    rightButtonState = digitalRead(rightButtonPin);

    delay(buttonDelay);
    
    
    if (rightButtonState == LOW || msInThisState >= maxTimeLimitMS) {

      // OK was pressed so we're accepting the point
      playerLastScored->score++;

      // Now eed to check whether the round is over, the game is over, or if we're still in the same round          
      if (playerLastScored->score == maxPoints) {
        // The player won the round   
        playerLastScored->roundsWon++;
        roundWinnerPlayer = playerLastScored;
           
        // Determine if the whole game is over and set state accordingly
        if (playerLastScored->roundsWon == roundsToWin) {
             currentState = STATE_GAMEOVER;
       
             gameWinnerPlayer = playerLastScored;
             return;
           }
           else {
             currentState=STATE_ROUNDOVER;
             return;
           }
           return;
         }

      else {
          // We're still in-game so go back to the in-game state
          currentState = STATE_INGAME;
          return;
      }      
    
    }
     
    if (leftButtonState == LOW) {
      // cancel point was pressed, so go to the confirm screen.  
      currentState = STATE_CONFIRM_CANCEL_POINT;
      yesCancelState = STATE_INGAME;
      noCancelState = STATE_GOAL_SCORED;
      return;
    }
    
    msInThisState += buttonDelay;

  }  
}


void showErrorScreen(String msg) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Err: ");
  lcd.print(msg);
  lcd.setCursor(0,3);
  lcd.print("                OK ");
  lcd.write(RIGHT_ARROW);
  
}


void activateGoals() {
  
  Serial.println("Activating goals");  
  goal1.activate();
  goal2.activate();
}

void deactivateGoals() {

  Serial.println("Deactivating goals");
  goal1.deactivate();
  goal2.deactivate();  
}

