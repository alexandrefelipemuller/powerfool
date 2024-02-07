/* 
* Memory is just 200 bytes, so we have to safe it
* instead of storing objects, we have to store just the essential
* <memory position = value, meaning>
* 0 = correction drift 0, consume (2 bytes)
* 2 = correction drift 1, speed (2 bytes)
* 4 = Total odometer (4 bytes)
* 8 = Shift Beep (2 bytes)
* 10 = rpm alert (2 bytes)
* 12 = press sensor (2 bytes)
* 14 = pulses per km, speed sensor (2 bytes)
*   Old VW 1800, Renault 4860, VW Fox 5000, chevrolet 15200
* 16 = speed limit (1 byte)
* 17 = lock doors speed (1 byte)
* 18 = Settings binary array (2 bytes)
*     0 - speed beep type
*     1 - injection sequential/semi
*     2 - Test Mode
*     
* 20 = fuel tank (2 bytes)
* 22 = oil pressure waring relay port (1 byte)
* 23 = Door lock relay port (1 byte)
* 24 = Hazard lights relay port (1 byte)
* 25 = speed input port (1 byte)
* 26 = injector input port (1 byte)
* 27 = break light input port (1 byte)
* 28 = menu display position
*/


void loadMemoryValues(){
  if (EEPROM.read(0) == 255){
    //First boot, clear memory, those are default values
    setDefaultMemoryValues();
  } 
  //Load values
  EEPROM.get(0,correction_drift[0]);
  EEPROM.get(2,correction_drift[1]);
  EEPROM.get(4,totalMileage);
  EEPROM.get(8,rpmLimit);
  EEPROM.get(10,rpmAlert);
  EEPROM.get(12,minPressure);
  EEPROM.get(14,speedSensor);
  EEPROM.get(16,speedLimit);
  EEPROM.get(18,settings);
  EEPROM.get(17,doorLockspd);
  EEPROM.get(20,tank);
  EEPROM.get(22,RL_P);
  EEPROM.get(23,RL_DL);
  EEPROM.get(24,RL_HZ);
  EEPROM.get(25,speed_in_pin);
  EEPROM.get(26,injector_pin);
  EEPROM.get(27,breakLight);
  EEPROM.get(28,speed_out_pin);
  EEPROM.get(29,consume_pin);
}

void setDefaultMemoryValues(){
    EEPROM.put(0,(int)0);
    EEPROM.put(2,(int)0);
    EEPROM.put(4,(long)0);
    EEPROM.put(8,(int)0);
    EEPROM.put(10,(int)0);
    EEPROM.put(12,(int)0);
    EEPROM.put(14,(int)4860);
    EEPROM.put(16,(char)0);
    EEPROM.put(17,(char)0);
    EEPROM.put(18,(int)0);
    EEPROM.put(20,(int)20000);
    EEPROM.put(22,(char)RL1);
    EEPROM.put(23,(char)RL2);
    EEPROM.put(24,(char)RL3);
    EEPROM.put(25,(char)DI2);
    EEPROM.put(26,(char)DI1);
    EEPROM.put(27,(char)DI3);
    EEPROM.put(28,(char)DO1);
    EEPROM.put(29,(char)DO2);
}
