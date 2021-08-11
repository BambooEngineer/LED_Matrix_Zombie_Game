# LED_Matrix_Zombie_Game
Arduino Nano SPI with a max7219 LED Matrix driver chip without the use of a matrix library. SPI code to drive one matrix was found on github and how to drive more than one required the max7219 datasheet. 

SPI{

Digital pin 11: MOSI
Digital pin 13: SCK 
Digital pin 7: Chip Select 

}

Tones come out of pins 6 and 8 ( had problems with Tone() using one pin ) 

Digital pin 5: Action Button
Digital pin 4: Move Left
Digital pin 3: Move Right
