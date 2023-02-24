#ifdef BUILD_BLUETOOTH
void sendBluetooth(inputFreq injectorInput, inputFreq speedInput, float volts, int sensorPressureVal){
    byte buf[8];

  // build & send CAN frames to RealDash.
  // a CAN frame payload is always 8 bytes containing data in a manner
  // described by the RealDash custom channel description XML file
  // all multibyte values are handled as little endian by default.
  // endianess of the values can be specified in XML file if it is required to use big endian values

  // build 1st CAN frame, RPM, VOLTS, SPEED, SENSOR
  unsigned int rpm = (int)(injectorInput.freq * 60 *((settings & 2 == 0)*2) );
  unsigned int voltage = volts*10;
  unsigned int speed =  (unsigned char) (speedInput.freq/((float) (speedSensor/3600.0f)));
  memcpy(buf, &rpm, 2);
  memcpy(buf + 2, &voltage, 2);
  memcpy(buf + 4, &speed, 2);
  memcpy(buf + 6, &sensorPressureVal, 2);

  // write first CAN frame to serial
  SendCANFrameToSerial(3200, buf);
}


void SendCANFrameToSerial(unsigned long canFrameId, const byte* frameData)
{
  // the 4 byte identifier at the beginning of each CAN frame
  // this is required for RealDash to 'catch-up' on ongoing stream of CAN frames
  const byte serialBlockTag[4] = { 0x44, 0x33, 0x22, 0x11 };
  BTSerial.write(serialBlockTag, 4);

  // the CAN frame id number (as 32bit little endian value)
  BTSerial.write((const byte*)&canFrameId, 4);

  // CAN frame payload
  BTSerial.write(frameData, 8);
}
#endif
