#include <DS3231.h>
#define echoPin 11 //Echo Pin
#define trigPin 12 //Trigger Pin
#define lampu 6
#define pompa 7 //pompa default
#define minipumps 8 //minipumps default 
#define TdsSensorPin A1
#define LDR A8
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  5           // sum of sample point
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneMQTTEthernet.h>

int LDRValue = 0;
int UltrasonicValue = 0;
int Hair;
int HT = 20;
int TDSValue = 0;
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
int maximumRange = 20; //kebutuhan akan maksimal range
int minimumRange = 10; //kebutuhan akan minimal range
int kepekatan;  
long duration, distance; //waktu untuk kalkulasi jarak
float averageVoltage = 0,tdsValue = 0,temperature = 25;
int getMedianNum(int bArray[], int iFilterLen);
unsigned long starttime;
unsigned long endtime;


char username[] = "47882ce0-50b9-11ea-b301-fd142d6c1e6c";
char password[] = "029d7070c597f578d165fe6de5d1c22d6ae51c3c";
char clientID[] = "057982d0-50ba-11ea-ba7c-716e7f5ba423";

DS3231  rtc(SDA, SCL);
Time t;

const int OnHour = 8;
const int OffHour = 18;


void setup() {
    Serial.begin(9600);
    rtc.begin();
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(lampu, OUTPUT);
    pinMode(pompa, OUTPUT);
    pinMode(minipumps, OUTPUT);
    pinMode(TdsSensorPin,INPUT);
    pinMode(LDR,INPUT); 
   
   Cayenne.begin(username, password, clientID);
}

void loop() {
 Cayenne.loop();
}

CAYENNE_OUT_DEFAULT()
{
   static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      //Serial.print(tdsValue);
      Serial.println("ppm");
   }

      digitalWrite(trigPin, LOW); 
      digitalWrite(trigPin, HIGH);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = (duration/2) / 29.1;
      delay(50);
      
      t = rtc.getTime();
      Serial.print(t.hour);
      Serial.print(" hour(s), ");
      Serial.print(" minute(s)");
      Serial.print(t.min);
      Serial.println(" ");
      delay (100);

      Serial.print("Intensitas Cahaya:");
      LDRValue = analogRead(LDR);
      Serial.println(LDRValue);
      delay(1000);
      Hair=HT-distance;
      Serial.print("Tinggi larutan:");
      Serial.println(Hair);
      delay(1000);

      Cayenne.virtualWrite(1, Hair);
      Cayenne.virtualWrite(2, LDRValue);
      Cayenne.virtualWrite(3, tdsValue);
      Cayenne.virtualWrite(4, 0);
      Cayenne.virtualWrite(5, 0);
      Cayenne.virtualWrite(6, 0);
      Cayenne.virtualWrite(7, 0);

       if(t.hour >= OnHour){
          if (LDRValue < 40 )
          {
            starttime = millis();
            endtime = starttime;
            while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
             {
              Serial.println (LDRValue);
              digitalWrite(lampu,LOW);
              
              endtime = millis();
              delay(1000);
             }
               if(LDRValue < 40)
               {
                Serial.println (LDRValue);
                Cayenne.virtualWrite(4, 1);
               }
               else
               {
                Serial.println (LDRValue);
                Cayenne.virtualWrite(4, 0);
               }
          }
          else
          {
            digitalWrite(lampu,HIGH);
            Serial.println("LIGHT OFF");
          }
        
       }
       else if (t.hour >= OffHour)
       {
            digitalWrite(lampu,HIGH);
            Serial.println("LIGHT OFF");   
       }
       else {
            digitalWrite(lampu,HIGH);
            Serial.println("LIGHT OFF");   
       }


        if (Hair < 10)
        {
            starttime = millis();
            endtime = starttime;

            while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
             {
                Serial.println (Hair);
                digitalWrite(pompa, LOW);
                endtime = millis();
                delay(1000);
             }
             if(Hair < 10){
               Serial.println (Hair);
               Cayenne.virtualWrite(5, 1);
             }
             else{
               Serial.println (Hair);
               Cayenne.virtualWrite(5, 0);
             }
           
                  if(TDSValue < 560){
                       
                       starttime = millis();
                       endtime = starttime;
                       delay(1000);
            
                        while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
                       {
                         Serial.println (TDSValue);
                         digitalWrite(minipumps, LOW);
                         endtime = millis();
                         delay(1000);
                       }
                       if (TDSValue < 560){
                          Serial.println (TDSValue);
                          Cayenne.virtualWrite(6, 1);
                       }
                       else{
                          Serial.println (TDSValue);
                          Cayenne.virtualWrite(6, 0);
                       }
                  }
                  else if (TDSValue > 840)
                  {  
                            starttime = millis();
                            endtime = starttime;
                            while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
                           {
                             Serial.println (TDSValue);
                             digitalWrite(pompa, LOW);
                             endtime = millis();
                             delay(1000);
                           }
                           if (TDSValue > 840){
                              Serial.println (TDSValue);
                              Cayenne.virtualWrite(7, 1);
                           }
                           else{
                              Serial.println (TDSValue);
                              Cayenne.virtualWrite(7, 0);
                           }
                  }
                  else
                  {
                    digitalWrite(minipumps, HIGH);
                     digitalWrite(pompa, HIGH);
                  }
        }else
        {
          digitalWrite (pompa, HIGH);
          if(TDSValue < 560){

                 starttime = millis();
                 endtime = starttime;
                 delay(1000);
            
                  while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
                   {
                         Serial.println (TDSValue);
                         digitalWrite(minipumps, LOW);
                         endtime = millis();
                         delay(1000);
                   }
                         if (TDSValue < 560){
                            Serial.println (TDSValue);
                            Cayenne.virtualWrite(6, 1);
                         }
                         else{
                            Serial.println (TDSValue);
                            Cayenne.virtualWrite(6, 0);
                         } digitalWrite(minipumps, LOW);
            }
            else if (TDSValue > 840){
              
                  starttime = millis();
                  endtime = starttime;
                  while ((endtime - starttime) < 10000) // do this loop for up to 1000mS
                   {
                       Serial.println (TDSValue);
                       digitalWrite(pompa, LOW);
                       endtime = millis();
                       delay(1000);
                   }
                           if (TDSValue > 840){
                              Serial.println (TDSValue);
                              Cayenne.virtualWrite(7, 1);
                           }
                           else{
                              Serial.println (TDSValue);
                              Cayenne.virtualWrite(7, 0);
                           }
            }
            else
            {
              digitalWrite(minipumps, HIGH);
               digitalWrite(pompa, HIGH);
            }
          
         }

}
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
