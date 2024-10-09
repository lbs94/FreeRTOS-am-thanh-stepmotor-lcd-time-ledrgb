#include <Key.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
// Include the libraries for the stepper motor driver, LCD, and keyboard matrix
#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <stdio.h>
#include <RtcDS1302.h>

ThreeWire myWire(30,32,28); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// Define the pins for the stepper motor driver
#define DIR_PIN 9
#define STEP_PIN 10
#define ENABLE_PIN 11

// Define the pins for the LEDs
#define LED1_PIN A13
#define LED2_PIN A14
#define LED3_PIN A15
// Define RGB LED pins
#define RED_PIN 6
#define GREEN_PIN 4
#define BLUE_PIN 5
// Define the pins for the LCD
#define RS_PIN 52
#define EN_PIN 53
#define D4_PIN 48
#define D5_PIN 49
#define D6_PIN 50
#define D7_PIN 51

// Define the rows and columns for the keyboard matrix
#define ROWS 4
#define COLS 4
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte rowPins[ROWS] = {A3, A2, A1, A0};
byte colPins[COLS] = {A4, A5, A6, A7};

// Define the stepper motor object
Stepper stepper(800, DIR_PIN, STEP_PIN);

// Define the LCD object
LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

// Define the keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Define the shared variable for the stepper motor status
int motorStatus = 0; // 0 = stopped, 1 = running, 2 = direction reversed
int last_event = 1;
char buff[50];
// Define the task functions

void taskMotor(void *pvParameters) {
int steps = 1600; // Number of steps to move
int delayTime = 500; // Delay time between steps in microseconds
while (1) {
if (motorStatus == 1) {
// Control the stepper motor
stepper.setSpeed(200); // Set the speed in steps per minute
stepper.step(steps); // Move the specified number of steps
vTaskDelay(pdMS_TO_TICKS(delayTime)); // Delay between steps
stepper.step(-steps); // Move the specified number of steps
vTaskDelay(pdMS_TO_TICKS(delayTime)); // Delay between steps
} else {
vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds if the motor is stopped
}
if (motorStatus == 2) {
// Control the stepper motor
stepper.setSpeed(200); // Set the speed in steps per minute
stepper.step(800); // Move the specified number of steps
stepper.step(0);
} else {
vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds if the motor is stopped
}
}
}

void taskLED(void *pvParameters) {
while (1) {
// Control the LEDs based on the motor status

if (motorStatus == 1) {
  digitalWrite(LED1_PIN, HIGH);
  //digitalWrite(LED2_PIN, LOW);
  //digitalWrite(LED3_PIN, HIGH);
} else if (motorStatus == 2) {
digitalWrite(LED1_PIN, LOW);
//digitalWrite(LED2_PIN, HIGH);
digitalWrite(LED3_PIN, HIGH);
} else {
digitalWrite(LED1_PIN, LOW);
//digitalWrite(LED2_PIN, LOW);
digitalWrite(LED3_PIN, LOW);
}
vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
}
}

void taskLCD(void *pvParameters) {
while (1) {
// Display the motor status on the LCD
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Motor: ");
if (motorStatus == 1) {
lcd.setCursor(6, 0);
lcd.print("Running");
} else if (motorStatus == 2) {
lcd.setCursor(6, 0);
lcd.print("Reversed");
} else {
lcd.setCursor(6, 0);
lcd.print("Stopped ");
}
lcd.setCursor(10, 1);
lcd.print("Luc Le");
 lcd.setCursor(0, 1);
 lcd.print(buff);
vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
}
}

void taskKeyboard(void *pvParameters) {
while (1) {
// Read input from the keyboard matrix and update the motor status
char key = keypad.getKey();
if (key != NO_KEY) {
switch (key) {
case 'A':
motorStatus = 1; // Start the motor
break;
case 'B':
motorStatus = 0; // Stop the motor

break;
case 'C':
motorStatus = 2; // Reverse the motor direction

break;
case 'D':
Serial.println("khong co gi");
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Hello LeVanLuc");
vTaskDelay(pdMS_TO_TICKS(200)); // Delay for 100 milliseconds
break;
case '1':
        // Turn on LED to red
        analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
        break;
      case '2':
        // Turn on LED to green
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
        break;
      case '3':
        // Turn on LED to blue
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
        break;
      case '0':
        // Turn off LED
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
        break;
default:
break;
}
lcd.setCursor(15, 0);
lcd.print(key);
}
vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
}
}
void tasksound(void *pvParameters)
{
while(1)
{
int sound = digitalRead(7);
  if(sound==LOW)
  {
    last_event++;
    Serial.println("da nhan duoc am thanh");
    if(last_event %2 == 0)
      {
        digitalWrite(LED2_PIN,HIGH);
        // vTaskDelay(pdMS_TO_TICKS(100));
      }
    if(last_event %2 != 0)
      {
        digitalWrite(LED2_PIN,LOW);
      }
    if(last_event==10)
      {
        last_event=1;
      }
  }
}

}
void ds1302Task(void *pvParameters) {
  while (true) {
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[26];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),//%02u/%02u/%04u
            //dt.Month(),
            //dt.Day(),
            //dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
            lcd.setCursor(0, 1);
            lcd.print(datestring); 
    Serial.println(datestring);
    if(dt.Hour()==16 && dt.Minute()==00)
    {
      Serial.println("Come back Home!!");
      lcd.setCursor(0, 0);
      lcd.print("Come back Home!");
      analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
      analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
         vTaskDelay(pdMS_TO_TICKS(50));
    }
    if(dt.Hour()==16 && dt.Minute()==01)
    {
      Serial.println("Come back Home!!");
      lcd.setCursor(0, 0);
      lcd.print("Come back Home!");
      analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
      analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
         vTaskDelay(pdMS_TO_TICKS(50));
    }
    if(dt.Hour()==07 && dt.Minute()==31)
    {
      Serial.println("Time Working!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time Working!!!");
      lcd.setCursor(0, 1);
      lcd.print("have a good day!");
    }

    if(dt.Hour()==07 && dt.Minute()==30)
    {
      Serial.println("Time Working!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time Working!!!");
      lcd.setCursor(0, 1);
      lcd.print("have a good day!");
      analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
         vTaskDelay(pdMS_TO_TICKS(50));
    }
    if(dt.Hour()==07 && dt.Minute()==29)
    {
      Serial.println("Time Working!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time Working!!!");
      lcd.setCursor(0, 1);
      lcd.print("have a good day!");
    }

    if(dt.Hour()==00 && dt.Minute()==00)
    {
      Serial.println("Good Night,have a nice dream!!!");
      lcd.setCursor(0, 0);
      lcd.print("Good Night!!!");
      lcd.setCursor(0, 1);
      lcd.print("Nice Dream");
    }
    if(dt.Hour()==21 && dt.Minute()==30)
    {
      Serial.println("Time to sleep!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time to sleep!!!");
      lcd.setCursor(0, 1);
      lcd.print("Nice Dream");
    }
    if(dt.Hour()==22 && dt.Minute()==00)
    {
      Serial.println("Time to sleep!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time to sleep!!!");
      lcd.setCursor(0, 1);
      lcd.print("Nice Dream");
        analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
      analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
         vTaskDelay(pdMS_TO_TICKS(50));
    }
    if(dt.Hour()==22 && dt.Minute()==01)
    {
      Serial.println("Time to sleep!!!");
      lcd.setCursor(0, 0);
      lcd.print("Time to sleep!!!");
      lcd.setCursor(0, 1);
      lcd.print("Nice Dream");
      analogWrite(RED_PIN, 255);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
        analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 255);
        analogWrite(BLUE_PIN, 0);
       vTaskDelay(pdMS_TO_TICKS(50));
      analogWrite(RED_PIN, 0);
        analogWrite(GREEN_PIN, 0);
        analogWrite(BLUE_PIN, 255);
       vTaskDelay(pdMS_TO_TICKS(50));

    }
}

void setup() {
Serial.begin(9600);
// Set the pins for the LEDs as outputs
pinMode(LED1_PIN, OUTPUT);
pinMode(LED2_PIN, OUTPUT);
pinMode(LED3_PIN, OUTPUT);
pinMode(7,INPUT);
pinMode(RED_PIN, OUTPUT);
pinMode(GREEN_PIN, OUTPUT);
pinMode(BLUE_PIN, OUTPUT);
// Set the pins for the stepper motor driver as outputs
pinMode(DIR_PIN, OUTPUT);
pinMode(STEP_PIN, OUTPUT);
pinMode(ENABLE_PIN, OUTPUT);
// Set the pins for the LCD as outputs
lcd.begin(16, 2);


    Serial.print("Compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid()) 
    {
        //    Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
// Start the FreeRTOS scheduler
xTaskCreate(taskMotor, "Motor", configMINIMAL_STACK_SIZE, NULL, 6, NULL);
xTaskCreate(taskLED, "LED", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
xTaskCreate(taskLCD, "LCD", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
xTaskCreate(taskKeyboard, "Keyboard", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
xTaskCreate(tasksound,"sound",configMINIMAL_STACK_SIZE,NULL,2,NULL);

xTaskCreate(ds1302Task, "DS1302Task", configMINIMAL_STACK_SIZE, NULL, 6, NULL);
vTaskStartScheduler();
}

void loop() {
// Nothing to do here since the tasks are handled by the scheduler
}