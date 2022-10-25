#define BUILD_DISPLAY
//#define BUILD_BLUETOOTH

#include <EEPROM.h>
#include "menu.h" 
//DISPLAY
#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//BLUETOOTH
//#include "bluetooth.h"
//#include <SoftwareSerial.h>
//SoftwareSerial BTSerial (8,9);

#define timer_freq 3000

#define speed_in_pin 2 // j8, verde
#define speed_out_pin 3 // j5, vermelho/laranja
#define wasteGateOut 4
#define intakeAirTempOut 5
#define menuButton 6
#define relayOut 10 // j11
#define consume_pin 11 // j4, branco/cinza
#define injector_pin 12 // j3, roxo
#define beep 13 // internal
#define wideBandSensor A0
#define voltageIn A1 // internal
#define sensorPressure A2 // azul
#define intakeAirTemp A3
#define wasteGate A6
#define sensorPressure2 A7 // marrom

#define injetor 23 // vazao do injetor a 12v, em lbs/h

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
unsigned char speedLimit, doorLockspd;
bool doorLocked = false, speedBeep = false, rpmBeep = false;

float odometer = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(injector_pin,INPUT);
  pinMode(speed_in_pin,INPUT);
  pinMode(voltageIn, INPUT);
  pinMode(sensorPressure,INPUT);
  pinMode(wideBandSensor,INPUT);
  pinMode(intakeAirTemp,INPUT);
  pinMode(wasteGate,INPUT);
  pinMode(sensorPressure2,INPUT);
  pinMode(relayOut, OUTPUT);
  pinMode(beep, OUTPUT);
  pinMode(consume_pin,OUTPUT);
  pinMode(speed_out_pin,OUTPUT);
  pinMode(wasteGateOut,OUTPUT);
  pinMode(intakeAirTempOut,OUTPUT);
  pinMode(menuButton,INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(menuButton), changeMenu, CHANGE);
  
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


void loop()
{
  if (Serial.available() > 0)
    Menu();
  #ifdef BUILD_DISPLAY
  if (digitalRead(menuButton) == LOW)
    changeMenu();
  #endif
    
  float volts = analogRead(voltageIn)*0.0197f;
  
  inputFreq injectorInput, speedInput;
  readFrequency(injector_pin, &injectorInput);
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
    setOutFrequency(consumption,0); 
  
  readFrequency(speed_in_pin, &speedInput);
  if (speedInput.period == 0.0)
    setOutFrequency(0.0,1);  
  else
    setOutFrequency(speedInput.freq,1);
   
  //Calculate distance
  unsigned long elapsedtime = millis() - last_millis;
  odometer += (elapsedtime*out_freq[1])/((float) speedSensor); //meters
  if (odometer > 500.0){
    tripA += odometer;
    totalMileage += odometer;
    odometer = 0.0;
    EEPROM.put(4, totalMileage);
  }
  last_millis = millis();

  //Speed
  unsigned char currentSpeed = (unsigned char) (out_freq[1]/((float) (speedSensor/3600.0f)));    
  speedManager(currentSpeed);
  
  /* Alerts */
  int rpm = injectorInput.freq*60*((settings & 2 == 0)*2);
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
  delay(10);
}

void setupTimer1(){
  TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
  TCNT1 = 65536-(16000000/1024/timer_freq); //configura timer
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
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
  EEPROM.get(17,doorLockspd);
  EEPROM.get(18,settings);
}

void speedManager(int currentSpeed){
    if (doorLockspd > 0 && !doorLocked && currentSpeed > doorLockspd){
    digitalWrite(relayOut,HIGH);
    delay(300);
    digitalWrite(relayOut,LOW);
    doorLocked=true;
  }
  if (speedLimit > 0){
    if (currentSpeed > speedLimit){
        if (settings % 2 == 0) //continuo ou curto
          digitalWrite(beep,HIGH);
        else{
          if (!speedBeep){
          digitalWrite(beep,HIGH);
          delay(80);
          digitalWrite(beep,LOW);
          delay(80);
          digitalWrite(beep,HIGH);
          delay(80);
          digitalWrite(beep,LOW);
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
    digitalWrite(relayOut,HIGH);
    setOutFrequency(3,2);
  }
  /*int sensorPressureVal2 = map(analogRead(A7), 204, 1024, 0, 10000);
  if (rpmAlert > 0 && rpm > rpmAlert && sensorPressureVal2 < minPressure){
    digitalWrite(relayOut,HIGH);
    setOutFrequency(3,2);
  }*/
  return sensorPressureVal;
}

void readFrequency(int pin, inputFreq *returnedValues){
  (*returnedValues).ontime = pulseInLong(pin,HIGH,200000);
  if ((*returnedValues).ontime == 0){
     (*returnedValues).offtime = 0.0; 
     (*returnedValues).period = 0.0;
     (*returnedValues).freq = 0.0;
  } else{
    (*returnedValues).offtime = pulseInLong(pin,LOW,200000); 
    (*returnedValues).period = ((*returnedValues).ontime+(*returnedValues).offtime);
    (*returnedValues).freq = (1000000.0f/(*returnedValues).period);    
  }
}

//Setup the frequency to output
void setOutFrequency(float baseFreq, int num){
  if (baseFreq > 400.0)
    baseFreq=400.0f;
  if (baseFreq < 2.0){
    //desabilita aquela saida
    digitalWrite(pulse_pin[num],HIGH);
    previousTime[num] = micros(); 
    baseFreq=0.0;
  }
  float drift=0;
  if (correction_drift[num] > 0)
    drift=(float) (correction_drift[num]/8191.75f)+1.0f;
  else
    drift=(float) (correction_drift[num]+32767)/32767.0f;
  out_freq[num]=(baseFreq*drift);
}


/* This timer runs 3k times a second and verify if need to change state of any pulse output
 */
ISR(TIMER1_OVF_vect)
{
 unsigned long currentTime = micros();
 for (int i=0;i < n_saidas_pulso; i++){
    if ((out_freq[i] > 2.0) &&
        (currentTime - previousTime[i]) >= (unsigned long)(1000000.0f/(out_freq[i]*2))) { //each pulse is high and low, so freq is double
      previousTime[i] = currentTime;
      digitalWrite(pulse_pin[i],!digitalRead(pulse_pin[i]));
    }
 }
 TCNT1 = 65536-(16000000/1024/timer_freq); // timer reset
}
