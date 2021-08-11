#include "avr/io.h"
#include "avr/delay.h"
/*
  Basic code for using Maxim MAX7219/MAX7221 with Arduino.
  Wire the Arduino and the MAX7219/MAX7221 together as follows:
  | Arduino   | MAX7219/MAX7221 |
  | --------- | --------------- |
  | MOSI (11) | DIN (1)         |
  | SCK (13)  | CLK (13)        |
  | I/O (7)*  | LOAD/CS (12)    |
    * - This should match the LOAD_PIN constant defined below.
  
  For the rest of the wiring follow the wiring diagram found in the datasheet.
  
  Datasheet: http://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
  Author:  Nicholas Dobie <nick@nickdobie.com>
  Date:    30 December 2013
  License: WTFPL (http://www.wtfpl.net/)
 */
#include <SPI.h>

// What pin on the Arduino connects to the LOAD/CS pin on the MAX7219/MAX7221
#define LOAD_PIN 7

/**
 * Transfers data to a MAX7219/MAX7221 register.
 * 
 * @param address The register to load data into
 * @param value   Value to store in the register
 */
//////////////////////////////////////////////// PWM AUDIO ABOVE

void maxTransfer(uint8_t address=0x00, uint8_t value=0x00, uint8_t address2=0x00, uint8_t value2=0x00, uint8_t address3=0x00, uint8_t value3=0x00, uint8_t address4=0x00, uint8_t value4=0x00) {

  // Ensure LOAD/CS is LOW
  digitalWrite(LOAD_PIN, LOW);

  
  SPI.transfer(address4); // Registers
  SPI.transfer(value4);

  SPI.transfer(address3);
  SPI.transfer(value3);

  SPI.transfer(address2);
  SPI.transfer(value2);

  SPI.transfer(address);
  SPI.transfer(value);

  // Tell chip to load in data
  digitalWrite(LOAD_PIN, HIGH);
}

uint8_t x[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // Rows
uint8_t y[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; // Columns

int player=0;
int ammo=0; // Sets bullet positions & spawn 
int kills=0; // Score sent through Serial UART
int Gameover=0; // Gamemode variable

int SpeedZ1 = random(500, 800); // Zombie Waves
int SpeedZ2 = random(500, 800);


void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  // Set load pin to output
  pinMode(LOAD_PIN, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  // Reverse the SPI transfer to send the MSB first  
  SPI.setBitOrder(MSBFIRST);
  
  // Start SPI
  SPI.begin();

  // Spi baremetal transmitting data would get stuck in the while loop !
  // easy to understand your just writing to registers and waiting for status bits 
  // but it just wouldnt work 
 
  maxTransfer(0x0F, 0x03, 0x0F, 0x03, 0x0F, 0x03, 0x0F, 0x00); // Display test register
  delay(1000);
  maxTransfer(0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00);
  
  // Enable mode B
  
  maxTransfer(0x09, 0x00, 0x09, 0x00, 0x09, 0x00, 0x09, 0x00);  // do not decode for digits since we are using a matrix not a 7 segment display
  // Use lowest intensity
  maxTransfer(0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00);
  
  // Scan all digits (all matrix rows)
  maxTransfer(0x0B, 0x07, 0x0B, 0x07, 0x0B, 0x07, 0x0B, 0x07);
    
  // Turn on chip
  maxTransfer(0x0C, 0x01, 0x0C, 0x01, 0x0C, 0x01, 0x0C, 0x01);
 
  for (uint8_t i = 0x00; i < 0x09; ++i){ // Sort of Clears the screen
    
      maxTransfer(i, 0x00, i, 0x00, i, 0x00, i, 0x00); 
    
  }

}

class Zombie {
  public:
    unsigned long previousMillis = 0;  // Zombie function variables
    int I = 0;
    int Z = 7;
    uint8_t first, second, third, fourth;
    uint8_t row;
    
    void run(uint8_t spawn=0x01, int Speed=500, int *matrix=0, uint8_t *row=0x00, uint8_t *column=0x00){ // zombie moving from one end to the other
      
      unsigned long pause = millis();

      if(pause - previousMillis >= Speed){ // >= Zombies movement speed in milliseconds 
          previousMillis = pause;
          --Z;
          if(Z==-1){ // once Z hits the edge of the matrix Hop to the next one 
            maxTransfer(first, 0x00, second, 0x00, third, 0x00, fourth, 0x00); // dont leave LEDs on
            Z = 7; // Zombie starts 0x80 place of next matrix
            ++I; // switch to next matrix
            if(I==4) I=0; 
            
          } 
        } 
      
        if(I==0){
          fourth = spawn; 
          third = 0x00;
          second = 0x00;
          first = 0x00;
        }
        if(I==1){
          fourth = 0x00;
          third = spawn;
          second = 0x00;
          first = 0x00;
        }
        if(I==2){
          fourth = 0x00;
          third = 0x00;
          second = spawn;
          first = 0x00;
        }
        if(I==3){ // Zombie is on player matrix (Collision possibility) 
          fourth = 0x00;
          third = 0x00;
          second = 0x00;
          first = spawn;
          if(first == x[player]){
            if(Z==0x01){
              maxTransfer(0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00); // Shutdown matrix
              Gameover = 1;
            }
            
          }
        }
        
        if((*matrix==I)&&(*row==spawn)&&(*column==y[Z])){
          ++kills;
          tone(8, 500, 10);
          I=0; // Bullet Collision
        }
          
        
        
        maxTransfer(first, y[Z], second, y[Z], third, y[Z], fourth, y[Z]); // LEDs dont stay on when Z decrements, Unlike player.
      /*
       * Bullets and zombies need to be on same matrix, row, and column
       * The only way to detect this seems to be through shared variables 
       * which with classes seems to only be done through parameters
       */
     
    }
  
};

class Bullet { // Zombies are pixels running right to left so bullets are just the opposite 
  
  public:

    unsigned long previousMillis = 0;  
    int I = 0;
    int Z = 0;
    uint8_t first, second, third, fourth;
    
    void pistol(uint8_t spawn=0x01, bool *hit=false, int *matrix=0, uint8_t *row=0x00, uint8_t *column=0x00, int Speed=80){ 
      
      //*hit = true;
      unsigned long pause = millis();

    
        if(pause - previousMillis >= Speed){ // >= Zombies movement speed in milliseconds 
            previousMillis = pause;
            ++Z;
            if(Z==7){ 
              maxTransfer(first, 0x00, second, 0x00, third, 0x00, fourth, 0x00); 
              Z = 0;
              ++I; 
              if(I==4){
                I = 0;
                *hit = false;
              }
            } 
          } 
    
        if(I==0){ 
          fourth = 0x00;
          third = 0x00;
          second = 0x00;
          first = spawn;
          *matrix = 3; 
        }
        if(I==1){
          fourth = 0x00;
          third = 0x00;
          second = spawn;
          first = 0x00;
          *matrix = 2; 
        }
        if(I==2){
          fourth = 0x00;
          third = spawn;
          second = 0x00;
          first = 0x00;
          *matrix = 1; 
        }
        if(I==3){ 
          fourth = spawn;
          third = 0x00;
          second = 0x00;
          first = 0x00;
          *matrix = 0;            
         // if(first == x[player]){
          //  if(Z==0x01) maxTransfer(0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00); // Shutdown matrix
        //  }
        }

        *row = spawn; // row is the spawn 
        *column = y[Z]; // column is the incrementing variable 
        maxTransfer(first, y[Z], second, y[Z], third, y[Z], fourth, y[Z]); 
        // matrix is the matrix but we can basically just use the variable I for that
       
    }
};

class Nuke {
  public:
    unsigned long previousMillis = 0;  // Zombie function variables
    int I = 0;
    int Z = 7;
    uint8_t first[2], second[2], third[2], fourth[2];
    uint8_t row[2];
    
    void drop(bool *blownup=false, uint8_t spawn=0x02, int Speed=100){ 
    // 0x08 max and 0x02 min for spawn since there are 2 pixels being displayed, cant have one get cutoff on the matrix
      
      unsigned long pause = millis();
      
    if(!*blownup){
      if(pause - previousMillis >= Speed){ 
          previousMillis = pause;
          --Z;
          if(Z==-1){ 
            for(int i = 0; i<2;i++) maxTransfer(first[i], 0x00, second[i], 0x00, third[i], 0x00, fourth[i], 0x00); // dont leave LEDs on
            Z = 7; 
            ++I; 
            if(I==4){
              I = 0;
              *blownup=true;
            }
            
          } 
         } 
         
        if(I==0){
          fourth[0] = spawn; 
          fourth[1] = spawn-1;
          third[0] = 0x00;
          third[1] = 0x00;
          second[0] = 0x00;
          second[1] = 0x00;
          first[0] = 0x00;
          first[1] = 0x00;
        }
        if(I==1){
          fourth[0] = 0x00; 
          fourth[1] = 0x00;
          third[0] = spawn;
          third[1] = spawn-1;
          second[0] = 0x00;
          second[1] = 0x00;
          first[0] = 0x00;
          first[1] = 0x00;
        }
        if(I==2){
          fourth[0] = 0x00; 
          fourth[1] = 0x00;
          third[0] = 0x00;
          third[1] = 0x00;
          second[0] = spawn;
          second[1] = spawn-1;
          first[0] = 0x00;
          first[1] = 0x00;
        }
        if(I==3){ // Zombie is on player matrix (Collision possibility) 
          fourth[0] = 0x00; 
          fourth[1] = 0x00;
          third[0] = 0x00;
          third[1] = 0x00;
          second[0] = 0x00;
          second[1] = 0x00;
          first[0] = spawn;
          first[1] = spawn-1;
          for(int i = 0; i<2;i++){
            if(first[i] == x[player]){
                if(Z==0x01){
                  SpeedZ1 += 40; 
                  SpeedZ2 += 40;
                  bomb();
                }
              }
          }
        }
        
        for(int i = 0; i<2;i++) maxTransfer(first[i], y[Z], second[i], y[Z], third[i], y[Z], fourth[i], y[Z]); 
      }
    }
  
};

void DisplayScore(){
  Serial.print("YOU GOT BIT");
  Serial.println("SCORE: ");
  Serial.print(kills, DEC);
  Gameover = 2;
}

bool debounceL = true;
bool debounceR = true;
bool debounceB = true;

Zombie myZ1;
Zombie myZ2;
Zombie myZ3;
Zombie myZ4;
Zombie myZ5;
Zombie myZ6;
Zombie myZ7;
Zombie myZ8;

Bullet chamber1;
Bullet chamber2;
Bullet chamber3;
Bullet chamber4;
Bullet chamber5;
Bullet chamber6;
bool hit1 = false;
bool hit2 = false;
bool hit3 = false;
bool hit4 = false;
bool hit5 = false;
bool hit6 = false;

uint8_t Bpos[6]={0x01,0x01,0x01,0x01,0x01,0x01};

int matrix[6]={0,0,0,0,0,0}; 
uint8_t row[6]={0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t column[6]={0x00,0x00,0x00,0x00,0x00,0x00};

unsigned long previousMillis = 0;  

Nuke planB;
bool blownup=true;
int bombdrop=0;
/////////////////////////////////////////////////////////////////////
void bomb(){
  Gameover = -1; // its gonna have to pause the whole game
  maxTransfer(0x0F, 0x03, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00); // Display test register
  tone(6, 400, 100);
  delay(400);
  maxTransfer(0x0F, 0x00, 0x0F, 0x03, 0x0F, 0x00, 0x0F, 0x00);
  tone(6, 300, 100);
  delay(400);
  maxTransfer(0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x03, 0x0F, 0x00);
  tone(6, 200, 100);
  delay(400);
  maxTransfer(0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x03);
  tone(6, 100, 100);
  delay(400);
  maxTransfer(0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00);
  myZ1.I=0; // Set all the zombies back to the fourth matrix cause they got blown up
  myZ2.I=0;
  myZ3.I=0;
  myZ4.I=0;
  myZ5.I=0;
  myZ6.I=0;
  myZ7.I=0;
  myZ8.I=0;
  Gameover = 0;
}
//////////////////////////////////////////////////////////////////////
void loop() { 

  unsigned long pause = millis();

  if(pause - previousMillis >= 15000){ 
      previousMillis = pause; // These Zspeeds are also used as timers 
      SpeedZ1 -= 50;// So every 15 seconds increase the zombie numbers
      SpeedZ2 -= 50;
      blownup=false; // drop a nuke powerup to make things easier !
      bombdrop = random(1,7);
      
  }
    
  if(Gameover==0){ // So after screen shuts off zombies stop showing up
    
    if(!blownup) planB.drop(&blownup, x[bombdrop], 30); // nuke drop cancels itself out after hitting first matrix
    
    for(int i = 0; i<8;i++){
        myZ1.run(0x01, SpeedZ1, &matrix[i], &row[i], &column[i]); 
        myZ2.run(0x02, SpeedZ2, &matrix[i], &row[i], &column[i]);
        if(SpeedZ1 < 600) myZ3.run(0x03, SpeedZ1, &matrix[i], &row[i], &column[i]); // timing for when other zombies join
        if(SpeedZ1 < 600) myZ4.run(0x04, SpeedZ2, &matrix[i], &row[i], &column[i]); 
        if(SpeedZ1 < 500) myZ5.run(0x05, SpeedZ1, &matrix[i], &row[i], &column[i]); 
        if(SpeedZ1 < 500) myZ6.run(0x06, SpeedZ2, &matrix[i], &row[i], &column[i]);
        if(SpeedZ1 < 300) myZ7.run(0x07, SpeedZ1, &matrix[i], &row[i], &column[i]);
        if(SpeedZ1 < 300) myZ8.run(0x08, SpeedZ2, &matrix[i], &row[i], &column[i]); 
    }
    
  
  }
  if(Gameover==1) DisplayScore(); // Gameover being a boolean did not work with displaying score, needed more possible values to work with instead of just 2
  
  
  maxTransfer(x[player], 0x01); // player ( Not interfering with Zombie movement because the other bytes are in NO OP opcode by default )
  if(x[player]<0x08) maxTransfer(x[player+1], 0x00); // Dont leave LEDs around player on so you always know which LED is the player
  if(x[player]>0x01) maxTransfer(x[player-1], 0x00); 

  if(ammo==1){ // Quick fix to not have bullets ghost, just increase ammo again so the booleans arent stuck being true and the functions can cancel themselves out
    hit1 = true;
    ++ammo;
  }
  if(ammo==3){
    hit2 = true; 
    ++ammo;
  }
  if(ammo==5){
    hit3 = true;
    ++ammo; 
  }
  if(ammo==7){
    hit4 = true; 
    ++ammo;
  }
  if(ammo==9){
    hit5 = true;
    ++ammo;
  }
  if(ammo==11){
    hit6 = true;
    ++ammo;
  }
  
  if(hit1) chamber1.pistol(Bpos[0], &hit1, &matrix[0], &row[0], &column[0]);
  if(hit2) chamber2.pistol(Bpos[1], &hit2, &matrix[1], &row[1], &column[1]);
  if(hit3) chamber3.pistol(Bpos[2], &hit3, &matrix[2], &row[2], &column[2]);
  if(hit4) chamber4.pistol(Bpos[3], &hit4, &matrix[3], &row[3], &column[3]);
  if(hit5) chamber5.pistol(Bpos[4], &hit5, &matrix[4], &row[4], &column[4]);
  if(hit6) chamber6.pistol(Bpos[5], &hit6, &matrix[5], &row[5], &column[5]);
  
  
  if(!(digitalRead(5))){ // shoot button
        if(debounceB){ // Bpos[ammo/2] because of ammo being increased again in the above conditionals
          Bpos[ammo/2] = x[player]; // Bullets spawn on the player as they should
          ++ammo; 
          if(ammo>13) ammo=0;
          debounceB = false;
        }
        tone(6, 400, 10);
  }
    
  if(!(digitalRead(3))){ // move right
      if(debounceL){
        if(player<0) player = 0; // player boundaries
        if(player>0) player-=1;
        debounceL = false;
      }
  }
  if(!(digitalRead(4))){ // move left
      if(debounceR){
          if(player>7) player = 7;
          if(player<7) player+=1;
          debounceR = false;
        }
  }
  
  if(digitalRead(3)) debounceL = true;
  if(digitalRead(4)) debounceR = true; 
  if(digitalRead(5)){
    debounceB = true;
    noTone(6);
  }
  
  // Loop through each code
  /*for (uint8_t i = 0; i < 0x80; ++i) // dont know how much it counts in hex
  { // but 0x80 is the hex to turn the MSB LED on so it goes through all the values up to it
    maxTransfer(0x01, 0x00); // Every digit Reg
    maxTransfer(0x02, 0x00);
    maxTransfer(0x03, 0x00);
    maxTransfer(0x04, 0x00);
    maxTransfer(0x05, 0x00);
    maxTransfer(0x06, 0x00);
    maxTransfer(0x07, 0x00);
    maxTransfer(0x08, 0x00);
    //delay(300);*/
 // }
  
}
