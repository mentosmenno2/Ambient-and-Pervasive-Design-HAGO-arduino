#include <Boards.h>
#include <Firmata.h>

#include <SPI.h>
#include <MFRC522.h>

#include "Timer.h"
#include <Servo.h>

//Events.
int lockEvent;
int tickEvent;

//5 second open delay before the door closes after card is removed.
int openDuration = 5000;
//Global timer.
Timer t;

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

//ID array
String accessIDS[1] = {"10619717181"};
//other card id: 2612915181

Servo myservo;

//Check if door is open.
bool open = false;
bool error = false;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Scan PICC to see UID and type...");
  
  //initialize LED
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level

  myservo.attach(6);
}

//Lock function called after timer ends.
void lock()
{
    digitalWrite(8, LOW);
    digitalWrite(7, HIGH);
    Serial.println("Door is locked.");
    t.stop(lockEvent);
    t.stop(tickEvent);
    open = false;

    myservo.write(90);   // sets the servo at 180 degree position
}

//Opened function called every second the door is open.
void opened()
{
    Serial.println("Door is open."); 
    digitalWrite(8, HIGH);  
    digitalWrite(7, LOW); 
}

void loop() 
{
  t.update();
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }  

  //Get the bytes from the card and convert them to a string value.
  String checkID = (String(mfrc522.uid.uidByte[0]) + String(mfrc522.uid.uidByte[1]) + String(mfrc522.uid.uidByte[2]) + String(mfrc522.uid.uidByte[3]));

  //Serial.println(checkID);

  error = false; 

  //Loop through the access id array and check if it matches the card.
  for(int i = 0; i <  sizeof(accessIDS); i++)
  {
      //Check if ID matches and if the door is not open yet open it.
      if (checkID == accessIDS[i] && !open ) 
      {
          Serial.println("Card ID Checked and confirmed: " + checkID);
          tickEvent = t.every(1000, opened);
          lockEvent = t.after(openDuration, lock);

          myservo.write(180);   // sets the servo at 180 degree position
          open = true;
      }
      //If door is already open keep it open and delay the lock event.
      else if(checkID == accessIDS[i] && open)
      {
          t.stop(lockEvent);
          lockEvent = t.after(openDuration, lock);

      }
      else
      {
          error = true;
      }
    }
    if(error && !open)
    {
        digitalWrite(7, LOW);
        delay(100);
        digitalWrite(7, HIGH);
        delay(100);
        digitalWrite(7, LOW);
        delay(100);
        digitalWrite(7, HIGH);
        delay(100);
        digitalWrite(7, LOW);
        delay(100);
        digitalWrite(7, HIGH);
        delay(100);
        error = false; 
    }
}
