// AUTHORS AND CREDIT:
// Hans-Christian Duedahl, Christian Norrig & Rasmus Wendelbo. 
// ITEK EAAA - 3rd sem - 2020 
// Last and final iteration - 29/05/2020 

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#endif

#define RST_PIN         5         // Reset pin for RFID RC522
#define SS_PIN         53         // SS pin for RFID RC522

#define PIN             6         // Configurable, pin for NeoPixel
#define NUMPIXELS      12         // Amount of pixel on the NeoPixel
#define timedelay      65         // Time (in milliseconds) to pause between pixels - kept low

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);     // Creates an instance of the Neopixel

String servo;         // Used to define create an instance of a servo, depending on which itemID was entered

int itemArray[] = {0,0,0,0,0,0,0,0,0,0,10,15,10,20}; // Price checker

const byte ROWS = 4;             // Four rows on keypad
const byte COLS = 4;             // Four columns on keypad
char keys[ROWS][COLS] = {        // Keymap that defines the key pressed according to the row and columns
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28};   // Connects to the row pinouts of the 4x4 keypad
byte colPins[COLS] = {30, 32, 34, 36};   // Connects to the column pinouts of the 4x4 keypad
String itemID;                           // String to stored the location of the itemID

const int enterButton = 11;      // Pin for enterbutton
const int exitButton = 9;        // Pin for exitbutton
int enterButtonState = 0;        // Variable for reading the enterbutton status
int exitButtonState = 0;         // Variable for reading the exitbutton status

int r = 1;                       // While loop variable changes between 1 and 2
int t;                           // Variable to set servo pins. Available pins: 31, 33, 35 & 37

int icounteren = 0;             // Variable for counting in Neopixel led_wait function
int d = 0;                      // Variable to count NeoPixel LED nubmer in led_wait, can be 0-11

//int e = 0;                      // Variable to count NeoPixel LED number, can be 0-11
//int o = 0;                      // Variable to change state of NeoPixels direction, can be 0 or 1

int w;                          // Variable for setting the angle on the servos
int n;                          // Variable for check the price on the array
int g;                          // Variable for setting the delay on the servos

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );   // Create instance of keypad with the correct rows, columns and pins.

String getID, readCard, displayName;          // getID is used to save read infomration in EEPROM, readCard to store card id and displayName to check the owner of the card from EEPROM
char getName;                                 // Reads the name from EEPROM and adding them together

MFRC522 mfrc522(SS_PIN, RST_PIN);             // Create MFRC522 instance with the assigned pins

LiquidCrystal_I2C lcd(0x27, 16, 2);           //Set the LCD address to 0x27 for a 16 chance and 2 line display

int c = 0;                                    // Inactive timer variable 1 
int b = 0;                                    // Inactive timer variable 2
int v = 0;                                    // Variable for runnin a function once in led_spin2
int j = -1;                                   // Variable for lighting one LED on the NeoPixel - One behind of h
int h = 0;                                    // Variable for lighting one LED on the Neopixel - One infront of j

void setup() {
  lcd.init();                       // Initialize LCD screen
  lcd.backlight();                  // Turns backlight on LCD to better see
  Serial.begin(9600);		            // Initialize serial communications with the PC  
  while (!Serial);		              // Do nothing if no serial port is opened
  SPI.begin();			                // Init SPI bus
  mfrc522.PCD_Init();		            // Init MFRC522
  delay(4);				                  // Optional delay
   
  pinMode(enterButton, INPUT_PULLUP);     // Initializes enterButton as an input with the internal pull-up resistor enabled:
  pinMode(exitButton, INPUT_PULLUP);      // Initializes exitButton as an input with the internal pull-up resistor enabled:

  pixels.begin();                     // initializes NeoPixel
  lcd_start_message();                // Print a message on the LCD

  // These next commands is not part of the code, but a manual way of inserting someones card id, name and currency.
  // 60-75 reserved to Rasmus Wendelbo and his currency, 220-236 to Christian Norrig and his currency & 380-402 to Hans-Christian Duedahl and his currency
  //EEPROM.write(8, 0x42);
  //EEPROM.write(9, 0x3C);
  //EEPROM.write(10, 0xFE);
  //EEPROM.write(11, 0x0A);
  //EEPROM.write(60, 0xC8);                       // Currency
  //EEPROM.write(220, 0xC8);                      // Currency
  //EEPROM.write(380, 0xC8);                      // Currency
  //EEPROM.put(221, "Christian Norrig");         // Name
}

void resets_variables() {         // Function to resest variables, since a lot is used throughout the code
  //e = 0;
  d = 0;
  r = 1;
  c = 0;
  b = 0;
  h = 0;
  v = 0;
  j = -1;
}

void inactivity_1() {       // Function to keep track of the user. If c == 300, that means that ~20 seconds have passed and the user will be kicked out of the system
  c++;
  if (c == 300) {
    lcd.clear();
    resets_variables();
    itemID = "";            // Reset itemID string
    led_error();
    lcd_start_message();
    }
}

void inactivity_2() {       // Function to keep track of the user. If c == 30000, that means that ~20 seconds have passed and the user will be kicked out of the system
  b++;
  if (b == 30000) {
    lcd.clear();
    resets_variables();
    itemID = "";          // Resets itemID string
    led_error();
    lcd_start_message();
    }
}

void lcd_start_message() {        // Function to greet the user with a message on the screen
  lcd.print("Scan Studentcard");
  lcd.setCursor(0,1);
  lcd.print("to begin process");
}

void exitprogram() {                            // Function to quit the program. If the user hits the red exitButton they will be kicked out of the system
   exitButtonState = digitalRead(exitButton);
   if (exitButtonState == LOW) {
     lcd.clear();
     resets_variables();
     itemID = "";         // Resets itemID string
     led_error();
     lcd_start_message();
     }
}

void readID () {                        // Function to save information regarding the user's card that have been placed near the vicinity of the RC522 module
  readCard = "";
  readCard += mfrc522.uid.uidByte[0];
  readCard += mfrc522.uid.uidByte[1];
  readCard += mfrc522.uid.uidByte[2];
  readCard += mfrc522.uid.uidByte[3];
}

void lcdclearline(int xCoord, int yCoord){      // Function to clear a specific line on the LCD
  lcd.setCursor(xCoord, yCoord);
  lcd.print("                        ");
}

void lcdscroll(int xCoord, int yCoord, String displaytext, String message) {        // Function that displays the name of the cardholder by going through it, one character at a time
   lcd.clear();
   lcd.setCursor(xCoord, yCoord);
   lcd.print(message+displaytext);
   for (int positionCounter = 9; positionCounter < displaytext.length(); positionCounter++) {
      lcd.scrollDisplayLeft();          // Scroll one position left:
      delay(200);
      }
    delay(1000);
    lcd.clear();
    lcd.print("Hello, "+displayName);
    lcd.setCursor(0,1);
    lcd.print("What will it be?");
}

void led_succes() {       // Function to let the user know that the puchase was a succes by blinking Green.
    pixels.setPixelColor(0, pixels.Color(0, 25, 0));
    pixels.setPixelColor(1, pixels.Color(0, 25, 0));
    pixels.setPixelColor(2, pixels.Color(0, 25, 0));
    pixels.setPixelColor(3, pixels.Color(0, 25, 0));
    pixels.setPixelColor(4, pixels.Color(0, 25, 0));
    pixels.setPixelColor(5, pixels.Color(0, 25, 0));
    pixels.setPixelColor(6, pixels.Color(0, 25, 0));
    pixels.setPixelColor(7, pixels.Color(0, 25, 0));
    pixels.setPixelColor(8, pixels.Color(0, 25, 0));
    pixels.setPixelColor(9, pixels.Color(0, 25, 0));
    pixels.setPixelColor(10, pixels.Color(0, 25, 0));
    pixels.setPixelColor(11, pixels.Color(0, 25, 0));
    pixels.show();    // Send the updated pixel colors to the hardware.
}

void led_error() {      // Function to let the user know that the purchase has failed in some way by blinking Red.
    pixels.setPixelColor(0, pixels.Color(25, 0, 0));
    pixels.setPixelColor(1, pixels.Color(25, 0, 0));
    pixels.setPixelColor(2, pixels.Color(25, 0, 0));
    pixels.setPixelColor(3, pixels.Color(25, 0, 0));
    pixels.setPixelColor(4, pixels.Color(25, 0, 0));
    pixels.setPixelColor(5, pixels.Color(25, 0, 0));
    pixels.setPixelColor(6, pixels.Color(25, 0, 0));
    pixels.setPixelColor(7, pixels.Color(25, 0, 0));
    pixels.setPixelColor(8, pixels.Color(25, 0, 0));
    pixels.setPixelColor(9, pixels.Color(25, 0, 0));
    pixels.setPixelColor(10, pixels.Color(25, 0, 0));
    pixels.setPixelColor(11, pixels.Color(25, 0, 0));
    pixels.show();    // Send the updated pixel colors to the hardware.
    delay(100);
}

void led_button() {     // Function to let the user know that the purchase is waiting for confirmation, by blinking Red and Green
    pixels.setPixelColor(0, pixels.Color(25, 0, 0));
    pixels.setPixelColor(1, pixels.Color(25, 0, 0));
    pixels.setPixelColor(2, pixels.Color(25, 0, 0));
    pixels.setPixelColor(3, pixels.Color(25, 0, 0));
    pixels.setPixelColor(4, pixels.Color(25, 0, 0));
    pixels.setPixelColor(5, pixels.Color(25, 0, 0));
    pixels.setPixelColor(6, pixels.Color(0, 25, 0));
    pixels.setPixelColor(7, pixels.Color(0, 25, 0));
    pixels.setPixelColor(8, pixels.Color(0, 25, 0));
    pixels.setPixelColor(9, pixels.Color(0, 25, 0));
    pixels.setPixelColor(10, pixels.Color(0, 25, 0));
    pixels.setPixelColor(11, pixels.Color(0, 25, 0));
    pixels.show();      // Send the updated pixel colors to the hardware.
}

void led_spin2() {      // Function to the let the user know that the system is waiting to get some keypad inputs, by spinning Blue
     if (v == 0) {
      pixels.setPixelColor(0, pixels.Color(0, 0, 25));
      pixels.setPixelColor(1, pixels.Color(0, 0, 25));
      pixels.setPixelColor(2, pixels.Color(0, 0, 25));
      pixels.setPixelColor(3, pixels.Color(0, 0, 25));
      pixels.setPixelColor(4, pixels.Color(0, 0, 25));
      pixels.setPixelColor(5, pixels.Color(0, 0, 25));
      pixels.setPixelColor(6, pixels.Color(0, 0, 25));
      pixels.setPixelColor(7, pixels.Color(0, 0, 25));
      pixels.setPixelColor(8, pixels.Color(0, 0, 25));
      pixels.setPixelColor(9, pixels.Color(0, 0, 25));
      pixels.setPixelColor(10, pixels.Color(0, 0, 25));
      pixels.setPixelColor(11, pixels.Color(0, 0, 25));
      pixels.show();
      v = 1;
     }

     pixels.setPixelColor(h, pixels.Color(0, 0, 100));  
     pixels.setPixelColor(j, pixels.Color(0, 0, 25));
     pixels.show();   // Send the updated pixel colors to the hardware.
     j++;   
     h++;   
     delay(timedelay); // Pause before next pass through loop
     if (h == 12){
        h = 0;
     }
     if (j == 12){
        j = 0;
     }      
}

//void led_spin() {  
      //if (o == 0){
          //pixels.setPixelColor(e, pixels.Color(0, 25, 0));
          //pixels.show();   // Send the updated pixel colors to the hardware.
          //e++;    
          //delay(timedelay); // Pause before next pass through loop
          //if (e == 12){
          //  o = 1;
          //}
      //}
      //if (o == 1){
          //pixels.setPixelColor(e, pixels.Color(0, 0, 25));
          //pixels.show();   // Send the updated pixel colors to the hardware.
          //e--;    
          //delay(timedelay); // Pause before next pass through loop
          //if (e == -1){
            //o = 0;
          //}
      //}
//}

void led_wait() {                     // Function to let the user know that the process havent started, by blinking Blue and Green.
      if (d%2==0 && icounteren%2==0){
        pixels.setPixelColor(d, pixels.Color(0, 25, 0));
      }
      if (d%2==1 && icounteren%2==0){
        pixels.setPixelColor(d, pixels.Color(0, 0, 25));
      }
      
      if (d%2==0 && icounteren%2==1){
        pixels.setPixelColor(d, pixels.Color(0, 0, 25));
      }
      if (d%2==1 && icounteren%2==1){
        pixels.setPixelColor(d, pixels.Color(0, 25, 0));
      }
      if (d==12){
        icounteren=icounteren+1;
        d = -1;
        pixels.show();    // Send the updated pixel colors to the hardware.
        }
      d++;
}

void loop() {
  led_wait();
  if ( ! mfrc522.PICC_IsNewCardPresent()) {   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.  
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {     // Select the card read
    return;
  }
  
  readID();     // Get information on the card holder if in EEPROM

  int x = 0;      // Start address for EEPROM
  int y = 4;      // End address for EEPROM
  int z = 0;      // Information variable for EEPROM
  int k = 1;      // One time variable to print LCD message
  r = 1;          // Variable to switch loop
  int p = 0;      // Variable for counting currency attempts
  t;              // Variable to set the pin of the servo depending on which item was chosen
  
  while (r == 1) {
    getID = ""; 
    z = 0;
    for (int i = x; i < y; i++) { // For loop to read the data in EEPROM
      getID += EEPROM.read(i);
      z = z + i * 10;             // Let the Arduino know which place to read the name and currency
    }
    
    if (getID == readCard) {      // If id on card is equal to values in EEPROM
      displayName = "";
      for (int i = z+1; i < 1024; i++) {
        if (EEPROM.read(i) == 0) {        // When a "0" is read break
          break;
        }
        getName = EEPROM.read(i);         // Saves the character from the read EEPROM
        displayName += getName;           // Adds the character together to form a name
      }
      lcdscroll(0,0,displayName,"Hello, ");
      delay(500);
      r = 2;                     // Changes while loop value
      break; 
    }

    if (y == 24) {               // If run 5 times break and let the user know that the card is not saved in EEPROM
      lcd.clear();
      led_error();
      d = 0;                          // Resets variable in led_wait
      lcd.print("Not authorized");
      delay(2000);
      lcd.clear();
      lcd_start_message();
      break;
    }
    
    x = x + 4;    // Equation to calculate start address value to check next set of data in EEPROM
    y = y + 4;    // Equation to calculate end address value to check next set of data in EEPROM
    
  }

  itemID = "";                      // Resets itemID string
  while(r == 2){
    inactivity_1();                 
    //led_spin();
    led_spin2();                    
    exitprogram();
    char key = keypad.getKey();     // Savse the user's keypad press
    if (key != NO_KEY && key != 'A' && key != 'B' && key != 'C' && key != '#' && key != '*'){   // If keypad was pressed and a number
        itemID += key;                                          // Saves keypad press in itemID
        c = 0;                                                  // Reset inactive timer variable 1 
        lcd.clear();
        lcd.print("Item: #"+itemID);
    }
    if (key == 'A'){                                        // If key "A" was pressed on keypad prints currency by reading the EEPROM
        lcdclearline(0,1);
        lcd.setCursor(0,1);
        c = 0;                                                  // Reset inactive timer variable 1 
        lcd.print("You have "+String(EEPROM.read(z))+"kr.");
    }
    if (key == '#') {                                       // If key "#" was pressed on keypad resets itemID and restarts item selection process
          lcd.clear();
          lcd.print("Re-enter ITEM ID");
          itemID = "";
          c = 0;                                            // Reset inactive timer variable 1 
          k = 1;                                            // Reset one time variable to print LCD message
    }

      while(itemID.length() == 2){                      // If itemID is 2 digits
        inactivity_2();
        led_button();
        if (itemID == "10") {       // Checks the itemID
            t = 31;                 // Variable to set servo pin to 31
          }
        if (itemID == "11") {       // Checks the itemID
            t = 33;                 // Variable to set servo pin to 33
          }
        if (itemID == "12") {       // Checks the itemID
            t = 35;                 // Variable to set servo pin to 35
          }
        if (itemID == "13") {       // Checks the itemID
            t = 37;                 // Variable to set servo pin to 37
          }
        exitprogram();
        if (k == 1) {               // Prints a one time message
          lcd.setCursor(0,1);
          lcd.print("Confirm purchase");
          b = 0;                                      // Reset Inactive timer variable 2
          k = 0;                                      // Reset one time variable to print LCD message
        }
        enterButtonState = digitalRead(enterButton);
        char key = keypad.getKey(); 
        if (enterButtonState == LOW) {          // If green enter button is pressed
          int m = EEPROM.read(z);               // Reads how much currency is on the card
          n = itemArray[itemID.toInt()];        // Convert the itemID to an an integer and selects the price that corresponds to that number
          
          if (n != 10 && n != 15 && n != 20) {    // Checks if the price of the item is valid
            led_error();
            lcd.clear();
            lcd.print("Error...");
            lcd.setCursor(0,1);
            lcd.print("Item unavailable");
            delay(2000);
            lcd.clear();
            lcd.print("Re-enter ITEM ID");
            itemID = "";
            k = 1;                            // Reset one time variable to print LCD message
            //e = 12;
            //o = 1;
            c = 0;                                  // Reset inactive timer variable 1 
            h = 0;                                  // Reset variable in led_spin2
            v = 0;                                  // Reset variable in led_spin2
            j = -1;                                 // Reset variable in led_spin2
            
            break;
          }
          
          if (m-n < 0) {                            // Checks if enough currency
            h = 0;                                  // Reset variable in led_spin2
            v = 0;                                  // Reset variable in led_spin2
            j = -1;                                 // Reset variable in led_spin2
            //e = 12;
            //o = 1;
            c = 0;                              // Reset inactive timer variable 1 
            led_error();
            lcd.clear();
            lcd.print("Order denied");
            lcd.setCursor(0,1);
            lcd.print("Not enough money");
            
            if (p <= 1){                          // The user have the ability to try multiple times even though there isnt enough currency on the card
              delay(2000);
              lcd.clear();
              lcd.print("Re-enter ITEM ID");
              itemID = "";
              k = 1;                              // Reset one time variable to print LCD message
              c = 0;                              // Reset inactive timer variable 1 
              p++;                                // Keeps count of user tries
              break;
              }
            if (p == 2){                          // If the user have tried a couple of times exit the system
              delay(2000);
              lcd.clear();
              lcd.print("Exiting...");
              delay(2000);
              lcd.clear();
              p = 0;
              c = 0;                          // Reset inactive timer variable 1 
              itemID = "";
              r = 1;                          // Changes while loop value
              lcd_start_message();
              }              
            }
            
          if (m-n >= 0) {                    // Checks currency
            lcd.clear();
            lcd.print("Order confirmed");
            led_succes();
            servo = servo+itemID;             // Combines the itemID with the servo name
            Servo servo;                      // Creates and instance of the combined servo name
            servo.attach(t);                  // Attach the servo to the assigned pin
            
            if (t == 31) {  //Checks which direction the Servo needs to rotate
              w = 50;           // Sets angle on servo
              g = 850;          // Sets delay on servo
            }
            if (t == 33) {  //Checks which direction the Servo needs to rotate
              w = 150;          // Sets angle on servo
              g = 900;          // Sets delay on servo
            }
            if (t == 35) {  //Checks which direction the Servo needs to rotate
              w = 50;           // Sets angle on servo
              g = 800;          // Sets delay on servo
            }
            if (t == 37) {  //Checks which direction the Servo needs to rotate
              w = 150;          // Sets angle on servo
              g = 850;          // Sets delay on servo
            }
            
            servo.write(w);           // Writes to servo
            delay(g);                 // Adds a little delay before servo is stopped
            servo.detach();           // Deatch the servo to stop it and save some power
            lcd.setCursor(0,1);
            EEPROM.update(z,m-n);     // Update the user's currency in the EEPROM
            lcd.print(n);
            lcd.print("kr deducted");            
            delay(3000);
            lcd.clear();
            lcd.print("Have a nice day!");
            delay(2000);
            lcd.clear();
            itemID = "";
            r = 1;                                // Changes while loop value
            c = 0;                                // Reset inactive timer variable 1 
            lcd_start_message();
            break;
          }
        }
        
        if (key == '#') {                      // The user have the ability ro re-enter the itemID by pressing "#" on the keypad
          lcd.clear();
          lcd.print("Re-enter ITEM ID");
          itemID = "";
          k = 1;                            // Reset one time variable to print LCD message
          //e = 12;
          //o = 1;
          c = 0;                          // Reset inactive timer variable 1 
          h = 0;                          // Reset variable in led_spin2
          v = 0;                          // Reset variable in led_spin2
          j = -1;                         // Reset variable in led_spin2
          break;
        }
      }
    }
  }
