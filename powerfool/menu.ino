#include <EEPROM.h>
#include <avr/pgmspace.h>
/* 
* Memory is just 200 bytes, so we have to safe it
* instead of storing objects, we have to store just the essential
* <memory position = value, meaning>
* 0 = correction drift 0, consume
* 1 = correction drift 1, speed
*/

void Menu() {
  if (diagnostic_mode)
    return;
  Serial.println(F("|****************************|"));
  Serial.println(F("|**| Powerfool Setup Menu |**|"));
  Serial.println(F("|****************************|"));
  Serial.println("");
  Serial.println(F("Select one of the following options:"));
  Serial.println(F("1 Ajuste Velocidade"));
  Serial.println(F("2 Ajuste Computador de bordo"));
  Serial.println(F("3 Funcoes especiais"));
  Serial.println(F("4 Modo diagnostico"));
  Serial.println(F("5 Sair do menu"));
  Serial.print(F("* Version:"));
  Serial.println(SW_VERSION,DEC);
 
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_num(1); break;
            case '2': subMenu_num(0); break;
            case '3': Serial.println("nao implementado"); break;
            case '4': diagnostic_mode=true; break;
            case '5': Serial.end(); return;
            default: continue;  // includes the case 'no input'
        }
      break;
    }
}
void subMenu_num(int position){
  Serial.println("");
  Serial.print(F("*** Valor em memoria: "));
  signed char value = EEPROM.read(position);
  Serial.print(memValueToCorrection(value));
  Serial.println("%");
  Serial.println(F("Valor entre -99 (%) e +400 (%), sendo 0 sem correcao"));
  Serial.println(F("+ Aumentar 1"));
  Serial.println(F("- Diminuir 1"));
  Serial.println(F("a Aumentar 10"));
  Serial.println(F("d Diminuir 10"));
  Serial.println(F("s Sair do menu"));
    for (;;) {
        switch (Serial.read()) {
            case '+':
              if (value > 126)
                value = 126;
          		value+=1;
              numberEntry(value,position);
  		        break;
            case '-':
                if (value <= -126)
                   value = -125;
                value-=1;
              numberEntry(value,position);
          		break;
            case 'a': 
                if (value > 117)
                  value = 116;
                value+=10;
          	    numberEntry(value,position);
       			break;
            case 'd': 
                if (value <= -117)
                   value = -116;
                value-=10;
          		numberEntry(value,position);
       			break;
          break;
            case 's': 
                Serial.println("Salvando valor...");
                EEPROM.write(position, value);
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
    return float (value*3.9f);
  else
    return float (value/1.27f);
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
