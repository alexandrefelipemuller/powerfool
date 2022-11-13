void changeMenu() {
  currentMenu++;
  lcd.clear();
}

void setMenu() {
  switch (currentMenu)
  {
    case 0:
      tripA=0;
      break;
    case 4:
      lap++;
      lastLap=millis()-startLap;
      if (lastLap < bestLap)
        bestLap=lastLap;
      startLap=millis();
      break;
    #ifndef is_ATM168p
    case 5:
      if (tank > 60000)
        tank=0;
      tank+=5000;
    #endif
    default:
      break;
  }
}

void menu0(){
    lcd.print(totalMileage/1000);
    lcd.print(F(" km"));
    lcd.setCursor(0,1);
    lcd.print(tripA);
    lcd.print(F(" m"));
}

void menu1(int rpm, float volts, int sensorPressure){
    lcd.print(rpm);
    padNumber(rpm);
    lcd.print(F(" rpm, "));
    lcd.print(volts);
    lcd.print(F("v "));
    lcd.setCursor(0,1);
    lcd.print(F("Pressao: "));
    lcd.print(float(sensorPressure/1000.0f));
    lcd.print(F("bar"));
}

void menu2(float out_freq, float consumption){
    unsigned char cspeed = (int) (out_freq/((float) (speedSensor/3600.0f))); 
    lcd.print(cspeed);
    padNumber(cspeed);
    lcd.print(F("km/h"));
    lcd.setCursor(0,1);
    if (consumption == 0.0f)
      lcd.print(0.0f);
    else
      lcd.print((out_freq/((float) (speedSensor/1000.0f)))/consumption);
    lcd.print(F(" km/l"));
}

void menu3(){
  lcd.print(("sensor1:"));
  lcd.print((float)(analogRead(A7)/204));
  lcd.print((" v"));
  lcd.setCursor(0,1);
  lcd.print(("sensor2:"));
  lcd.print((float)(analogRead(A6)/204));
  lcd.print((" v"));
}

void menu4(){
    lcd.setCursor(0,0);
    unsigned long currentLap = millis()-startLap;
    printTime(currentLap);
    lcd.setCursor(10,0);
    if (bestLap < (unsigned long) -1)
      printTime(bestLap);
    lcd.setCursor(0,1);
    lcd.print(F("LAP "));
    lcd.print(lap);
    lcd.print(" ");
    printTime(lastLap);
    lcd.print(" ");
}

#ifndef is_ATM168p
  void menu5(){
    lcd.print((int)(tank/1000));
    lcd.print(".");
    lcd.print((int)(tank%100));
    lcd.print(" l");
    lcd.setCursor(0,1);
    lcd.print("aut. ");
    lcd.print((float)(tank*(tripA/tank)));
    lcd.print(F(" km"));
  }
#endif

void printTime(unsigned long lTime){
    lcd.print((unsigned int)(lTime/60000));
    lcd.print(":");
    lcd.print((unsigned int)((lTime/1000)%60));
    lcd.print(".");
    lcd.print((unsigned int)(lTime%100));

}

void padNumber(unsigned char number){
 if (number < 10)
    lcd.print("   ");
 else if (number < 100){
    lcd.print("  ");
 }
 else if (number < 1000){
    lcd.print(" ");
 }
}

void refreshMenu(inputFreq injectorInput, inputFreq speedInput, float consumption, float volts, int sensorPressureVal) {
  int rpm = injectorInput.freq*60*((settings & 2 == 0)*2);
  lcd.setCursor(0,0);
  switch (currentMenu)
  {
    case 1:
      menu1(rpm, volts, sensorPressure);
      break;
    case 2:
     menu2(speedInput.freq, consumption);
      break;
    case 3:
     menu3();
      break;
    case 4:
     menu4();
      break;
     #ifndef is_ATM168p
      case 5:
      menu5();
      break;
     #endif
    default:
      currentMenu = 0;
      menu0();
  }
}
