#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "menu.h"
#define timer_freq 3000

#define injector_pin 12 // roxo
#define consume_pin 11 // branco
#define speed_in_pin 2 // verde
#define speed_out_pin 3 // vermelho
#define voltageIn A1 // interno

const float injetor = 23; // vazao do injetor a 12v, em lbs/h

// set up pulse pins
const int n_saidas_pulso = 2;
int out_freq[n_saidas_pulso] = {1,1};
unsigned long previousTime[n_saidas_pulso] = {0,0};
//Define pinos para pulsar, devem ser iniciador em setup
int pulse_pin[n_saidas_pulso] = {-1,-1}; 
int correction_drift[n_saidas_pulso] = {-1,-1}; 

int diagnostic_mode = 0;

//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 

const int pin_BL = 10; 

LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

unsigned long ontime, offtime, period;
float drift;
float freq,duty;
   
void setup()
{
  Serial.begin(9600);
  pinMode(injector_pin,INPUT);
  pinMode(consume_pin,OUTPUT);
  pinMode(speed_out_pin,OUTPUT);
  pinMode(speed_in_pin,INPUT);
  pinMode(voltageIn, INPUT);
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Freq:");
  lcd.setCursor(0,1);
  lcd.print("Outf:");  
  

  pulse_pin[0]=consume_pin;
  pulse_pin[1]=speed_out_pin;
  int drift0 = EEPROM.read(0);
  if (drift0 == 255){
    //First boot, clear memory
    EEPROM.write(0,0);
    EEPROM.write(1,0);
    drift0=0;
  } 
  correction_drift[0] = drift0;
  correction_drift[1] = EEPROM.read(1);
  
  // Configuração do timer1 
  TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
 
  TCNT1 = 65536-(16000000/1024/timer_freq); //configura timer
  
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1

}

void loop()
{
  if (Serial.available() > 0 || diagnostic_mode)
    Menu();
  ontime = pulseIn(injector_pin,HIGH,250000);
  offtime = pulseIn(injector_pin,LOW,250000); 
  period = ontime+offtime;
  freq = 1000000.0/period;
  duty = (float)offtime/(float)(period); 
  
  float volts = analogRead(voltageIn)*0.0197f;
  
  float vazao = injetor * (0.126);// * (normalizeVoltage(volts)/12); // correction by voltage, result in ml/s 
  
  float consumo = freq*vazao*duty;
   
  // 0.083 Renault empirical constant
  setOutFrequency(consumo/0.083f,0); 
  
  ontime = pulseIn(speed_in_pin,HIGH,200000);
  offtime = pulseIn(speed_in_pin,LOW,200000); 
  period = ontime+offtime;
  if (period == 0.0)
    setOutFrequency(0.0,1);  
  else
    setOutFrequency(1000000.0f/period,1);
     
  lcd.setCursor(6,0); 
  lcd.print((int) freq);
  lcd.print("Hz, ");
  lcd.setCursor(12,0);
  lcd.print((int) (duty*100)); 
  lcd.print("%"); 

  lcd.setCursor(6,1); 
  lcd.print((int) out_freq[0]);
  lcd.print("  ");
  
  if (diagnostic_mode){
    clearScreen();
    Serial.print("Entrada velocidade: ");
    Serial.println(1000000.0/period);
    Serial.print("Saida velocidade: ");
    Serial.println(out_freq[1]);
    Serial.print("Velocidade: ");
    Serial.print(out_freq[1]/1.35f); //Parametrize it later
    Serial.println(" km/h");
    Serial.print("Entrada injetores: ");
    Serial.print(freq);
    Serial.print("Hz, (%)");
    Serial.println(duty);
    Serial.print("Saida consumo: ");
    Serial.println(out_freq[0]);
    Serial.print("Consumo: ");
    Serial.print(consumo);
    Serial.println(" ml/s");
    Serial.print("Consumo Instantaneo: ");
    Serial.print((out_freq[1]/4.86f)/consumo);
    Serial.println(" km/l");
    
    Serial.print("Tensao bateria: ");
    Serial.print(volts);
    Serial.println(" v");
  }
  delay(100);
}

void setOutFrequency(float baseFreq, int num){
  if (baseFreq > 320)
    baseFreq=320.0f;
  if (baseFreq < 2){
    //desabilita aquela saida
    digitalWrite(pulse_pin[num],LOW);
    previousTime[num] = micros(); 
    baseFreq=1;
  }
  float drift=(correction_drift[num]+100)/100;
  out_freq[num]=baseFreq*drift;
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

ISR(TIMER1_OVF_vect)
{
 TCNT1 = 65536-(16000000/1024/timer_freq); // timer reset
 unsigned long currentTime = micros();
 for (int i=0;i < n_saidas_pulso; i++)
    if ((currentTime - previousTime[i]) >= (unsigned long)1000000.0f/out_freq[i]<<1) {  //cada pulso um high e um low, por isso mult por 2
        digitalWrite(pulse_pin[i],!digitalRead(pulse_pin[i]));
    	previousTime[i] = currentTime;
  }
}

