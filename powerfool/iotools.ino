
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
  //out_freq[num]=(out_freq[num]+(baseFreq*drift))/2; //Use median to Smooth the signal
}

void pulsePinOnce(unsigned char pin, unsigned long ms){
  digitalWrite(pin,HIGH);
  delay(ms);
  digitalWrite(pin,LOW);
  delay(ms);
}

void timerLoop(){
 unsigned long currentTime = micros();
 for (int i=0;i < n_saidas_pulso; i++){
    if ((out_freq[i] > 2.0) &&
        (currentTime - previousTime[i]) >= (unsigned long)(1000000.0f/(out_freq[i]*2))) { //each pulse is high and low, so freq is double
      previousTime[i] = currentTime;
      digitalWrite(pulse_pin[i],!digitalRead(pulse_pin[i]));
    }
 }
}
