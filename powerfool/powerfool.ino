#define BUILD_DISPLAY
#define BUILD_BLUETOOTH
//#define is_ESP32
//#define is_TEST true //Used to test hardware

#include <EEPROM.h>
#include "menu.h" 
#include "iotools.h"
#include "memutils.h"
//DISPLAY
#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//BLUETOOTH
#include "bluetooth.h"
#include <SoftwareSerial.h>
SoftwareSerial BTSerial (8,9);

#define timer_freq 3000

#define setButton 2
#define menuButton 3
#define beep 4
#define DI2 5
#define DI1 6
#define DI3 A0
#define RL3 7
#define RL2 10
#define RL1 11
#define DO1 12
#define DO2 13
#define voltageIn A1 // internal
#define sensorPressure A2
#define intakeAirTemp A3
#define sensorTemp A6
#define sensorPressure2 A7

#define injetor 20.0 // vazao do injetor a 12v, em lbs/h

// set up pulse pins
#define n_saidas_pulso 3
float out_freq[n_saidas_pulso] = {1,1,1};
unsigned long previousTime[n_saidas_pulso] = {0,0,0};
//Define pinos para pulsar, devem ser iniciador em setup
int pulse_pin[n_saidas_pulso] = {-1,-1,-1}; 
int correction_drift[n_saidas_pulso] = {0,0,0}; 

bool diagnostic_mode = false;

unsigned long last_millis, totalMileage, tripA;
unsigned int rpmLimit, rpmAlert, minPressure, speedSensor, settings;

unsigned char speedLimit, lastSpeed;
bool doorLocked = false, speedBeep = false, rpmBeep = false;

float odometer = 0;
float fuel = 0;
unsigned int tank;
unsigned char doorLockspd;

unsigned char RL_P, RL_DL, RL_HZ, speed_in_pin, injector_pin, breakLight;
unsigned char consume_pin = DO1;
unsigned char speed_out_pin = DO2;

void setup()
{
  Serial.begin(9600);
  pinMode(DI1,INPUT);
  pinMode(DI2,INPUT_PULLUP);
  pinMode(DI3,INPUT);
  pinMode(voltageIn, INPUT);
  pinMode(sensorPressure,INPUT);
  pinMode(sensorPressure2, INPUT);
  pinMode(intakeAirTemp,INPUT);
  pinMode(sensorTemp,INPUT);
  pinMode(beep, OUTPUT);
  pinMode(DO2,OUTPUT);
  pinMode(DO1,OUTPUT);
  pinMode(RL1,OUTPUT);
  pinMode(RL2,OUTPUT);
  pinMode(RL3,OUTPUT);
  pinMode(setButton,INPUT_PULLUP);
  pinMode(menuButton,INPUT_PULLUP);
  
  #ifdef BUILD_DISPLAY
  attachInterrupt(digitalPinToInterrupt(setButton), setMenu, RISING);  
  attachInterrupt(digitalPinToInterrupt(menuButton), changeMenu, RISING);
  #endif
  pulse_pin[0]=consume_pin;
  pulse_pin[1]=speed_out_pin;
  pulse_pin[2]=beep;
  
  loadMemoryValues();
  setupTimer1(); 

  #ifdef BUILD_DISPLAY
    lcd.init();
    lcd.createChar(0, degreeSymbol);
    lcd.backlight(); 
  #endif
  #ifdef BUILD_BLUETOOTH
    BTSerial.begin(9600);
  #endif
   pulsePinOnce(beep,500);
}

#ifdef is_ESP32
  hw_timer_t *esp_timer = NULL;
  void setupTimer1(){
      esp_timer = timerBegin(0, 80, true);
      timerAttachInterrupt(esp_timer, &onTimer, true);
      timerAlarmWrite(esp_timer, (1000000/timer_freq), true);
      timerAlarmEnable(esp_timer);
  }
#else
  void setupTimer1(){
    TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
    TCCR1B = 0;                        //limpa registrador
    TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
    TCNT1 = 65536-(16000000/1024/timer_freq); //configura timer
    TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
  }
#endif


void loop()
{
  if (Serial.available() > 0)
    Menu();

  #ifdef BUILD_DISPLAY
  if (digitalRead(setButton) != LOW)
    releaseSetButton();
  if (digitalRead(menuButton) != LOW)
    releaseMenuButton();
  #endif
  
  float volts = analogRead(voltageIn)*0.0197f;
  
  inputFreq injectorInput, speedInput;
  readFrequency(injector_pin, 2, &injectorInput);
  float duty;
  if ((float)(injectorInput.period) == 0.0f)
    duty=0.0;
  else
    duty = (float)injectorInput.offtime/(float)(injectorInput.period); 
   
  float vazao = injetor * (0.126); // result in ml/s 
  float consumption = injectorInput.freq*vazao*duty;

  readFrequency(speed_in_pin, 4, &speedInput);
  
  #ifndef is_TEST  
    if (injectorInput.period == 0.0)
      setOutFrequency(0.0,0);  
    else
      setOutFrequency(consumption/0.083f,0); 
    
    if (speedInput.period == 0.0)
      setOutFrequency(0.0,1);  
    else
      setOutFrequency(speedInput.freq,1);

   //Calculate distance and consumed fuel
   calculateDistante(millis() - last_millis);
  #else
    pinMode(injector_pin,INPUT_PULLUP);
    pinMode(speed_in_pin,INPUT_PULLUP);
    setOutFrequency(120.0f,0);
    setOutFrequency(120.0f,1); 
  #endif

  //Speed
  unsigned char currentSpeed = (unsigned char) (out_freq[1]/((float) (speedSensor/3600.0f)));    
  speedManager(currentSpeed);
  
  last_millis = millis();
  
  /* Alerts */
  int rpm = injectorInput.freq*60*(((settings & 2 == 0)+1)*2);
  int sensorPressureVal = alertsManager(rpm);

  if (diagnostic_mode){
    clearScreen();
    diagnosticReport(injectorInput, speedInput, consumption, volts, sensorPressureVal);
    delay(200);
  }
  #ifdef BUILD_DISPLAY
    refreshMenu(injectorInput, speedInput, consumption, volts, sensorPressureVal);
  #endif
  #ifdef BUILD_BLUETOOTH
    sendBluetooth(injectorInput, speedInput, volts, sensorPressureVal);
  #endif
  delay(5);
}
void calculateDistante(unsigned long elapsedtime){
    fuel += (out_freq[0]/1000)*elapsedtime;
    if (fuel > 500.0){
    tank-=fuel;
    fuel=0.0;       
    EEPROM.put(20, tank);
    }
  odometer += (elapsedtime*out_freq[1])/((float) speedSensor); //meters
  if (odometer > 500.0){
    tripA += odometer;
    totalMileage += odometer;
    odometer = 0.0;
    EEPROM.put(4, totalMileage);
  }
}

void speedManager(int currentSpeed){
    /* Emergency Stop Signal */
    unsigned long elapsedTime = millis() - last_millis;
    unsigned long deceleration = (currentSpeed - lastSpeed)/(elapsedTime*1000);
    lastSpeed = currentSpeed; 
    if (digitalRead(breakLight) == HIGH && deceleration > 28){
      pulsePinOnce(RL_HZ,500);
    }

    /* door lock */
    if (doorLockspd > 0 && !doorLocked && currentSpeed > doorLockspd){
      pulsePinOnce(RL_DL,300);
      doorLocked=true;
    }
    
  /* Speed limit */
  if (speedLimit > 0){
    if (currentSpeed > speedLimit){
        if (settings % 2 == 0) //continuo ou curto
          digitalWrite(beep,HIGH);
        else{
          if (!speedBeep){
          pulsePinOnce(beep,80);
          pulsePinOnce(beep,80);
          }
        }
        speedBeep=true;
    }
    else{
      digitalWrite(beep,LOW);
      speedBeep=false;
    }
  }
}

int alertsManager(int rpm){
  if (rpmLimit > 0){
    if (rpm > rpmLimit){
      digitalWrite(beep,HIGH);
      rpmBeep=true;
    }else{
      if (rpmBeep){
        digitalWrite(beep,LOW);
        rpmBeep=false;
      }  
    }
  }
  int sensorPressureVal = map(analogRead(A2), 204, 1024, 0, 10000);
  if (rpmAlert > 0 && rpm > rpmAlert && sensorPressureVal < minPressure){
    digitalWrite(RL_P,HIGH);
    setOutFrequency(3,2);
  }
  return sensorPressureVal;
}


/* This timer runs 3k times a second and verify if need to change state of any pulse output
 */
#ifdef is_ESP32
void IRAM_ATTR onTimer(){
 timerLoop(); 
}
#else
ISR(TIMER1_OVF_vect)
{
  timerLoop();
  TCNT1 = 65536-(16000000/1024/timer_freq); // timer reset
}
#endif
