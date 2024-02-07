#ifndef is_ESP32
#include <avr/pgmspace.h>
#else
#define F(s) (s)
#endif
/* 
* See memutils diagram of the memory usage
*/

#ifdef is_ESP32
  void resetFunc(){}
#else
  void(* resetFunc) (void) = 0;
#endif

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
  Serial.println(F("5 Entradas e saidas"));
  if (getBit(2))
    Serial.println(F("6 Relatorio de teste"));
  Serial.println(F("7 Reset de fabrica"));
  Serial.println(F("T Modo Teste"));
  Serial.println(F("9 Reboot"));
  Serial.print(F("* Version:"));
  Serial.println(SW_VERSION,DEC);
 
    for (;;) {
        switch (Serial.read()) {
            case '1': subMenu_a(); break;
            case '2': subMenu_e(); break;
            case '3': diagnostic_mode=true; break;
            case '4': Serial.println(F("bye...")); Serial.end();
            case '5': subMenu_p(); break;
            case '7': resetFab(); break;
            case '9': 
                      resetFunc();
                      break;
            case 'T':
            case 't':
                      settingsChange(2);
                      break;
            case '6': if (getBit(2))
                        testReport();
                      break;

            default: continue;  // includes the case 'no input'
        }
      Menu();
      break;
    }
}
/* Prompt user and store a numerical parameter */
/* Params: memory position, min and max values */
void subMenu_range(unsigned char position, char min, char max){
  if (min > max){
    int temp = max;
    max=min;
    min = temp;
  }
  unsigned char value;
  EEPROM.get(position,value);
  Serial.println("");
  Serial.print(F("*** Valor em memoria: "));
  Serial.println(pinToName(value));
  Serial.println(F("+ Proximo"));
  Serial.println(F("- Anterior"));
  Serial.println(F("s Sair do menu"));
    for (;;) {
        switch (Serial.read()) {
            case '+':
              value++;
              if (value > max)
                value=min;
              Serial.print(F("*** Novo valor: "));
              Serial.println(pinToName(value));
  		        break;
            case '-':
              value--;
              if (value < min)
                value=max;
              Serial.print(F("*** Novo valor: "));
              Serial.println(pinToName(value));
              break;
            case 's': 
            case 'S': 
                Serial.println("Salvando valor...");
                Serial.println(value);
                EEPROM.put(position, (unsigned char) value);
                return;
            default: continue;  // includes the case 'no input'
        }
    }
}

/* Prompt user and store a numerical parameter */
/* Params: memory position, if is percentage, and data type */
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
            case 'A': 
                if (value >= upperLimit-(step*10))
                  value = upperLimit-(step*10);
                else
                  value+=(step*10);
         			numberEntry(value,position,isPercent);
         			break;
            case 'd':
            case 'D':  
                if (value <= lowerLimit+(step*10))
                  value = lowerLimit+(step*10);
                else
                  value-=(step*10);
         			numberEntry(value,position,isPercent);
         			break;
            case 's': 
            case 'S': 
                Serial.println("Salvando valor...");
                if (isPercent){
                  Serial.println(memValueToCorrection(value));
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

int getBit(int posicao) {
  // Verifica se a posição fornecida é válida (0 a 31 para um int de 32 bits)
  if (posicao >= 0 && posicao < sizeof(int) * 8) {
    int bitv = 1 << posicao; // Cria uma máscara para isolar o bit na posição especificada
    return (settings & bitv) ? 1 : 0; // Verifica o valor do bit e retorna 1 ou 0
  } else {
    // Trate o caso de posição inválida aqui, se necessário
    return -1; // Ou outro valor de erro
  }
}

void numberEntry(int value, int position, bool isPercent){
  Serial.print(F("Novo valor: "));
  if (isPercent){
     Serial.print(memValueToCorrection(value));
     Serial.print(F("% de correcao"));
  }
  else{
     Serial.print(value);  
  }
  Serial.println(F(", opcoes +, -, a, d ou s para sair e salvar"));
}

void resetFab(){
  Serial.print(F("Deseja apagar tudo? (s/n)"));
  for(;;){
    switch (Serial.read()){
    case 's':
    case 'S':
        setDefaultMemoryValues();
        Serial.println(F("Memoria apagada."));
        return;
    case 'n':
    case 'N':
        Serial.println(F("Cancelando..."));
        return;
    }
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
    Serial.print(F("Pressao sensor: "));
    Serial.println((int)sensorPressureVal);
    Serial.print(F("Consumo Instantaneo: "));
    Serial.print((out_freq[1]/((float) (speedSensor/1000.0f)))/consumption);
    Serial.println(F(" km/l"));
    Serial.print(F("Odometro trip A (m): "));
    Serial.println(tripA);
    Serial.print(F("Distancia total (km): "));
    Serial.println(totalMileage/1000);
    Serial.print(F("Velocidade: "));
    Serial.print(out_freq[1]/((float) (speedSensor/3600.0f)));
    Serial.println(F(" km/h"));
    Serial.print(F("Entrada injetores: "));
    Serial.print(injectorInput.freq);
    Serial.print("Hz, (%)");
    float duty = (float)injectorInput.offtime/(float)(injectorInput.period);
    Serial.println(duty);
    Serial.print("Saida consumo: ");
    Serial.println(out_freq[0]);
    Serial.print(F("Consumo: "));
    Serial.print(consumption);
    Serial.println(F(" ml/s"));
    Serial.print(F("Tensao bateria: "));
    Serial.print(volts);
    Serial.println(F(" v"));
    Serial.print(F("RPM: "));
    Serial.println((int)(injectorInput.freq*60*(getBit(1)*2))); // semi, sequential

    Serial.println(F("Pressione ESC para sair"));
    if (Serial.read() == 27)
      diagnostic_mode = false;
}
void subMenu_e(){
    clearScreen();
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
            //case '6': typev = UCHAR; subMenu_num(17,false,typev); break;
            case 27: return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_e();
        break;
    }
}
void subMenu_a(){
    clearScreen();
    printBannerMsg("Ajustes");
    Serial.println(F("Select one of the following options:"));
    Serial.println(F("1 Leitura sensor de velocidade"));
    Serial.println(F("2 Saida sinal de velocidade"));
    Serial.println(F("3 Saida sinal de consumo"));
    Serial.print(F("4 Mudar leitura tipo injeção:"));    
    if (getBit(1))
      Serial.println(F(" (sequencial)")); 
    else
      Serial.println(F(" (semisequencial)")); 
    Serial.println(F("ESC Voltar"));
    varType typev = INT;
    for (;;) {
        switch (Serial.read()) {
            case '1':  subMenu_num(14,false,typev); break;
            case '2':  subMenu_num(2,true,typev); break;
            case '3':  subMenu_num(0,true,typev); break;
            case '4':  settingsChange(1); break;
            case 27: return;
            default: continue;  // includes the case 'no input'
        }
        subMenu_a();
        break;
    }
}

char* pinToName(int pin){
  switch (pin){
    case RL1:
      return "RL1";
    case RL2:
      return "RL2";
    case RL3:
      return "RL3";
    case DI1:
      return "DI1";
    case DI2:
      return "DI2";
    case DI3:
      return "DI3";
    default:
    return "";
  }
}

void subMenu_p(){
    clearScreen();
    loadMemoryValues();
    printBannerMsg("Entradas e saidas");
    Serial.println(F("Select one of the following options:"));
    Serial.print(F("1 Saida alerta Pressao:"));
    Serial.println(pinToName(RL_P));
    Serial.print(F("2 Saida travamento portas:"));
    Serial.println(pinToName(RL_DL));
    Serial.print(F("3 Saida pisca alerta:"));
    Serial.println(pinToName(RL_HZ));
    Serial.print(F("4 Entrada bico injetor:"));    
    Serial.println(pinToName(injector_pin));
    Serial.print(F("5 Entrada velocidade da roda:")); 
    Serial.println(pinToName(speed_in_pin));
    Serial.print(F("6 Entrada luz de freio:")); 
    Serial.println(pinToName(breakLight));
    Serial.println(F("ESC Voltar"));
    varType typev = UCHAR;
    for (;;) {
        switch (Serial.read()) {
            case '1':  subMenu_range(22, RL3, RL1); break;
            case '2':  subMenu_range(23, RL3, RL1); break;
            case '3':  subMenu_range(24, RL3, RL1); break;
            case '4':  subMenu_range(26, DI2, DI1); break;
            case '5':  subMenu_range(25, DI2, DI1); break;
            case '6':  subMenu_range(27, DI2, DI3); break;
            case 27: return;
            default: continue;  // includes the case 'no input'
        }
        Menu();
        break;
    }
}
 
void testReport(){
   float volts = analogRead(voltageIn)*0.0197f;
   Serial.print(F("Tensao bateria: "));
   Serial.print(volts);
   Serial.println(F(" v"));
   inputFreq dg1, dg2;
   readFrequency(injector_pin, 15, &dg1);
   readFrequency(speed_in_pin, 15, &dg2);
   Serial.print(F("Digital Input 1: "));
   if (dg1.freq > 126.0f || dg1.freq <  114.0f){
      Serial.println("Failed!");
      Serial.print("DG1:");
      Serial.println(dg1.freq);
   }
   else{
      Serial.println("OK");
      Serial.print(F("Digital Output 1: "));
      Serial.println("OK");
   }
   Serial.print(F("Digital Input 2: "));
   if (dg2.freq > 126.0f || dg2.freq <  114.0f){
      Serial.println("Failed!");
      Serial.print("DG2:");
      Serial.println(dg2.freq);
   }
   else{
      Serial.println("OK");
      Serial.print(F("Digital Output 2: "));
      Serial.println("OK");
   }
  Serial.print(F("Digital input 3: "));
  Serial.println(analogRead(breakLight));
   
   Serial.print(F("Sensor Input 1: "));
   if (digitalRead(sensorTemp) != HIGH)
      Serial.println("Failed!");
   else
      Serial.println("OK");
   Serial.print(F("Sensor Input 2: "));
   if (digitalRead(intakeAirTemp) != HIGH)
      Serial.println("Failed!");
   else
      Serial.println("OK");
   Serial.print(F("Analog Input 1: "));
      if (digitalRead(sensorPressure) != HIGH)
      Serial.println("Failed!");
   else
      Serial.println("OK");
   Serial.print(F("Analog Input 2: "));
   if (digitalRead(sensorPressure2) != HIGH)
      Serial.println("Failed!");
   else
      Serial.println("OK");
  digitalWrite(RL1, HIGH);
  digitalWrite(RL2, HIGH);
  Serial.print(F("setButton: "));
  if (digitalRead(setButton) != LOW)
    Serial.println("Failed!");
  else
    Serial.println("OK");
  Serial.print(F("menuButton: "));
  if (digitalRead(menuButton) != LOW)
    Serial.println("Failed!");
  else
    Serial.println("OK");
}

void clearScreen(){
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
}
