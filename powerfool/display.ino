
bool menuButtonPressed = false;
bool setButtonPressed = false;

void releaseSetButton(){
  setButtonPressed = false;
}
void releaseMenuButton(){
  menuButtonPressed = false;
}

void changeMenu() {
  if (!menuButtonPressed)
  {
    currentMenu++;
    menuButtonPressed = true;
  }
}



void setMenu() {
  if (!setButtonPressed)
  {
    setButtonPressed = true;
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
      case 5:
        if (tank > 60000)
          tank=0;
        tank+=5000;
      case 6:
        is2step = true;
        break;
      default:
        break;
    }
  }
}

void menu0(){
    lcd.print(totalMileage/1000);
    lcd.print(F(" km"));
    printSpace();
    lcd.setCursor(0,1);
    lcd.print(tripA);
    lcd.print(F(" m"));
    printSpace();
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
    printSpace();
    lcd.setCursor(0,1);
    if (consumption == 0.0f)
      lcd.print(0.0f);
    else
      lcd.print((out_freq/((float) (speedSensor/1000.0f)))/consumption);
    lcd.print(F(" km/l"));
    printSpace();
}

void menu3(){
  lcd.print(("Temp.:"));
  char temp1 = 106-(analogRead(sensorTemp)/7.7f);
  lcd.print(temp1);
  lcd.write(0);
  lcd.print(("C "));
  printSpace();
  lcd.setCursor(0,1);
  temp1 = 106-(analogRead(intakeAirTemp)/7.7f);
  lcd.print(("Temp. Ar:"));
  lcd.print(temp1);
  lcd.write(0);
  lcd.print(("C "));
  printSpace();
}

void menu4(){
    lcd.setCursor(0,0);
    unsigned long currentLap = millis()-startLap;
    printTime(currentLap);
    printSpace();
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

  void menu5(){
    lcd.print((int)(tank/1000));
    lcd.print(".");
    lcd.print((int)(tank%100));
    lcd.print(" l");
    printSpace();
    lcd.setCursor(0,1);
    lcd.print("aut. ");
    lcd.print((float)(tank*(tripA/tank)));
    lcd.print(F(" km"));
    printSpace();
  }

  void menu6(int rpm){
    lcd.print(rpm);
    padNumber(rpm);
    lcd.print(F(" rpm"));
    printSpace();
    lcd.setCursor(0,1);
    if (setButtonPressed)
      lcd.print("Hold...");
    else{
      lcd.print("Go!");
      is2step = false;
    }
    printSpace();
  }

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
    case 5:
      menu5();
      break;
    case 6:
      menu6(rpm);
      break;
    default:
      currentMenu = 0;
      menu0();
  }
}

void printSpace(){
 lcd.print(F("             "));
}