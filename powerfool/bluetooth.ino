void sendBluetooth(inputFreq injectorInput, inputFreq speedInput, float volts, int sensorPressureVal){
   if(BTSerial.available() > 0){
    BTSerial.println("Entrada velocidade: ");
    BTSerial.println(speedInput.freq);
    BTSerial.print(F("Saida velocidade: "));
    BTSerial.println(out_freq[1]);
    BTSerial.print(F("Distancia total (km): "));
    BTSerial.println(totalMileage/1000);
    BTSerial.print(F("Velocidade: "));
    BTSerial.print(out_freq[1]/((float) (speedSensor/3600.0f)));
    BTSerial.println(F(" km/h"));
    BTSerial.print(F("Tensao bateria: "));
    BTSerial.print(volts);
    BTSerial.println(F(" v"));
    BTSerial.print(F("RPM: "));
    BTSerial.println((int)(injectorInput.freq * 60 *((settings & 2 == 0)*2) )); // semi, sequential
    BTSerial.print(F("Pressao sensor: "));
    BTSerial.println((int)sensorPressureVal);
    BTSerial.write(27);
    }
}
