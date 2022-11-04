unsigned char currentMenu = 0;
unsigned char lap = 0;
unsigned long lastLap, startLap=0;
unsigned long bestLap=-1;
void changeMenu();
void setMenu();
void refreshMenu(inputFreq injectorInput, inputFreq speedInput, float consumption, float volts, int sensorPressureVal);
