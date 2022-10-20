#include <EEPROM.h>
#include <avr/pgmspace.h>
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
* 21 - IAT sensor (2 bytes)
* 23 - Lambda sensor (2 bytes)
* 25 - Wastegate adjust (2 bytes)
*/
void(* resetFunc) (void) = 0;

void printBannerMsg(const char *message){
  Serial.println(F("|****************************|"));
  Serial.print(F("|**| "));
  Serial.print(message); 
  for (int i=strlen(message)-1; i < 20; i++)
    Serial.print(" ");
  Serial.println(F("|**|"));
  Serial.println(F("|****************************|"));
  Serial.println("");
}

void Menu() {
  if (diagnostic_mode)
    return;
  printBannerMsg("Dashlane  Setup Menu");
  Serial.println(F("Select one of the following options:"));
  Serial.println(F("1 Ajustes"));
  Serial.println(F("2 Funcoes especiais"));
  Serial.println(F("3 Modo diagnostico"));
  Serial.println(F("4 Sair do menu"));
  Serial.println(F("5 Reboot"));
  Serial.print(F("* Version:"));
  Serial.println(SW_VERSION,DEC);
 
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_a(); break;
            case '2': subMenu_e(); break;
            case '3': diagnostic_mode=true; break;
            case '4': Serial.println(F("bye...")); Serial.end();
            case '5': resetFunc();
            default: continue;  // includes the case 'no input'
        }
      Menu();
      break;
    }
}


/* Prompt user and store a numerical parameter */  
void subMenu_num(int position, bool isPercent, varType t){
  unsigned char step;
  int value;
  EEPROM.get(position,value);
  Serial.println("");
  Serial.print(F("*** Valor em memoria: "));
  static int upperLimit, lowerLimit;
  switch(t){
      case UCHAR:
          step=1;
          upperLimit=256;
          lowerLimit=0;
          break;
      case INT:
          step=50;
          upperLimit=32718;
          lowerLimit=upperLimit*-1;
          break;
    default:
    break;
  }
  if (isPercent){
      Serial.print(memValueToCorrection(value));
      Serial.println("%");
      Serial.println(F("Valor entre -99 (%) e +400 (%), sendo 0 sem correcao"));
  } else {
      Serial.println(value);
      if (t == INT)
        Serial.println(F("Valor entre 0 e 32 mil, sendo 0 desabilitado"));
      else
        Serial.println(F("Valor entre 0 e 250, sendo 0 desabilitado"));
  }
  Serial.println(F("+ Aumentar"));
  Serial.println(F("- Diminuir"));
  Serial.println(F("a Aumentar 10x"));
  Serial.println(F("d Diminuir 10x"));
  Serial.println(F("0 deixar sem correcao"));
  Serial.println(F("s Sair do menu"));
    for (;;) {
        switch (Serial.read()) {
            case '0':
              value=0;
              numberEntry(value,position,isPercent);
              break;
            case '+':
              if (value >= upperLimit)
                value = upperLimit;
              else
          		  value+=step;
              numberEntry(value,position,isPercent);
  		        break;
            case '-':
                if (value <= lowerLimit)
                   value = lowerLimit;
                else
                  value-=step;
              	numberEntry(value,position,isPercent);
              	break;
            case 'a': 
                if (value >= upperLimit-(step*10))
                  value = upperLimit-(step*10);
                else
                  value+=(step*10);
         			numberEntry(value,position,isPercent);
         			break;
            case 'd': 
                if (value <= lowerLimit+(step*10))
                  value = lowerLimit+(step*10);
                else
                  value-=(step*10);
         			numberEntry(value,position,isPercent);
         			break;
            case 's': 
                Serial.println("Salvando valor...");
                if (isPercent){
                  Serial.println(memValueToCorrection(value));
                  correction_drift[position/2] = value;
                }else{
                  Serial.println(value);
                }
                if (t == UCHAR)
                  EEPROM.put(position, (unsigned char) value);
                else
                  EEPROM.put(position, (int) value);
                return;
            default: continue;  // includes the case 'no input'
        }
    }
}

void settingsChange(unsigned char num){
  int bitv = (1 << num);
  settings ^= bitv;
  EEPROM.put(18, (int) settings);
}
void numberEntry(int value, int position, bool isPercent){
  Serial.print(F("Novo valor: "));
  if (isPercent){
     Serial.print(memValueToCorrection(value));
     correction_drift[position/2] = value;
     Serial.print(F("% de correcao"));
  }
  else{
     Serial.print(value);  
  }
  Serial.println(F(", opcoes +, -, a, d ou s para sair e salvar"));
}

float memValueToCorrection(int value){
  if (value > 0)  
    return float (value/81.9175f);
  else
    return float (value/327.67f);
}
void diagnosticReport(inputFreq injectorInput, inputFreq speedInput, float volts, int sensorPressureVal){
    Serial.print(F("Entrada velocidade: "));
    Serial.println(speedInput.freq);
    Serial.print(F("Saida velocidade: "));
    Serial.println(out_freq[1]);
    
    Serial.print(F("Distancia total (km): "));
    Serial.println(totalMileage/1000);
 
    Serial.print(F("Velocidade: "));
    Serial.print(out_freq[1]/((float) (speedSensor/3600.0f)));
    Serial.println(F(" km/h"));
    Serial.print(F("Entrada injetores: "));
    Serial.print(injectorInput.freq);
    Serial.print("Hz, (%)");
    float duty = (float)injectorInput.offtime/(float)(injectorInput.period);
    Serial.print(F("Tensao bateria: "));
    Serial.print(volts);
    Serial.println(F(" v"));
    Serial.print(F("RPM: "));
    Serial.println((int)(injectorInput.freq * 60 *((settings & 2 == 0)*2) )); // semi, sequential
    Serial.print(F("Pressao sensor: "));
    Serial.println((int)sensorPressureVal);
    Serial.println(F("Pressione ESC para sair"));
    if (Serial.read() == 27)
      diagnostic_mode = false;
}
void subMenu_e(){
    printBannerMsg("Special features");
    Serial.println(F("Select one of the following options:"));
    Serial.println(F("1 Shift Beep"));
    Serial.println(F("2 Rotacao minima sensor pressao"));
    Serial.println(F("3 Pressao minima sensor (1000 = 1 bar)"));
    Serial.println(F("4 Aviso velocidade excedida em km/h"));
    Serial.print(F("5 Mudar tipo aviso velocidade excedida, beep:"));    
    if (settings%2 == 0)
      Serial.println(F(" (continuo)")); 
    else
      Serial.println(F(" (curto)")); 
    Serial.println(F("6 Travamento automatico de portas km/h"));
    Serial.println(F("ESC Voltar"));
    varType typev = INT;
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_num(8,false,typev); break;
            case '2': subMenu_num(10,false,typev); break;
            case '3': subMenu_num(12,false,typev); break;
            case '4': typev = UCHAR; subMenu_num(16,false,typev); break;
            case '5': settingsChange(0); break;
            case '6': typev = UCHAR; subMenu_num(17,false,typev); break;
            case 27: return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_e();
        break;
    }
}
void subMenu_a(){
    printBannerMsg("Ajustes");
    Serial.println(F("Select one of the following options:"));
    Serial.println(F("1 Leitura sensor de velocidade"));
    Serial.println(F("2 Saida sensor de velocidade"));
    Serial.println(F("3 Saida sensor de temperatura do ar"));
    Serial.println(F("4 Saida sensor de sonda lambda"));
    Serial.println(F("5 Saida sensor wastegate"));
        Serial.print(F("6 Mudar leitura tipo injeção:"));    
    if (settings & 2 == 0)
      Serial.println(F(" (semisequencial)")); 
    else
      Serial.println(F(" (sequencial)")); 
    Serial.println(F("ESC Voltar"));
    varType typev = INT;
    for (;;) {
        switch (Serial.read()) {
            case '1':  subMenu_num(14,false,typev); break;
            case '2':  subMenu_num(2,true,typev); break;
            case '3':  subMenu_num(21,true,typev); break;
            case '4':  subMenu_num(23,true,typev); break;
            case '5':  subMenu_num(25,true,typev); break;
            case '6':  settingsChange(1); break;
            case 27: return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_a();
        break;
    }
}

void clearScreen(){
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
}
