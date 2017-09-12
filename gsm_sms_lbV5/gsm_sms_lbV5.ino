#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include "LiquidCrystal_I2C.h"

RTC_DS3231 RTC;
LiquidCrystal_I2C lcd(0x27, 16, 2);
SMSGSM sms;

#define relay1 10
#define relay2 9
#define buzzer 6   // 6
#define led1 5
#define led2 13

float temp_float;
char state_buffer[3];

boolean started = false;
char smsbuffer[50];
char phone_number[15];
char cphone;
String strphone_number = "";
String str_cc[5];
String chk_cc = "";
byte hour_onint,minute_onint,hour_offint,minute_offint;
boolean start_on = false;
boolean start_off = false;
boolean start_check = false;
boolean start_cc = false;
boolean sol_cc = false;
boolean qin = false;
boolean var_error = false;
boolean var_clear = false;
boolean relay2_state;
byte lcd_page = 0;
byte lcd_couter = 0;
byte numq = 0;
byte qcc = 0;
byte qhour_onint[5];
byte qhour_offint[5];
byte qminute_onint[5]; 
byte qminute_offint[5];
byte qhon[5];
byte qhoff[5];
byte qmon[5]; 
byte qmoff[5];
byte chon,cmon,choff,cmoff;
byte hour_ontemp,min_ontemp,hour_offtemp,min_offtemp;
void setup()
{
  //Serial connection.
  Wire.begin();
  RTC.begin();
  lcd.home();
  lcd.begin();
  //lcd.noBacklight();
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(buzzer, LOW);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(led1,HIGH);delay(250);digitalWrite(led1,LOW);
  digitalWrite(led2,HIGH);delay(250);digitalWrite(led2,LOW);
  digitalWrite(buzzer,HIGH);delay(250);digitalWrite(buzzer,LOW);
  Serial.begin(9600);
  lcd.print("Wait for signal.");
  //Serial.println("System Setup");
  if(gsm.begin(115200))
  {
     Serial.println("Status=READY");
     started=true;
     lcd.setCursor(0,0);
     lcd.print("System is Ready!");
     delay(300);
     lcd.setCursor(0,1);
     lcd.print("=======<<=======");
     delay(5000);
     //sms.SendSMS("0932305135", "System was rebooted again! Queued stack before was cancel all.");
     //sms.SendSMS("0864390821", "System was rebooted again! Queued stack before was cancel all.");
     //lcd.clear();
  //Serial.println("OK");
  }else 
  Serial.println("Status=IDLE");delay(3000); // give time for GSM module to register on network.
  
}
void loop()
{
  if(sol_cc==true)
  {
    //String chk_cc, str_cc[3], qhon[3], qmon[3], qhoff[3], qmoff[3]
    for(byte i=0;i<5;i++)
    {
      if(chk_cc == str_cc[i])
      {
        Serial.print(i);Serial.print(" ");Serial.println("OK! CHK_CC = STR_CC ...");
        for(byte j=i;j<=qcc;j++)
        {
          qhon[j]=0;
          qmon[j]=0;
          qhoff[j]=0;
          qmoff[j]=0;
          qhon[j] = qhon[j+1];
          qmon[j] = qmon[j+1];
          qhoff[j] = qhoff[j+1];
          qmoff[j] = qmoff[j+1];
        }
      }
    }
    qcc--;
    sol_cc = false;
  }
  
  if(qin==true)
  {
    //hour_onint, minute_onint, hour_offint, minute_offint
    hour_ontemp = hour_onint;  min_ontemp = minute_onint;
    hour_offtemp = hour_offint;  min_offtemp = minute_offint;
    str_cc[qcc] = "";
    str_cc[qcc]+=cphone;
    str_cc[qcc]+=hour_onint;
    str_cc[qcc]+=minute_onint;
    str_cc[qcc]+=hour_offint;
    str_cc[qcc]+=minute_offint;
    
    qhon[qcc]=hour_onint;
    qmon[qcc]=minute_onint;
    qhoff[qcc]=hour_offint;
    qmoff[qcc]=minute_offint;
    qcc++;
    
    if(numq==0)
    {
      qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
      qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
      numq++;
    }else if(numq>0)
    {
      if(hour_ontemp == qhour_onint[numq-1] && hour_offtemp == qhour_offint[numq-1])    //  ==&&==
      {
        if(min_ontemp <= qminute_onint[numq-1] && min_offtemp >= qminute_offint[numq-1])
        {
          qhour_onint[numq-1] = hour_ontemp;  qminute_onint[numq-1] = min_ontemp;
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(min_ontemp > qminute_onint[numq-1] && min_ontemp <= qminute_offint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(min_ontemp > qminute_onint[numq-1] && min_ontemp > qminute_offint[numq-1])
        {
          qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
          qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
          numq++;
        }
      }else if(hour_ontemp == qhour_onint[numq-1] && hour_offtemp > qhour_offint[numq-1])    //  ==&& >
      {
        if(hour_ontemp < qhour_offint[numq-1] && min_ontemp >= qminute_onint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }if(hour_ontemp < qhour_offint[numq-1] && min_ontemp <= qminute_onint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(hour_ontemp == qhour_offint[numq-1] && min_ontemp <= qminute_offint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(hour_ontemp == qhour_offint[numq-1] && min_ontemp <= qminute_onint[numq-1])
        {
          qhour_onint[numq-1] = hour_ontemp;  qminute_onint[numq-1] = min_ontemp;
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(hour_ontemp == qhour_offint[numq-1] && min_ontemp > qminute_onint[numq-1])
        {
            qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
            qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
            numq++;        
        }
      }else if(hour_ontemp < qhour_onint[numq-1] && hour_offtemp == qhour_offint[numq-1])    //  < &&==
      {
        if(min_offtemp >= qminute_offint[numq-1])
        {
          qhour_onint[numq-1] = hour_ontemp;  qminute_onint[numq-1] = min_ontemp;
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }
      }else if(hour_ontemp < qhour_onint[numq-1] && hour_offtemp > qhour_offint[numq-1])    //  < && >
      {
        qhour_onint[numq-1] = hour_ontemp;  qminute_onint[numq-1] = min_ontemp;
        qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
      }else if(hour_ontemp > qhour_onint[numq-1] && hour_offtemp == qhour_offint[numq-1])    //  > &&==
      {
        if(hour_ontemp < qhour_offint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }else if(hour_ontemp == qhour_offint[numq-1])
        {
          if(min_ontemp <= qminute_offint[numq-1])
          {
            qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
          }else
          {
            qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
            qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
            numq++;
          }
        }
      }else if(hour_ontemp > qhour_onint[numq-1] && hour_offtemp > qhour_offint[numq-1])    //  > && >
      {
        if(hour_ontemp == qhour_offint[numq-1])
        {
          if(min_ontemp <= qminute_offint[numq-1])
          {
            qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
          }else if(min_ontemp > qminute_offint[numq-1])
          {
            qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
            qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
            numq++;
          }
        }else if(hour_ontemp > qhour_offint[numq-1])
        {
          qhour_onint[numq] = hour_ontemp;  qminute_onint[numq] = min_ontemp;
          qhour_offint[numq] = hour_offtemp;  qminute_offint[numq] = min_offtemp;
          numq++;
        }else if(hour_ontemp < qhour_offint[numq-1])
        {
          qhour_offint[numq-1] = hour_offtemp;  qminute_offint[numq-1] = min_offtemp;
        }
      }
    }
    
    var_clear==true;
    qin=false;
  }
  
  if(var_clear==true)
  {
    qhour_onint[numq+1] = 0;  qminute_onint[numq+1] = 0;
    qhour_offint[numq+1] = 0;  qminute_offint[numq+1] = 0;      
    hour_onint=0;  minute_onint=0;
    hour_offint=0;  minute_offint=0;
    var_clear = false;
  }
  
//Big Loop 
  if(started)
  { 
   if(lcd_couter%2==0){
      digitalWrite(led1,HIGH);delay(100);
      digitalWrite(led2,LOW);delay(450);
      digitalWrite(led1,HIGH);
    }else{
      digitalWrite(led2,HIGH);delay(100);
      digitalWrite(led1,LOW);delay(450);
      digitalWrite(led2,HIGH);
    }
      
      if(lcd_page==0)
      {
        lcd_clock();
      }else if(lcd_page==1)
      {
        show_stat();
      }else if(lcd_page==2)
      {
        show_qpump1();
      }else if(lcd_page==3)
      {
        show_qpump2();
      }
      lcd_couter++;
      if(lcd_couter==2){
      lcd_page++;
      lcd_couter=0;
      }
      if(lcd_page==4){
      lcd_page=0;
      }
    
     for(byte l=0;l<numq;l++)
     {  
       for(byte i=0;i<numq;i++)
       {
         if(qhour_onint[i+1]!=0)
         {
           if(qhour_onint[i+1]==qhour_onint[i])
           {
             if(qminute_onint[i+1]<=qminute_onint[i])
             {
               byte temp_hon = qhour_onint[i];
               byte temp_mon = qminute_onint[i];
               qhour_onint[i] = qhour_onint[i+1];
               qminute_onint[i] = qminute_onint[i+1];
               qhour_onint[i+1] = temp_hon;
               qminute_onint[i+1] = temp_mon;
             }
           }
           if(qhour_onint[i+1]<qhour_onint[i] && qhour_offint[i+1]!=0)
           {
             byte temp_hon = qhour_onint[i];
             byte temp_mon = qminute_onint[i];
             qhour_onint[i] = qhour_onint[i+1];
             qminute_onint[i] = qminute_onint[i+1];
             qhour_onint[i+1] = temp_hon;
             qminute_onint[i+1] = temp_mon;
           }
           if(qhour_offint[i+1]==qhour_offint[i])
           {
             if(qminute_offint[i+1]<=qminute_offint[i])
             {
               byte temp_hoff = qhour_offint[i];
               byte temp_moff = qminute_offint[i];
               qhour_offint[i] = qhour_offint[i+1];
               qminute_offint[i] = qminute_offint[i+1];
               qhour_offint[i+1] = temp_hoff;
               qminute_offint[i+1] = temp_moff;
             }
           }
           if(qhour_offint[i+1]<qhour_offint[i])
           {
             byte temp_hoff = qhour_offint[i];
             byte temp_moff = qminute_offint[i];
             qhour_offint[i] = qhour_offint[i+1];
             qminute_offint[i] = qminute_offint[i+1];
             qhour_offint[i+1] = temp_hoff;
             qminute_offint[i+1] = temp_moff;
           }
         }
       }
     }
     
   if(numq>0)
   {
     for(byte i=numq;i<0;i--)
     {
       if(qhour_onint[i] == qhour_onint[i-1] && qhour_offint[i] == qhour_offint[i-1])
       {
         if(qminute_onint[i] <= qminute_onint[i-1] && qminute_offint[i] >= qminute_offint[i-1])
         {
           qhour_onint[i-1] == qhour_onint[i];  qminute_onint[i-1] == qminute_onint[i];
           qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
           numq--;
         }
       }
       if(qhour_onint[i] == qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1])
       {
         if(qminute_onint[i] <= qminute_onint[i-1])
         {
           qhour_onint[i-1] == qhour_onint[i];  qminute_onint[i-1] == qminute_onint[i];
           qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
           numq--;
         }
       }
       if(qhour_onint[i] < qhour_onint[i-1] && qhour_offint[i] == qhour_offint[i-1])
       {
         if(qminute_offint[i] >= qminute_offint[i-1])
         {
           qhour_onint[i-1] == qhour_onint[i];  qminute_onint[i-1] == qminute_onint[i];
           qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
           numq--;
         }
       }
       if(qhour_onint[i] < qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1])
       {
         qhour_onint[i-1] == qhour_onint[i];  qminute_onint[i-1] == qminute_onint[i];
         qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
         numq--; 
       }
       if(qhour_onint[i] > qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1] && qhour_onint[i] < qhour_offint[i-1])
       {
         qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
         numq--; 
       }else if(qhour_onint[i] > qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1] && qhour_onint[i] == qhour_offint[i-1])
       {
         if(qminute_onint[i] <= qminute_offint[i-1])
         {
           qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
           numq--;
         } 
       }
       qhour_onint[i]=0;  qminute_onint[i]=0;
       qhour_offint[i]=0;  qminute_offint[i]=0;
     }
   }
   
   if(numq>0)
   {
     int i = numq-1;
     if(qhour_onint[i] > qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1] && qhour_onint[i] < qhour_offint[i-1])
       {
         qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
         qhour_onint[i]=0;  qminute_onint[i]=0;
         qhour_offint[i]=0;  qminute_offint[i]=0;
         numq--; 
       }else if(qhour_onint[i] > qhour_onint[i-1] && qhour_offint[i] > qhour_offint[i-1] && qhour_onint[i] == qhour_offint[i-1])
       {
         if(qminute_onint[i] <= qminute_offint[i-1])
         {
           qhour_offint[i-1] == qhour_offint[i];  qminute_offint[i-1] == qminute_offint[i];
           qhour_onint[i]=0;  qminute_onint[i]=0;
           qhour_offint[i]=0;  qminute_offint[i]=0;
           numq--;
         } 
       }
   }

     Serial.print("All Queue = ");Serial.println(numq);
     for(byte i=0;i<numq;i++)
       {
          Serial.print(i+1);Serial.print(" ON > ");Serial.print(qhour_onint[i]);Serial.print(":");Serial.print(qminute_onint[i]);
          Serial.print("  OFF > ");Serial.print(qhour_offint[i]);Serial.print(":");Serial.println(qminute_offint[i]);
       }
     for(byte i=0;i<qcc;i++)
       {
          Serial.print(i);Serial.print(" String STR >");Serial.println(str_cc[i]);
       }
     Serial.print(qcc);Serial.print(" ");Serial.print("#");Serial.print(" String CC  >");Serial.println(chk_cc);
     
       DateTime now = RTC.now();
       Active_Relay();
       
          //get 1st sms
          sms.GetSMS(1,phone_number,20,smsbuffer,20);
          strphone_number = "";
          strphone_number = phone_number;
          if(strphone_number=="+66864390821"||strphone_number=="+66933013813"||strphone_number=="+66932305135")
          {
            digitalWrite(buzzer,HIGH);delay(100);
            digitalWrite(buzzer,LOW);delay(50);
            digitalWrite(buzzer,HIGH);delay(100);
            digitalWrite(buzzer,LOW);delay(50);
            digitalWrite(buzzer,HIGH);delay(100);
            digitalWrite(buzzer,LOW);delay(50);
            lcd.clear();
            lcd.setCursor(1,0);
            lcd.print("+SMS INCOMING+");
            lcd.setCursor(2,1);
            lcd.print(phone_number);
            delay(1000);
            
            if(strphone_number=="+66864390821")
            {
              cphone = 'A';
            }else if(strphone_number=="+66933013813")
            {
              cphone = 'B';
            }else if(strphone_number=="+66932305135")
            {
              cphone = 'C';
            }
            
            if(smsbuffer[0]=='O' && smsbuffer[1]=='N')
            {
              start_on = true;
              if(smsbuffer[6]=='\n' && smsbuffer[7]=='O' && smsbuffer[8]=='F' && smsbuffer[9]=='F')
              {
                start_off = true;
              }
            }else if(smsbuffer[0]=='C' && smsbuffer[1]=='H' && smsbuffer[2]=='E' && smsbuffer[3]=='C' && smsbuffer[4]=='K')
            {
              start_check = true;
            }else if(smsbuffer[0]=='C' && smsbuffer[1]=='A' && smsbuffer[2]=='N' && smsbuffer[3]=='C' && smsbuffer[4]=='E' && smsbuffer[5]=='L' && smsbuffer[6]=='\n')
            {
              start_cc = true;
            }else{}
            
            if(start_on == true)
            {
              //hour unit 1
              if(smsbuffer[2] == '0')
              {
                int temp1=smsbuffer[2]-48;
                hour_onint=temp1*10;
              }else if(smsbuffer[2] == '1')
              {
                int temp1=smsbuffer[2]-48;
                hour_onint=temp1*10;
              }else if(smsbuffer[2] == '2')
              {
                int temp1=smsbuffer[2]-48;
                hour_onint=temp1*10;
              }else{hour_onint=99;}
              //hour unit 2
              if(smsbuffer[3] == '0')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '1')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '2')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '3')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '4')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '5')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '6')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '7')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '8')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[3] == '9')
              {
                int temp2=smsbuffer[3]-48;
                hour_onint+=temp2;
              }
              //minute unit 1
              if(smsbuffer[4] == '0')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[4] == '1')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[4] == '2')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[4] == '3')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[4] == '4')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[4] == '5')
              {
                int temp3=smsbuffer[4]-48;
                minute_onint=temp3*10;
              }else{minute_onint=99;}
              //minute unit 2
              if(smsbuffer[5] == '0')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[5] == '2')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[5] == '4')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[5] == '5')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[5] == '6')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[5] == '8')
              {
                int temp4=smsbuffer[5]-48;
                minute_onint+=temp4;
              }
              //ident base time correct?
              if(hour_onint > 0 && hour_onint <= 23 && minute_onint%2==0 || minute_onint%5==0 && minute_onint<60 )
              {
                hour_onint = hour_onint;
                minute_onint = minute_onint;
              }else{
                hour_onint = 0;
                minute_onint = 0;
              }
              start_on = false;    
            }
            
            if(start_off == true)
            {
              //hour unit 1
              if(smsbuffer[10] == '0')
              {
                int temp1=smsbuffer[10]-48;
                hour_offint=temp1*10;
              }else if(smsbuffer[10] == '1')
              {
                int temp1=smsbuffer[10]-48;
                hour_offint=temp1*10;
              }else if(smsbuffer[10] == '2')
              {
                int temp1=smsbuffer[10]-48;
                hour_offint=temp1*10;
              }else{hour_offint=99;}
              //hour unit 2
              if(smsbuffer[11] == '0')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '1')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '2')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '3')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '4')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '5')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '6')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '7')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '8')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[11] == '9')
              {
                int temp2=smsbuffer[11]-48;
                hour_offint+=temp2;
              }
              //minute unit 1
              if(smsbuffer[12] == '0')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[12] == '1')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[12] == '2')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[12] == '3')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[12] == '4')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[12] == '5')
              {
                int temp3=smsbuffer[12]-48;
                minute_offint=temp3*10;
              }else{minute_offint=99;}
              //minute unit 2
              if(smsbuffer[13] == '0')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[13] == '2')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[13] == '4')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[13] == '5')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[13] == '6')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[13] == '8')
              {
                int temp4=smsbuffer[13]-48;
                minute_offint+=temp4;
              }
              //ident base time correct?
              if(hour_onint > 0 && hour_offint > 0 && hour_offint <= 23 && hour_offint >= hour_onint && minute_offint%2 == 0 || minute_offint%5 == 0 && minute_offint < 60)
              {
                if(hour_offint > hour_onint)
                {
                  hour_offint = hour_offint;
                  minute_offint = minute_offint;
                  qin = true;
                }else if(hour_offint == hour_onint)
                {
                  if(minute_offint > minute_onint)
                   { 
                    hour_offint = hour_offint;
                    minute_offint = minute_offint;
                    qin = true;
                    }
                  }
                }else{qin = false;}
              //sms.SendSMS(phone_number, "OK! Your Command is Queue.");
              }else{
                //sms.SendSMS(phone_number, "Command is ERROR! Please send again later.");
                delay(1000);
              }
              start_off = false;
            }else if(start_check == true)
            {
              //Detect_state();
              //String sendsms = "OK! Water pump is ";
              //char buff[50];
              relay2_state = digitalRead(relay2);
              if(relay2_state==0){
                sms.SendSMS(phone_number, "OK! Water pump is OFF.");
                /*sendsms+=state_buffer;
                sendsms+=". Stopper last by ";
                sendsms+=stopphonenum;
                sendsms.toCharArray(buff,50);
                //sms.SendSMS(phone_number, buff);
                Serial.println(buff);
                sendsms=="";
                */
              }
              if(relay2_state==1){
                //Detect_state();
                //String sendsms = "OK! Water pump is ";
                //char buff[50];
                relay2_state = digitalRead(relay2);
                if(relay2_state==1){
                  sms.SendSMS(phone_number, "OK! Water pump is ON.");
                  /*sendsms+=state_buffer;
                  sendsms+=". Stopper last by ";
                  sendsms+=stopphonenum;
                  sendsms.toCharArray(buff,50);
                  //sms.SendSMS(phone_number, buff);
                  Serial.println(buff);
                  sendsms=="";
                  */
                }
              }
              delay(1000);
              start_check = false;
              
            }else if(start_cc == true)
            {
              //hour unit 1
              if(smsbuffer[7] == '0')
              {
                int temp1=smsbuffer[7]-48;
                hour_onint=temp1*10;
              }else if(smsbuffer[7] == '1')
              {
                int temp1=smsbuffer[7]-48;
                hour_onint=temp1*10;
              }else if(smsbuffer[7] == '2')
              {
                int temp1=smsbuffer[7]-48;
                hour_onint=temp1*10;
              }else{hour_onint=99;}
              //hour unit 2
              if(smsbuffer[8] == '0')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '1')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '2')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '3')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '4')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '5')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '6')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '7')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '8')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }else if(smsbuffer[8] == '9')
              {
                int temp2=smsbuffer[8]-48;
                hour_onint+=temp2;
              }
              //minute unit 1
              if(smsbuffer[9] == '0')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[9] == '1')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[9] == '2')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[9] == '3')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[9] == '4')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else if(smsbuffer[9] == '5')
              {
                int temp3=smsbuffer[9]-48;
                minute_onint=temp3*10;
              }else{minute_onint=99;}
              //minute unit 2
              if(smsbuffer[10] == '0')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[10] == '2')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[10] == '4')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[10] == '5')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[10] == '6')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }else if(smsbuffer[10] == '8')
              {
                int temp4=smsbuffer[10]-48;
                minute_onint+=temp4;
              }
              //ident base time correct?
              if(hour_onint > 0 && hour_onint <= 23 && minute_onint%2==0 || minute_onint%5==0 && minute_onint<60 )
              {
                chon = hour_onint;
                cmon = minute_onint;
              }
              //hour unit 1
              if(smsbuffer[12] == '0')
              {
                int temp1=smsbuffer[12]-48;
                hour_offint=temp1*10;
              }else if(smsbuffer[12] == '1')
              {
                int temp1=smsbuffer[12]-48;
                hour_offint=temp1*10;
              }else if(smsbuffer[12] == '2')
              {
                int temp1=smsbuffer[12]-48;
                hour_offint=temp1*10;
              }else{hour_offint=99;}
              //hour unit 2
              if(smsbuffer[13] == '0')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '1')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '2')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '3')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '4')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '5')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '6')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '7')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '8')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }else if(smsbuffer[13] == '9')
              {
                int temp2=smsbuffer[13]-48;
                hour_offint+=temp2;
              }
              //minute unit 1
              if(smsbuffer[14] == '0')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[14] == '1')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[14] == '2')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[14] == '3')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[14] == '4')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else if(smsbuffer[14] == '5')
              {
                int temp3=smsbuffer[14]-48;
                minute_offint=temp3*10;
              }else{minute_offint=99;}
              //minute unit 2
              if(smsbuffer[15] == '0')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[15] == '2')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[15] == '4')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[15] == '5')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[15] == '6')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }else if(smsbuffer[15] == '8')
              {
                int temp4=smsbuffer[15]-48;
                minute_offint+=temp4;
              }
              //ident base time correct?
              if(hour_onint > 0 && hour_offint > 0 && hour_offint <= 23 && hour_offint >= hour_onint && minute_offint%2 == 0 || minute_offint%5 == 0 && minute_offint < 60)
              {
                if(hour_offint > hour_onint)
                {
                  choff = hour_offint;
                  cmoff = minute_offint;
                  chk_cc = "";
                  chk_cc += cphone;
                  chk_cc += chon;
                  chk_cc += cmon;
                  chk_cc += choff;
                  chk_cc += cmoff;
                  sol_cc = true;
                }else if(hour_offint == hour_onint)
                {
                  if(minute_offint > minute_onint)
                   { 
                    choff = hour_offint;
                    cmoff = minute_offint;
                    chk_cc = "";
                    chk_cc += cphone;
                    chk_cc += chon;
                    chk_cc += cmon;
                    chk_cc += choff;
                    chk_cc += cmoff;
                    sol_cc = true;
                  }
                }else{sol_cc = false;}
              }else{
                sol_cc = false;
              }//sms.SendSMS(phone_number, "OK! Your Command is Queue.");
              //end of start_cc
              start_cc = false;
          }else{}
          
          sms.DeleteSMS(1);
          delay(1000);
     }
}

void lcd_clock()
{
  DateTime now = RTC.now();
    lcd.clear();
    lcd.setCursor(2,0);
    if(now.hour()<10){
      lcd.print('0');lcd.print(now.hour());
    }else {lcd.print(now.hour());
      }
    lcd.setCursor(4,0);
    lcd.print(':');
    
    lcd.setCursor(5,0);
    if(now.minute()<10){
      lcd.print('0');lcd.print(now.minute());
    }else {lcd.print(now.minute());lcd.print(' ');
      }
    /*
    lcd.setCursor(6,0);
    if(now.second()<10){
      lcd.print('0');lcd.print(now.second());lcd.print('s');
    }else {lcd.print(now.second());lcd.print('s');
      }
    */
    lcd.setCursor(8,0);
    lcd.print(RTC.getTemperature());
    lcd.print('C');
    
    lcd.setCursor(0,1);    
    if(now.day()<10){
      lcd.print('0');lcd.print(now.day());lcd.print('/');
    }else{lcd.print(now.day());lcd.print('/');
      }
    
    if(now.month()<10){
      lcd.print('0');lcd.print(now.month());lcd.print('/');
    }else{lcd.print(now.month());lcd.print('/');
      }
    
    lcd.print(now.year());
    lcd.setCursor(12,1);
    lcd.print("sCPE");
}

void show_stat()
{
  Detect_state();
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Now Status");
  lcd.setCursor(0,1);
  lcd.print("WaterPump is ");lcd.print(state_buffer);
}

void show_qpump1()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Q1 ");
  if(qhon[0]<10){
    lcd.print('0');lcd.print(qhon[0]);lcd.print(".");
  }else {
    lcd.print(qhon[0]);lcd.print(".");
  }
  if(qmon[0]<10){  
    lcd.print('0');lcd.print(qmon[0]);
  }else{
    lcd.print(qmon[0]);
  }
  lcd.print(" - ");
  if(qhoff[0]<10){
    lcd.print('0');lcd.print(qhoff[0]);lcd.print(".");
  }else{
    lcd.print(qhoff[0]);lcd.print(".");
  }
  if(qmoff[0]<10){  
    lcd.print('0');lcd.print(qmoff[0]);
  }else{
    lcd.print(qmoff[0]);
  }
  lcd.setCursor(0,1);
  lcd.print("Q2 ");
  if(qhon[1]<10){
    lcd.print('0');lcd.print(qhon[1]);lcd.print(".");
  }else {
    lcd.print(qhon[1]);lcd.print(".");
  }
  if(qmon[1]<10){  
    lcd.print('0');lcd.print(qmon[1]);
  }else{
    lcd.print(qmon[1]);
  }
  lcd.print(" - ");
  if(qhoff[1]<10){
    lcd.print('0');lcd.print(qhoff[1]);lcd.print(".");
  }else{
    lcd.print(qhoff[1]);lcd.print(".");
  }
  if(qmoff[1]<10){  
    lcd.print('0');lcd.print(qmoff[1]);
  }else{
    lcd.print(qmoff[1]);
  }
}

void show_qpump2()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Q3 ");
  if(qhon[2]<10){
    lcd.print('0');lcd.print(qhon[2]);lcd.print(".");
  }else {
    lcd.print(qhon[2]);lcd.print(".");
  }
  if(qmon[2]<10){  
    lcd.print('0');lcd.print(qmon[2]);
  }else{
    lcd.print(qmon[2]);
  }
  lcd.print(" - ");
  if(qhoff[2]<10){
    lcd.print('0');lcd.print(qhoff[2]);lcd.print(".");
  }else{
    lcd.print(qhoff[2]);lcd.print(".");
  }
  if(qmoff[2]<10){  
    lcd.print('0');lcd.print(qmoff[2]);
  }else{
    lcd.print(qmoff[2]);
  }
  lcd.setCursor(0,1);
  lcd.print("Q4 ");
  if(qhon[3]<10){
    lcd.print('0');lcd.print(qhon[3]);lcd.print(".");
  }else {
    lcd.print(qhon[3]);lcd.print(".");
  }
  if(qmon[3]<10){  
    lcd.print('0');lcd.print(qmon[3]);
  }else{
    lcd.print(qmon[3]);
  }
  lcd.print(" - ");
  if(qhoff[3]<10){
    lcd.print('0');lcd.print(qhoff[3]);lcd.print(".");
  }else{
    lcd.print(qhoff[3]);lcd.print(".");
  }
  if(qmoff[3]<10){  
    lcd.print('0');lcd.print(qmoff[3]);
  }else{
    lcd.print(qmoff[3]);
  }
}

void Detect_state()
{ 
  relay2_state = digitalRead(relay2);
  if(relay2_state==0){state_buffer[0]='O';state_buffer[1]='F';state_buffer[2]='F';}
  if(relay2_state==1){state_buffer[0]=' ';state_buffer[1]='O';state_buffer[2]='N';}     
}

void Active_Relay()
{
  DateTime now = RTC.now();
  if(now.hour()>=qhour_onint[0] && now.minute()>=qminute_onint[0] && now.hour()<=qhour_offint[0] && now.minute()<qminute_offint[0])
  {
    boolean stat=digitalRead(relay2);
    if(stat==0)
    {
      PumpON();
      var_clear = true;
    }
  }
  if(now.hour()==qhour_offint[0] && now.minute()==qminute_offint[0])
  {
    boolean stat=digitalRead(relay2);
    if(stat==1)
    {
      PumpOFF();
      var_clear = true;
    }
  }
}

void PumpON()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water Pump is ON");
  lcd.setCursor(0,1);
  lcd.print("Please Wait.....");
  digitalWrite(buzzer,HIGH);
  digitalWrite(led2,HIGH);
  delay(500);
  digitalWrite(led2,LOW);
  digitalWrite(buzzer,LOW);
  digitalWrite(relay2,LOW);
  digitalWrite(relay1,HIGH);
  digitalWrite(led1,HIGH);
  delay(30000);
  lcd.setCursor(14,1);
  lcd.print("OK");
  digitalWrite(buzzer,HIGH);
  digitalWrite(led2,HIGH);
  delay(500);
  digitalWrite(led2,LOW);
  digitalWrite(buzzer,LOW);
  digitalWrite(relay2,HIGH);
  delay(10000);
  digitalWrite(led1,LOW);
  digitalWrite(relay1,LOW);
  delay(1000);
}

void PumpOFF()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WaterPump is OFF");
  lcd.setCursor(1,1);
  lcd.print("Wait next Queue.");
  digitalWrite(buzzer,HIGH);
  digitalWrite(led2,HIGH);
  delay(500);
  digitalWrite(buzzer,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(relay2,LOW);
  delay(1000);
  
  qhour_onint[0]=0;
  qminute_onint[0]=0;
  qhour_offint[0]=0;
  qminute_offint[0]=0;
      
    for(byte i=0;i<numq;i++)
      {
        qhour_onint[i]=qhour_onint[i+1];
        qminute_onint[i]=qminute_onint[i+1];
        qhour_offint[i]=qhour_offint[i+1];
        qminute_offint[i]=qminute_offint[i+1];
      }
    numq--;
}


