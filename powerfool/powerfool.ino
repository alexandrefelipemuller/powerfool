#include <EEPROM.h>
#include "menu.h"
#include "bluetooth.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define timer_freq 3000

#define speed_in_pin 2
#define speed_out_pin 3
#define wasteGateOut 4
#define intakeAirTempOut 5
#define wideBandOut 6
#define relayOut 10
#define injector_pin 12
#define beep 13
#define wideBandSensor A0
#define voltageIn A1
#define sensorPressure A2
#define intakeAirTemp A3
#define wasteGate A6
#define sensorPressure2 A7

LiquidCrystal_I2C lcd(0x27,16,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial BTSerial (8,9);

// set up pulse pins
#define n_saidas_pulso 1
float out_freq[n_saidas_pulso] = {1};
unsigned long previousTime[n_saidas_pulso] = {0};
//Define pinos para pulsar, devem ser iniciador em setup
int pulse_pin[n_saidas_pulso] = {-1}; 
int correction_drift[n_saidas_pulso] = {0}; 

bool diagnostic_mode = false;

unsigned long last_millis, totalMileage;
unsigned int rpmBeep, rpmAlert, minPressure, speedSensor, settings;
int iatAdjust, wideAdjust, wasteGateAdjust;
unsigned char speedLimit;
unsigned char currentMap;
bool speedBeep = false;

float odometer = 0;

void setup()
{
  Serial.begin(9600);
  BTSerial.begin(9600);
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
  pinMode(speed_out_pin,OUTPUT);
  pinMode(wasteGateOut,OUTPUT);
  pinMode(intakeAirTempOut,OUTPUT);
  pinMode(wideBandOut,OUTPUT);
  
  pulse_pin[0]=beep;
  
  loadMemoryValues();
  
  setupTimer1(); 

  lcd.init();
  lcd.backlight();  
}


void displayReport(int rpm, float volts, int sensorPressure){
    lcd.setCursor(0,0);
    lcd.print(F("Bateria: "));
    lcd.print(volts);
    lcd.print(F(" v "));
    lcd.setCursor(0,1);
    lcd.print(F("Pressao: "));
    lcd.print(float(sensorPressure/1000.0f));
    lcd.print(F("bar"));
}

void loop()
{
 
  if (Serial.available() > 0)
    Menu();

  float volts = analogRead(voltageIn)*0.0197f;
  
  inputFreq injectorInput, speedInput;
  readFrequency(injector_pin, 3, &injectorInput);
  
  //Calculate distance
  unsigned long elapsedtime = millis() - last_millis;
  odometer += (elapsedtime*out_freq[1])/((float) speedSensor); //meters
  if (odometer > 500.0){
    totalMileage += odometer;
    odometer = 0.0;
    EEPROM.put(4, totalMileage);
  }
  last_millis = millis();

  readFrequency(speed_in_pin, 4, &speedInput);
  //Speed
  unsigned char currentSpeed = (unsigned char) (speedInput.freq/((float) (speedSensor/3600.0f)));    
  speedManager(currentSpeed);
  
  /* Alerts */
  int rpm = injectorInput.freq * 60 *(((settings ^ 2 > 0)+1)*2);
  int sensorPressureVal = alertsManager(rpm);

  /* Piggyback */
  int input = analogRead(wideBandSensor);
  float drift = getDrift(wideAdjust);
  input = input*drift;
  analogWrite(wideBandOut, map(input, 0, 1023, 0, 255));

  input = analogRead(wasteGate);
  drift = getDrift(wasteGateAdjust);
  input = input*drift;
  analogWrite(wasteGateOut, map(input, 0, 1023, 0, 255));

  analogRead(intakeAirTemp);
  drift = getDrift(iatAdjust);
  input = input*drift;
  analogWrite(intakeAirTempOut, map(input, 0, 1023, 0, 255));
  
  
  if (diagnostic_mode){
    clearScreen();
    diagnosticReport(injectorInput, speedInput, volts, sensorPressureVal);
    delay(100);
  }

  displayReport(rpm, volts, sensorPressureVal);
  sendBluetooth(injectorInput, speedInput, volts, sensorPressureVal);
  delay(50);
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
    EEPROM.put(0,(char)0);
    EEPROM.put(2,(int)0);
    EEPROM.put(4,(long)0);
    EEPROM.put(8,(int)0);
    EEPROM.put(10,(int)0);
    EEPROM.put(12,(int)0);
    EEPROM.put(14,(int)4860);
    EEPROM.put(16,(char)0);
    EEPROM.put(18,(int)0);
    EEPROM.put(21,(int)0);
    EEPROM.put(23,(int)0);
    EEPROM.put(25,(int)0);
  } 
  //Load values
  EEPROM.get(0,currentMap);
  EEPROM.get(4,totalMileage);
  EEPROM.get(8,rpmBeep);
  EEPROM.get(10,rpmAlert);
  EEPROM.get(12,minPressure);
  EEPROM.get(14,speedSensor);
  EEPROM.get(16,speedLimit);
  EEPROM.get(18,settings);
  EEPROM.get(21,iatAdjust);
  EEPROM.get(23,wideAdjust);
  EEPROM.get(25,wasteGateAdjust);
}

void speedManager(int currentSpeed){
  if (speedLimit > 0 && currentSpeed > speedLimit){
      if (settings % 2 == 0)
        digitalWrite(beep,HIGH);
      else{
        if (!speedBeep)
         digitalWrite(beep,HIGH);
        delay(50);
        digitalWrite(beep,LOW);
        speedBeep=true;
      }
  }
  else{
    digitalWrite(beep,LOW);
    speedBeep=false;
  }
}

int alertsManager(int rpm){
  if (rpmBeep > 0 && (rpm > rpmBeep)){
    digitalWrite(beep,HIGH);
  }else{
    digitalWrite(beep,LOW);
  }
  int sensorPressureVal = map(analogRead(A2), 204, 1024, 0, 10000);
  if (rpmAlert > 0 && rpm > rpmAlert && sensorPressureVal < minPressure){
    digitalWrite(relayOut,HIGH);
    setOutFrequency(3,0);
  }
  return sensorPressureVal;
}

// Read frequency and period from a pulse pin
void readFrequency(int pin, char samples, inputFreq *returnedValues){ 
  for (int i=0;i < samples; i++){
    unsigned long ontime = pulseInLong(pin,HIGH,250000);
    if (ontime == 0){
      (*returnedValues).offtime = 0.0; 
      (*returnedValues).period = 0.0;
      (*returnedValues).freq = 0.0;
      return;
    }
    (*returnedValues).ontime += ontime;
    (*returnedValues).offtime += pulseInLong(pin,LOW,250000);
  }
  (*returnedValues).offtime = (*returnedValues).offtime/samples;
  (*returnedValues).ontime = (*returnedValues).ontime/samples;
  (*returnedValues).period = ((*returnedValues).ontime+(*returnedValues).offtime);
  (*returnedValues).freq = (1000000.0f/(*returnedValues).period);    
}

// Convert memory settings to drift in float
float getDrift(int memValue){
  if (memValue > 0)
    return (float) (memValue/8191.75f)+1.0f;
  else
    return (float) (memValue+32767)/32767.0f;
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
  drift = getDrift(correction_drift[num]);
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
