#include <LiquidCrystal_I2C.h>
#include <Arduino.h> // always required when using platformio

// % LCD related settings and stuff...------------------------------------------
const uint8_t LCD_ADDRESS = 0x27;
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 4;

#include "Wire.h" // For I2C
// #include "LCD.h" // For LCD
//Set the pins on the I2C chip used for LCD connections
//ADDR,EN,R/W,RS,D4,D5,D6,D7
// LiquidCrystal_I2C V_LCD(0x27,2,1,0,4,5,6,7);

uint8_t lcd_clock[8] = {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};
uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
uint8_t check[8] = {0x0, 0x1 ,0x3, 0x16, 0x1c, 0x8, 0x0};
uint32_t liebe = 0;


LiquidCrystal_I2C V_LCD(LCD_ADDRESS,LCD_COLS,LCD_ROWS);

// setup serial
void setup_serial(){
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------------------------------");
  Serial.println("-----------------------   Vital Clues   ---------------------");
  Serial.println("--------------------   Ver.01 | JR | 2019  ------------------");
  Serial.println("-------------------------------------------------------------");
  delay(1000);
}

// setup small LCD
void setup_lcd(){
  V_LCD.init();                      // initialize the lcd
  V_LCD.backlight();
  V_LCD.setBacklight(HIGH);
  V_LCD.createChar(0, lcd_clock);
  V_LCD.createChar(1, heart);
  V_LCD.createChar(2, check);
  V_LCD.clear();
  V_LCD.home();
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void setup(void) {
  setup_serial();
  setup_lcd();
  V_LCD.print("Hello World");
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(void) {


  V_LCD.home();
  // V_LCD.clear();
  V_LCD.print("Ich liebe dich enton +");
  V_LCD.print(liebe++);
  // delay(10);

}
