
void changeMenu() {
  currentMenu++;
  lcd.clear();
}
void menu1(int rpm, float volts, int sensorPressure){
    lcd.setCursor(0,0);
    lcd.print(rpm);
    lcd.print(F(" rpm, "));
    lcd.print(volts);
    lcd.print(F("v "));
    lcd.setCursor(0,1);
    lcd.print(F("Pressao: "));
    lcd.print(float(sensorPressure/1000.0f));
    lcd.print(F("bar"));
}

void menu0(){
    lcd.setCursor(0,0);
    lcd.print(F("Welcome"));
}

void menu2(float consumption){
    lcd.setCursor(0,0);
    lcd.print(consumption);
    lcd.print(F(" ml/s"));
}

void refreshMenu(int rpm, float volts, int sensorPressure, float consumption) {
  lcd.setCursor(0,0);
  switch (currentMenu)
  {
    case 1:
      menu1(rpm, volts, sensorPressure);
      break;
    case 2:
      menu2(consumption);
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    default:
      currentMenu = 0;
      menu0();
  }
}
