void changeMenu() {
  currentMenu++;
  lcd.clear();
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

void menu2(float out_freq){
    unsigned char cspeed = (int) (out_freq/((float) (speedSensor/3600.0f))); 
    lcd.print(cspeed);
    padNumber(cspeed);
    lcd.print(F("km/h"));
}

void menu3(float out_freq, float consumption){
    if (consumption == 0.0f)
      lcd.print(0.0f);
    else
      lcd.print((out_freq/((float) (speedSensor/1000.0f)))/consumption);
    lcd.print(F(" km/l"));
    lcd.setCursor(0,1);
    lcd.print(consumption);
    lcd.print(F(" ml/s"));

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
     menu2(speedInput.freq);
      break;
    case 3:
     menu3(speedInput.freq, consumption);
      break;
    default:
      currentMenu = 0;
      menu0();
  }
}
