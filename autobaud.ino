long detRate00(int recpin)  // function to return valid received baud rate
                         // Note that the serial monitor has no 600 baud option and 300 baud
                         // doesn't seem to work with version 22 hardware serial library
 {
 long baud, rate = 10000, x;
 for (int i = 0; i < 10; i++) {
     while(digitalRead(recpin) == 1){} // wait for low bit to start
     x = pulseIn(recpin,LOW);   // measure the next zero bit width
     rate = x < rate ? x : rate;
 }
 
 if (rate < 12)
     baud = 115200;
     else if (rate < 20)
     baud = 57600;
     else if (rate < 29)
     baud = 38400;
     else if (rate < 40)
     baud = 28800;
     else if (rate < 60)
     baud = 19200;
     else if (rate < 80)
     baud = 14400;
     else if (rate < 150)
     baud = 9600;
     else if (rate < 300)
     baud = 4800;
     else if (rate < 600)
     baud = 2400;
     else if (rate < 1200)
     baud = 1200;
     else
     baud = 0;  
  return baud;
 }

 long detRate(int recpin)  // function to return valid received baud rate
                         // Note that the serial monitor has no 600 baud option and 300 baud
                         // doesn't seem to work with version 22 hardware serial library
 {
  while(digitalRead(recpin) == 1){} // wait for low bit to start
  long baud;
  long rate = pulseIn(recpin,LOW);   // measure zero bit width from character 'U'
    if (rate < 0)
      baud = 0;

     else if (rate < 4)
      baud = 500000;   
     else if (rate < 6)
      baud = 230400;          
     else if (rate < 12)
      baud = 115200;
     else if (rate < 20)
       baud = 57600;
     else if (rate < 29)
       baud = 38400;
     else if (rate < 40)
      baud = 28800;
     else if (rate < 60)
      baud = 19200;
     else if (rate < 80)
      baud = 14400;
     else if (rate < 150)
       baud = 9600;
     else if (rate < 300)
       baud = 4800;
     else if (rate < 600)
       baud = 2400;
     else if (rate < 1200)
       baud = 1200;
     else 
       baud = 0;  
  return baud; 
 }