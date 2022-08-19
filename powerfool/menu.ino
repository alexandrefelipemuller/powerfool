#include <EEPROM.h>
#include <avr/pgmspace.h>
/* 
* Memory is just 200 bytes, so we have to safe it
* instead of storing objects, we have to store just the essential
* <memory position = value, meaning>
* 0 = correction drift 0, consume (2 bytes)
* 2 = correction drift 1, speed (2 bytes)
* 4 = Total odometer (4 bytes)
*/
void(* resetFunc) (void) = 0;

void Menu() {
  if (diagnostic_mode)
    return;
  Serial.println(F("|****************************|"));
  Serial.println(F("|**| Dashlane  Setup Menu |**|"));
  Serial.println(F("|****************************|"));
  Serial.println("");
  Serial.println(F("Select one of the following options:"));
  Serial.println(F("1 Ajuste Velocidade"));
  Serial.println(F("2 Ajuste Computador de bordo"));
  Serial.println(F("3 Funcoes especiais"));
  Serial.println(F("4 Modo diagnostico"));
  Serial.println(F("5 Sair do menu"));
  Serial.println(F("6 Reboot"));
  Serial.print(F("* Version:"));
  Serial.println(SW_VERSION,DEC);
 
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_num(1); break;
            case '2': subMenu_num(0); break;
            case '3': Serial.println("nao implementado"); break;
            case '4': diagnostic_mode=true; break;
            case '5': Serial.println(F("bye...")); Serial.end(); return;
            case '6': resetFunc();
            default: continue;  // includes the case 'no input'
        }
      break;
    }
}
void subMenu_num(int position){
  Serial.println("");
  Serial.print(F("*** Valor em memoria: "));
  int value;
  EEPROM.get(position,value);
  Serial.print(memValueToCorrection(value));
  Serial.println("%");
  Serial.println(F("Valor entre -99 (%) e +400 (%), sendo 0 sem correcao"));
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
              numberEntry(value,position);
              break;
            case '+':
              if (value > 32717)
                value = 32717;
          		value+=50;
              numberEntry(value,position);
  		        break;
            case '-':
                if (value <= -32718)
                   value = -32718;
                value-=50;
              numberEntry(value,position);
          		break;
            case 'a': 
                if (value > 31768)
                  value = 31767;
                value+=1000;
          	    numberEntry(value,position);
       			break;
            case 'd': 
                if (value <= -31768)
                   value = -31767;
                value-=1000;
          		numberEntry(value,position);
       			break;
          break;
            case 's': 
                Serial.println("Salvando valor...");
                EEPROM.put(position, (int) value);
                correction_drift[position] = value;
                return;
            default: break;  // includes the case 'no input'
        }
    }
}

void numberEntry(int value, int position){
  Serial.print(F("Novo valor: ")); 
  Serial.print(memValueToCorrection(value));
  correction_drift[position] = value;
  Serial.println(F("% de correcao, opcoes +, -, a, d ou s para sair e salvar"));
}

float memValueToCorrection(int value){
  if (value > 0)  
    return float (value/81.9175f);
  else
    return float (value/327.67f);
}
void diagnosticReport(inputFreq injectorInput, inputFreq speedInput, float consumption, float volts){
    Serial.print("Entrada velocidade: ");
    Serial.println(speedInput.freq);
    Serial.print("Saida velocidade: ");
    Serial.println(out_freq[1]);
    
    Serial.print("Distancia total (km): ");
    Serial.println(totalMileage/1000);
    Serial.print("Odometro trip A (m): ");
    Serial.println(tripA);
 
    Serial.print("Velocidade: ");
    Serial.print(out_freq[1]/1.35f); //Parametrize it later
    Serial.println(" km/h");
    Serial.print("Entrada injetores: ");
    Serial.print(injectorInput.freq);
    Serial.print("Hz, (%)");
    Serial.println(duty);
    Serial.print("Saida consumo: ");
    Serial.println(out_freq[0]);
    Serial.print("Consumo: ");
    Serial.print(consumption);
    Serial.println(" ml/s");
    Serial.print("Consumo Instantaneo: ");
    Serial.print((out_freq[1]/4.86f)/consumption);
    Serial.println(" km/l");
    Serial.print("Tensao bateria: ");
    Serial.print(volts);
    Serial.println(" v");
}
void subMenu_e(){
  /* Serial.println("|***|  Special features  |***|");
  Serial.println("");
  Serial.println("Select one of the following options:");
  Serial.println("1 ");
  Serial.println("2 ");
  Serial.println("3 ");
  Serial.println("4 ");
  Serial.println("5 Exit");
    for (;;) {
        switch (Serial.read()) {
            case '1': Serial.println("nao implementado"); break;
            case '2': Serial.println("nao implementado"); break;
            case '3': Serial.println("nao implementado"); break;
            case '4': Serial.println("nao implementado"); break;
            case '5': Serial.end(); return;
            default: continue;  // includes the case 'no input'
        }
      break;
    }*/
}

void clearScreen(){
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
}
