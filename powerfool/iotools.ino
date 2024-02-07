void readFrequency(int pin, char samples, inputFreq *returnedValues){
  (*returnedValues).offtime = 0.0; 
  (*returnedValues).ontime = 0.0; 
  (*returnedValues).period = 0.0;
  (*returnedValues).freq = 0.0;
  unsigned long timeout = 200000; // Initial timeout of 200 ms
  for (int i=0;i < samples; i++){
    unsigned long ontime = pulseInLong(pin,HIGH,timeout);
    if (ontime == 0){
      return;
    }
    (*returnedValues).ontime += ontime;
    unsigned long offtime = pulseInLong(pin,LOW,timeout);
    (*returnedValues).offtime += offtime;
    timeout = max(ontime, offtime)*3;
  }
  (*returnedValues).offtime = (*returnedValues).offtime/samples;
  (*returnedValues).ontime = (*returnedValues).ontime/samples;
  (*returnedValues).period = ((*returnedValues).ontime+(*returnedValues).offtime);
  (*returnedValues).freq = (1000000.0f/(*returnedValues).period);    
}
//Setup the frequency to output
void setOutFrequency(float baseFreq, int num){
  if (baseFreq > 500.0)
    baseFreq=500.0f;
  if (baseFreq < 2.0){
    //desabilita aquela saida
    digitalWrite(pulse_pin[num],HIGH);
    baseFreq=0.0;
  }
  float drift=0;
  if (correction_drift[num] > 0)
    drift=(float) (correction_drift[num]/8191.75f)+1.0f;
  else
    drift=(float) (correction_drift[num]+32767)/32767.0f;

  out_freq[num]=(baseFreq*drift);
  // Calcula o número total de interrupções necessárias para cada ciclo de PWM fora da ISR
  totalInterruptsForCycle[num] = (F_CPU / 1024 / out_freq[num] / 6);
}

void pulsePinOnce(unsigned char pin, unsigned long ms){
  digitalWrite(pin,HIGH);
  delay(ms);
  digitalWrite(pin,LOW);
  delay(ms);
}

void timerLoop(){
  for (int i = 0; i < n_saidas_pulso; i++) {
    if (out_freq[i] > 2.0){
      pulseCounters[i]++;
      if (pulseCounters[i] >= totalInterruptsForCycle[i]) {
        digitalWrite(pulse_pin[i], !digitalRead(pulse_pin[i]));
        pulseCounters[i] = 0; // Reseta o contador após completar um ciclo
      }
    }
  }
}
