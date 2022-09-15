#include <EEPROM.h>
#include "menu.h"
#define timer_freq 3000

#define injector_pin 12 // j3, roxo
#define consume_pin 11 // j4, branco/cinza
#define speed_in_pin 2 // j8, verde
#define speed_out_pin 3 // j5, vermelho/laranja
#define relayOut 10 // j11
#define beep 13 // internal
#define voltageIn A1 // internal

const float injetor = 23; // vazao do injetor a 12v, em lbs/h

// set up pulse pins
#define n_saidas_pulso 2
float out_freq[n_saidas_pulso] = {1,1};
unsigned long previousTime[n_saidas_pulso] = {0,0};
//Define pinos para pulsar, devem ser iniciador em setup
int pulse_pin[n_saidas_pulso] = {-1,-1}; 
int correction_drift[n_saidas_pulso] = {0,0}; 

bool diagnostic_mode = false;

unsigned long last_millis;
unsigned long totalMileage, tripA;

struct inputFreq{
  unsigned long ontime;
  unsigned long offtime;
  unsigned long period;
  float freq;
};
typedef struct inputFreq inputFreq;

float drift, duty, odometer = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(injector_pin,INPUT);
  pinMode(consume_pin,OUTPUT);
  pinMode(speed_out_pin,OUTPUT);
  pinMode(speed_in_pin,INPUT);
  pinMode(voltageIn, INPUT);
  pinMode(relayOut, OUTPUT);
  pinMode(beep, OUTPUT);
  
  pulse_pin[0]=consume_pin;
  pulse_pin[1]=speed_out_pin;
  
  if (EEPROM.read(0) == 255){
    //First boot, clear memory
    EEPROM.put(0,(int)0);
    EEPROM.put(2,(int)0);
    EEPROM.put(4,(long)0);
 
  } 
  //Load values
  EEPROM.get(0,correction_drift[0]);
  EEPROM.get(2,correction_drift[1]);
  EEPROM.get(4,totalMileage);
  
  setupTimer1();
}

void loop()
{
  if (Serial.available() > 0)
    Menu();

  float volts = analogRead(voltageIn)*0.0197f;
  
  inputFreq injectorInput, speedInput;
  readFrequency(injector_pin, &injectorInput);
  duty = (float)injectorInput.offtime/(float)(injectorInput.period); 
  float vazao = injetor * (0.126);// * (normalizeVoltage(volts)/12); // correction by voltage, result in ml/s 
  float consumption = injectorInput.freq*vazao*duty;
   
  // 0.083 Renault empirical constant
  if (injectorInput.period == 0.0)
    setOutFrequency(0.0,0);  
  else
    setOutFrequency(consumption/0.083f,0); 
  
  readFrequency(speed_in_pin, &speedInput);
  if (speedInput.period == 0.0)
    setOutFrequency(0.0,1);  
  else
    setOutFrequency(speedInput.freq,1);
   
  if (diagnostic_mode){
    clearScreen();
    diagnosticReport(injectorInput, speedInput, consumption, volts);
    delay(150);
  }

  //Calculate distance
  unsigned long elapsedtime = millis() - last_millis;
  odometer += (elapsedtime*out_freq[1])/4860.0f; //meters
  if (odometer > 500.0){
    tripA += odometer;
    totalMileage += odometer;
    odometer = 0.0;
    EEPROM.put(4, totalMileage);
  }
  last_millis = millis();

 /* if ((injectorInput.freq*60*1) > 6000){
    digitalWrite(beep,HIGH);
  }else{
    digitalWrite(beep,LOW);
  }*/

  delay(100);
}

void setupTimer1(){
  TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
  TCNT1 = 65536-(16000000/1024/timer_freq); //configura timer
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
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

float normalizeVoltage(float input){
  if (input < 12)
    return 12.0;
  else
    if (input > 15)
      return 15.0;
    else
      return input;
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
