#include <EEPROM.h>

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
  Serial.println("|****************************|");
  Serial.println("|**|       Setup Menu     |**|");
  Serial.println("|****************************|");
  Serial.println("");
  Serial.println("Select one of the following options:");
  Serial.println("1 Ajuste Velocidade");
  Serial.println("2 Ajuste Computador de bordo");
  Serial.println("3 Funcoes especiais");
  Serial.println("4 Modo diagnostico");
  Serial.println("5 Sair do menu");
  Serial.print("* Version:");
  Serial.println(SW_VERSION,DEC);
 
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_num(1); break;
            case '2': subMenu_num(0); break;
            case '3': Serial.println("nao implementado"); break;
            case '4': diagnostic_mode=1; break;
            case '5': Serial.end(); return;
            default: continue;  // includes the case 'no input'
        }
      break;
    }
}
void subMenu_num(int position){
  Serial.println("");
  Serial.print("*** Valor em memoria: ");
  signed char value = EEPROM.read(position);
  Serial.println(value);
  Serial.println("Valor entre -100 (%) e +100 (%), sendo 0 sem correcao");
  Serial.println("+ Aumentar 1");
  Serial.println("- Diminuir 1");
  Serial.println("a Aumentar 10");
  Serial.println("d Diminuir 10");
  Serial.println("s Sair do menu");
    for (;;) {
        switch (Serial.read()) {
            case '+':
  		value+=1;
  		numberEntry(value);
  		break;
            case '-':
                if (value <= -99)
                   value = -98;
                value-=1;
             	numberEntry(value);
          		break;
            case 'a': 
                value+=10;
          		numberEntry(value);
       			break;
            case 'd': 
                if (value <= -89)
                   value = -89;
                value-=10;
          		numberEntry(value);
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

void numberEntry(int value){
  Serial.print("Novo valor: ");
  Serial.print(value);
  Serial.println(", opcoes +, -, x, d ou s para sair e salvar ");
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
