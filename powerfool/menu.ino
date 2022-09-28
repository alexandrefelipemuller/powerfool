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
* 10 = rpm alerta (2 bytes)
* 12 = pressao sensor (2 bytes)
* 14 = pulses per km, speed sensor (2 bytes)
*   Renault 4860, chevrolet 15200
*/
void(* resetFunc) (void) = 0;

void printBannerMsg(char *message){
  Serial.println(F("|****************************|"));
  Serial.print(F("|**| "));
  Serial.print(message); 
  for (int i=strlen(message)-1; i < 20; i++)
    Serial.print(" ");
  Serial.println(F("|**|"));
  Serial.println(F("|****************************|"));
}

void Menu() {
  if (diagnostic_mode)
    return;
  printBannerMsg(F("Dashlane  Setup Menu"));
  Serial.println("");
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

typedef enum {
    UCHAR,
    UINT,
    CHAR,
    INT
} varType;

/* Prompt user and store a numerical parameter */  
void subMenu_num(int position, bool isPercent, varType t){
  unsigned char step;
  switch(t){
      case UCHAR:
          unsigned char value;
          step=1;
          break;
      case INT:
          step=50;
          int value;
          break;
    default:
  }
  EEPROM.get(position,value);
  Serial.println("");
  Serial.print(F("*** Valor em memoria: "));
  if (isPercent){
      Serial.print(memValueToCorrection(value));
      Serial.println("%");
      Serial.println(F("Valor entre -99 (%) e +400 (%), sendo 0 sem correcao"));
  } else {
      Serial.println(value);
      Serial.println(F("Valor entre 0 e 32 mil, sendo 0 desabilitado"));
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
              break;
            case '+':
              if (value > 32717)
                value = 32717;
          		value+=step;
  		        break;
            case '-':
                if (value <= -32718)
                   value = -32718;
                value-=step;
          		break;
            case 'a': 
                if (value > 31768)
                  value = 31767;
                value+=(step*10);
       			break;
            case 'd': 
                if (value <= -31768)
                   value = -31767;
                value-=step(*10);
       			break;
          break;
            case 's': 
                Serial.println("Salvando valor...");
                if (isPercent){
                  Serial.println(memValueToCorrection(value));
                  correction_drift[position/2] = value;
                }else{
                  Serial.println(value);
                }
                EEPROM.put(position, (int) value);
                return;
            default: break;  // includes the case 'no input'
        }
       numberEntry(value,position,isPercent);
    }
}

void numberEntry(int value, int position, bool isPercent){
  Serial.print(F("Novo valor: "));
  if (isPercent){
     Serial.print(memValueToCorrection(value));
     correction_drift[position/2] = value;
     Serial.println(F("% de correcao, opcoes +, -, a, d ou s para sair e salvar"));
  }
  else{
     Serial.print(value);
     Serial.println(F(", opcoes +, -, a, d ou s para sair e salvar"));  
  }
}

float memValueToCorrection(int value){
  if (value > 0)  
    return float (value/81.9175f);
  else
    return float (value/327.67f);
}
void diagnosticReport(inputFreq injectorInput, inputFreq speedInput, float consumption, float volts, int sensorPressureVal){
    Serial.print(F("Entrada velocidade: "));
    Serial.println(speedInput.freq);
    Serial.print(F("Saida velocidade: "));
    Serial.println(out_freq[1]);
    
    Serial.print(F("Distancia total (km): "));
    Serial.println(totalMileage/1000);
    Serial.print(F("Odometro trip A (m): "));
    Serial.println(tripA);
 
    Serial.print(F("Velocidade: "));
    Serial.print(out_freq[1]/((float) (speedSensor/3600.0f)));
    Serial.println(F(" km/h"));
    Serial.print(F("Entrada injetores: "));
    Serial.print(injectorInput.freq);
    Serial.print(F("Hz, (%)"));
    Serial.println((float)injectorInput.offtime/(float)(injectorInput.period));  
    Serial.print(F("Saida consumo: "));
    Serial.println(out_freq[0]);
    Serial.print(F("Consumo: "));
    Serial.print(consumption);
    Serial.println(F(" ml/s"));
    Serial.print(F("Consumo Instantaneo: "));
    Serial.print((out_freq[1]/((float) (speedSensor/1000.0f)))/consumption);
    Serial.println(F(" km/l"));
    Serial.print(F("Tensao bateria: "));
    Serial.print(volts);
    Serial.println(F(" v"));
    Serial.print(F("RPM: "));
    Serial.println((int)(injectorInput.freq * (60/1) )); // 1 semi, 2 sequential
    Serial.print(F("Pressao sensor: "));
    Serial.println((int)sensorPressureVal);
    Serial.println(F("Pressione ESC para sair"));
    if (Serial.read() == 27)
      diagnostic_mode = false;
}
void subMenu_e(){
    printBannerMsg(F("Special features"));
    Serial.println("");
    Serial.println(F("Select one of the following options:"));
    Serial.println(F("1 Shift Beep"));
    Serial.println(F("2 Rotacao minima sensor pressao"));
    Serial.println(F("3 Pressao minima sensor (1000 = 1 bar)"));
    Serial.println(F("4 Aviso velocidade excedida em km/h"));
    Serial.println(F("5 Exit"));
    varType type;
    for (;;) {
        switch (Serial.read()) {
            case '1': type = varType.INT; subMenu_num(8,false,type); break;
            case '2': type = varType.INT; subMenu_num(10,false,type); break;
            case '3': type = varType.INT; subMenu_num(12,false,type); break;
            case '4': type = varType.UCHAR; subMenu_num(14,false,type); break;
            case '5': return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_e();
        break;
    }
}
void subMenu_a(){
    printBannerMsg(F("Ajustes"));
    Serial.println("");
    Serial.println(F("Select one of the following options:"));
    Serial.println(F("1 Leitura sensor de velocidade"));
    Serial.println(F("2 Saida sensor de velocidade"));
    Serial.println(F("3 Saida sinal de consumo"));
    Serial.println(F("4 Exit"));
    varType type;
    for (;;) {
        switch (Serial.read()) {
            case '1':  type = varType.INT; subMenu_num(14,false,type); break;
            case '2':  type = varType.INT; subMenu_num(2,true,type); break;
            case '3':  type = varType.INT; subMenu_num(0,true,type); break;
            case '4': return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_e();
        break;
    }
}

void clearScreen(){
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
}
