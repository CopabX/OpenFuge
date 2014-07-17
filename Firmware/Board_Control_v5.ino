#include <LiquidCrystal.h>;

int encA=9;
int encB=10;
int encPos=0;
int encPLast=LOW;
int n=LOW;
int sel=0;
int lsel=0;;
int temp;
boolean pressed=false;
boolean ipowerup=true;
int pB;
int pA;

int timer=300;
int past;
int tick;
double rpm=0;
int pastrpm;
double rpmtime=0;
int avgrpm;
double gforce;

unsigned long zero=0;
boolean ran=false;
boolean powerup=false;

int sigcnt=0;

//PID
double kp=.008;
double ki=0.55;
double kd=0.006;

//RPM variables
int tRPM=7500; //target RPM
double rpmtimer=0; //time interval from HIGH-->LOW-->HIGH when the IR initially reads LOW
int signal=1540;
int adjust=0;
int maxRPM=9000;
int battPercent=0;

int preverror=0;
int accerror=0;
int error=0;

//Motor Vars
int powercnt=0;
int lowerlim=1600;

//Battery Vars
int cellcnt=4;
double voltage=0;
double lvc = 11.2;//Low voltage cuttoff
boolean battend=false;

int safetypin=8;


char* menu[] = {"START!", "RPM:", "Time:"};

LiquidCrystal lcd(0, 1, 2, 3, 4, 5);

void setup()
{
  pinMode(encA,INPUT);
  pinMode(encB,INPUT);
  pinMode(23,INPUT); //button
  pinMode(safetypin,INPUT);//safety
  pinMode(21,INPUT);//cell count (HIGH=5 cell)
  pinMode(20,INPUT);//voltage read
  
  
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);


  lcd.begin(16, 2);
  lcd.setCursor(1,0);//column, row
  lcd.print(menu[0]);
  lcd.setCursor(1,1);
  lcd.print(menu[1]);
}

void updateBatt()
{
  voltage=(analogRead(20)/1023.0)*55.0;
}
  

void loop()
{  
  if(ipowerup==true)
  {
    lcd.clear();
    lcd.print("INITIALIZING");
    zero=millis();
    lcd.setCursor(0,1);
    lcd.print("PLEASE WAIT");
    powerup=false;
    while(((millis()-zero)/1000)<1)
    {
      if(powerup==false)
      {
        for(int i=0;i<10;i++)
        {
         // for(int j=0;j<3;j++)
         // {
            digitalWrite(6, HIGH);
            delayMicroseconds(1500+(i*50));
            digitalWrite(6, LOW);
            delay(18);
            delayMicroseconds(500-(i*50));
        //  }
        }
        powerup=true;
      }
      else
      { 
        if((timer-((millis()-zero)/1000))<=1)
        {
          digitalWrite(6, HIGH);
          delayMicroseconds(1500);
          digitalWrite(6, LOW);
          delay(18);
          delayMicroseconds(500);
        }
        else
        {
          digitalWrite(6, HIGH);
          delayMicroseconds(2000);
          digitalWrite(6, LOW);
          delay(18);
        }
      }
    }
    
    for(int j=0;j<50;j++)
    {
      digitalWrite(6, HIGH);
      delayMicroseconds(1500);
      digitalWrite(6, LOW);
      delay(18);
      delayMicroseconds(500);
    }
    
    if(analogRead(21)>=900)
    {
      cellcnt=5;//5 LiPO4 cells
      maxRPM=9000;
    }
    else
    {
      cellcnt=4;//4 cells
      maxRPM=9000;
    }
    
    ipowerup=false;
  }
  else
  {
    if((sigcnt==740)||(sigcnt==0))
    {
      digitalWrite(6, HIGH);
      delayMicroseconds(1500);
      digitalWrite(6, LOW);
      sigcnt=0;
    }
    else
    {
      delayMicroseconds(25);
      sigcnt++;
    }
  }
  
  
  n=digitalRead(encA);
  if ((encPLast == LOW) && (n == HIGH)) 
  {
     if((digitalRead(encB) == HIGH)&&(pB==HIGH))
     {
       if(sel!=0)
         sel--;
     } 
     else 
     {
       if(sel<2)
         sel++;
     }
  }
  encPLast = n;
  pB=digitalRead(encB);
  
  if(digitalRead(23)==LOW)
  {
    pressed=true;
    while(digitalRead(23)==LOW)
      delay(30);
  }
  
  
  if(lsel!=sel)
    lcd.clear();
  
  if(pressed==false)
  {
    updateBatt();
    if(sel<2)
    {
      lcd.setCursor(1,0);
      lcd.print(menu[0]);     
      if(voltage<=lvc)
      {
        lcd.print(" LOW BATT");
      }
      else if(digitalRead(safetypin)==LOW)
      {
        lcd.print(" LID OPEN");
      }
      else if(digitalRead(safetypin)==HIGH)
      {
        lcd.print("         ");
      }
      
      lcd.setCursor(1,1);
      lcd.print(menu[1]);
      lcd.print(tRPM);
      
      lcd.setCursor(0,sel);
      lcd.print(">");
    }
    else
    {
      lcd.setCursor(1,0);
      lcd.print(menu[2]);
      lcd.print(timer);
      lcd.setCursor(1,1);
      lcd.print("^");
      
      lcd.setCursor(0,0);
      lcd.print(">");
    }
  }
  else
  {
    pressed=false;
    delay(200);
    lcd.clear();
    
    switch(sel)
    {
      case 0:
        break;
      case 1:
        tick=tRPM;
        break;
      case 2:
        tick=timer;
        break;
    }
    
    sigcnt=0;
    
    while((digitalRead(23)==HIGH)&&(ran==false))
    { 
      switch(sel)
      {
        case 0: //RUN
          updateBatt();
          if(voltage>lvc)
          {
            lcd.print("RUNNING");
            zero=millis();
            lcd.setCursor(0,1);
            powerup=false;
           
            while((digitalRead(safetypin)==HIGH)&&((((millis()-zero)/1000))<timer)&&(digitalRead(23)==HIGH)&&((((millis()-zero)/1000)<10)||(avgrpm>4300)))
            {
              if(powerup==false)
              {
                for(int i=0;i<8;i++)
                {
                 // for(int j=0;j<3;j++)
                 // {
                    digitalWrite(6, HIGH);
                    delayMicroseconds(1500+(i*50));
                    digitalWrite(6, LOW);
                    delay(18);
                    delayMicroseconds(500-(i*50));
                //  }
                }
                powerup=true;
              }
              else
              { 
                if((timer-((millis()-zero)/1000))<=2)
                {
                  digitalWrite(6, HIGH);
                  delayMicroseconds(1500);
                  digitalWrite(6, LOW);
                  delay(18);
                  delayMicroseconds(500);
                }
                else
                {
                  if(avgrpm<3000)
                  {
                    digitalWrite(6, HIGH);
                    delayMicroseconds(2000);
                    digitalWrite(6, LOW);
                    delayMicroseconds(18000);
                  }
                  else
                  {
                    digitalWrite(6, HIGH);
                    delayMicroseconds(signal);
                    digitalWrite(6, LOW);
                    delayMicroseconds(20000-signal);
                  }
                }
              }
              
              updateBatt();
              pastrpm=rpm;
              rpmtime=pulseIn(7,HIGH);
              rpmtime+=pulseIn(7,LOW);
              rpmtime/=1000000;
              rpm=1/rpmtime;
              rpm*=60;
              avgrpm=(avgrpm+rpm+pastrpm)/3;
                
              gforce=72.8*pow(avgrpm/1000,2);
              
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("RUNNING");
              lcd.setCursor(0,1);
              
              lcd.print(timer-((millis()-zero)/1000));
              
              preverror=error; //Error turnover
              error=avgrpm-tRPM; //Calc error (actual-target) 
              adjust=error*kp; //Adds P term
              accerror+=adjust*ki; //Adds to accumulated error (I term)
              adjust+=(error-preverror)*kd; //adds D term
              
              signal=signal-adjust;
              
              if(signal<=lowerlim)//Prevents stalling
                signal=lowerlim;
              if(signal>2000)//Signal max is 2ms HIGH
                signal=2000;
              
              if(tRPM-300>avgrpm)
                powercnt++;
              else
                powercnt=0;
              
              lcd.setCursor(6,1);
              lcd.print("RPM:");
              lcd.print(avgrpm);
              
              lcd.setCursor(9,0);     
              if(powercnt>70)
              {
                lcd.setCursor(8,0);
                lcd.print("LOW BATT");
              }
              else
              {
                lcd.print("G:");
                lcd.print((int)gforce);
              }
            }
            for(int j=0;j<50;j++)
            {
              digitalWrite(6, HIGH);
              delayMicroseconds(1500);
              digitalWrite(6, LOW);
              delay(18);
              delayMicroseconds(500);
            }    
          }
          ran=true;
          break;
          
        case 1: //Set target RPM value
          lcd.setCursor(0,0);
          lcd.print("RPM");
          past=tick;
          n=digitalRead(encA);
          if((encPLast == LOW) && (n == HIGH)) 
          {
             if((digitalRead(encB) == HIGH)&&(pB==HIGH)) 
             {
               if(tRPM!=4500)
                 tRPM-=500;
             } 
             else 
             {
               if(tRPM<maxRPM)
                 tRPM+=500;
             }
          }
          encPLast = n;
          pB=digitalRead(encB);
          lcd.setCursor(0,1);
          lcd.print(tRPM);
          tick=tRPM;
          break;
          
        case 2: //Set run time
          lcd.setCursor(0,0);
          lcd.print("Time?");
          tick=timer/15;
          past=tick;
          n=digitalRead(encA);
          if ((encPLast == LOW) && (n == HIGH)) 
          {
             if((digitalRead(encB) == HIGH)&&(pB==HIGH)) 
             {
               if(timer!=0)
                 tick-=1;
             } 
             else 
             {
               if(timer<900)
                 tick+=1;
             }
          }
          encPLast = n;
          pB=digitalRead(encB);
          lcd.setCursor(0,1);
          lcd.print("Sec:");
          lcd.setCursor(4,1);
          lcd.print(timer);
          
          lcd.setCursor(8,1);
          lcd.print("Min:");
          lcd.print((double)timer/60);
          
          timer=tick*15;
          break;
      }
      if(past!=tick)
        lcd.clear();  
     
      if((sel==1)||(sel==2))  
      {
        if(sigcnt==370)
        {
          digitalWrite(6, HIGH);
          delayMicroseconds(1500);
          digitalWrite(6, LOW);
          sigcnt=0;
        }
        else
        {
          delayMicroseconds(50);
          sigcnt++;
        }
      }
    }
    if(digitalRead(23)==LOW)
    {
      while(digitalRead(23)==LOW)
        delay(30);
    }
 
    lcd.clear();
    sigcnt=0;
  }
  ran=false;
  adjust=0;
  powerup=false;
  error=0;
  preverror=0;  
  lsel=sel;
} 
