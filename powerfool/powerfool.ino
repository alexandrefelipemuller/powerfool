#define BUILD_DISPLAY
#define BUILD_BLUETOOTH
//#define is_ESP32
//#define is_ATM168p

#include <EEPROM.h>
#include "menu.h" 
#include "iotools.h"
//DISPLAY
#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//BLUETOOTH
#include "bluetooth.h"
#include <SoftwareSerial.h>
SoftwareSerial BTSerial (7,8);

#define timer_freq 3000

#define setButton 2
#define menuButton 3
#define beep 4
#define speed_in_pin 5
#define injector_pin 6
#define RL3 9
#define RL2 10
#define RL1 11
#define speed_out_pin 12
#define consume_pin 13
#define breakLight A0
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
#ifndef is_ATM168p
float fuel = 0;
unsigned int tank;
unsigned char doorLockspd;
#endif
void setup()
{
  Serial.begin(9600);
  pinMode(injector_pin,INPUT);
  pinMode(speed_in_pin,INPUT_PULLUP);
  pinMode(voltageIn, INPUT);
  pinMode(sensorPressure,INPUT);
  pinMode(beep, OUTPUT);
  pinMode(consume_pin,OUTPUT);
  pinMode(speed_out_pin,OUTPUT);
  pinMode(RL1,OUTPUT);
  pinMode(RL2,OUTPUT);
  pinMode(RL3,OUTPUT);
  pinMode(setButton,INPUT_PULLUP);
  pinMode(menuButton,INPUT_PULLUP);
  
  #ifdef BUILD_DISPLAY
  attachInterrupt(digitalPinToInterrupt(setButton), setMenu, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(menuButton), changeMenu, CHANGE);
  #endif
  pulsePin(beep,500);
  pulse_pin[0]=consume_pin;
  pulse_pin[1]=speed_out_pin;
  pulse_pin[2]=beep;
  
  loadMemoryValues();
  
  setupTimer1(); 

  #ifdef BUILD_DISPLAY
    lcd.init();
    lcd.backlight(); 
  #endif
  #ifdef BUILD_BLUETOOTH
    BTSerial.begin(9600);
  #endif
}

void loadMemoryValues(){
    if (EEPROM.read(0) == 255){
    //First boot, clear memory
    EEPROM.put(0,(int)0);
    EEPROM.put(2,(int)0);
    EEPROM.put(4,(long)0);
    EEPROM.put(8,(int)0);
    EEPROM.put(10,(int)0);
    EEPROM.put(12,(int)0);
    EEPROM.put(14,(int)4860);
    EEPROM.put(16,(char)0);
    EEPROM.put(17,(char)0);
    EEPROM.put(18,(int)0);
    EEPROM.put(20,(int)20000);
  } 
  //Load values
  EEPROM.get(0,correction_drift[0]);
  EEPROM.get(2,correction_drift[1]);
  EEPROM.get(4,totalMileage);
  EEPROM.get(8,rpmLimit);
  EEPROM.get(10,rpmAlert);
  EEPROM.get(12,minPressure);
  EEPROM.get(14,speedSensor);
  EEPROM.get(16,speedLimit);
  EEPROM.get(18,settings);
  #ifndef is_ATM168p
  EEPROM.get(17,doorLockspd);
  EEPROM.get(20,tank);
  #endif
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
  
  if (injectorInput.period == 0.0)
    setOutFrequency(0.0,0);  
  else
    setOutFrequency(consumption/0.083f,0); 
  
  readFrequency(speed_in_pin, 4, &speedInput);
  if (speedInput.period == 0.0)
    setOutFrequency(0.0,1);  
  else
    setOutFrequency(speedInput.freq,1);
   
  //Calculate distance and consumed fuel
  calculateDistante(millis() - last_millis);
  last_millis = millis();

  //Speed
  unsigned char currentSpeed = (unsigned char) (out_freq[1]/((float) (speedSensor/3600.0f)));    
  speedManager(currentSpeed);
  
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
  #ifndef is_ATM168p
    fuel += (out_freq[0]/1000)*elapsedtime;
    if (fuel > 500.0){
    tank-=fuel;
    fuel=0.0;       
    EEPROM.put(20, tank);
    }
  #endif
  odometer += (elapsedtime*out_freq[1])/((float) speedSensor); //meters
  if (odometer > 500.0){
    tripA += odometer;
    totalMileage += odometer;
    odometer = 0.0;
    EEPROM.put(4, totalMileage);
  }
}

void speedManager(int currentSpeed){
  #ifndef is_ATM168p
    /* Emergency Stop Signal */
    unsigned long elapsedTime = millis() - last_millis;
    unsigned long deceleration = (lastSpeed - currentSpeed)/(elapsedTime*1000);
    lastSpeed = currentSpeed; 
    if (digitalRead(breakLight) == HIGH && deceleration > 28){
      pulsePin(RL3,500);
    }

    /* door lock */
    if (doorLockspd > 0 && !doorLocked && currentSpeed > doorLockspd){
      pulsePin(RL2,300);
      doorLocked=true;
    }
  #endif
  /* Speed limit */
  if (speedLimit > 0){
    if (currentSpeed > speedLimit){
        if (settings % 2 == 0) //continuo ou curto
          digitalWrite(beep,HIGH);
        else{
          if (!speedBeep){
          pulsePin(beep,80);
          pulsePin(beep,80);
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
    digitalWrite(RL1,HIGH);
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
