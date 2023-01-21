struct inputFreq{
  unsigned long ontime;
  unsigned long offtime;
  unsigned long period;
  float freq;
};
typedef struct inputFreq inputFreq;
void timerLoop();
void pulsePinOnce(unsigned char pin, unsigned long ms);
void setOutFrequency(float baseFreq, int num);
void readFrequency(int pin, char samples, inputFreq *returnedValues);
