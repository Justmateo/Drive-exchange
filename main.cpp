#include "mbed.h"
#include <cstdio>
#include "Ultrasonic.h"
#include "HCSR04.h"

enum RICHTING{RECHTDOOR,LINKS,RECHTS,AFLEVEREN,DEFAULT};
enum EVENT{begin};
enum AFLEVEREN_MARKER{AAN,UIT};

AFLEVEREN_MARKER afleveren_marker;
RICHTING richting;
EVENT event;
Timer timer;

PinName pin_ECHO = D11; // Echo pin
PinName pin_TRIG = D12; // Trigger pin


DigitalOut led1(LED1);              //controle leds om te zien of de sensoren werken
DigitalOut led2(D2);

DigitalOut Mlinksomhoog_aflevering(D3);          //linker motor naar voren rijden int 1 en 4 hbrug2
DigitalOut Mrechtsomhoog_aflevering(D4);        //linker motor naar achter rijden in 2 en 3 hbrug 2
DigitalOut Mlinksvooruit(D5);          //linker motor naar voren rijden int4 hbrug 1
DigitalOut Mlinksachteruit(D6);        //linker motor naar achter rijden int3 hbrug 1 
DigitalOut Mrechtsachteruit(D7);         //rechter motor naar voren rijden int2 hbrug 1
DigitalOut Mrechtsvooruit(D8);       //rechter motor naar achter rijden int1 hbrug 1


InterruptIn drukkonp_1(D9, PullDown);
InterruptIn drukkonp_2(D13, PullDown); 

AnalogIn lijnsensorlinks(A0);           //input signaal lijnsensoren
AnalogIn lijnsensorrechts(A1);
AnalogIn Halsensor(A2);                 //input halsensor

PwmOut PWMrechts(D1);               //pwm output signaal rechterwiel
PwmOut PWMlinks(D6);                //pwm output signaal linkerwiel
PwmOut PWMomhoog(D10);              //pwm output signaal Omhoog/omlaag
PwmOut PWMaflever(A5);              //pwm output signaal afleverenpakket

float lijnsensor_variable_l1 = 0;   //variablen voor keuze van lijn volgen 
float lijnsensor_variable_R1 = 0;
float lijnsensor_variable_l2 = 0;
float lijnsensor_variable_R2 = 0;

float getDistance_cm();             // floats voor ultrasoonsensor
float Ultra_cm(void);

HCSR04 Ultra(pin_ECHO, pin_TRIG); // Sensor

void Stop () {
    Mlinksvooruit=1;           // welke kant draait de linker motor op
    Mlinksachteruit=0;

    PWMlinks= 0 ;           // Dutycycle instellen PWMlinks

    Mrechtsvooruit=1;
    Mrechtsachteruit=0;

    PWMrechts=0 ;
}

void Vooruit () {
    Mlinksvooruit=1;           // welke kant draait de linker motor op
    Mlinksachteruit=0;

    PWMlinks.period(0.001);   // Periode tijd instellen PWMlinks
    PWMlinks= 0.4 ;           // Dutycycle instellen PWMlinks

    Mrechtsvooruit=1;
    Mrechtsachteruit=0;

    PWMrechts.period(0.001);
    PWMrechts=0.4 ;
}

void Rechtsaf (){
    Mlinksvooruit=1;           // welke kant draait de linker motor op
    Mlinksachteruit=0;

    PWMlinks.period(0.001);   // Periode tijd instellen PWMlinks
    PWMlinks=0.7  ;           // Dutycycle instellen PWMlinks

    Mrechtsvooruit=1;
    Mrechtsachteruit=0;

    PWMrechts.period(0.001);
    PWMrechts=0.3 ;
}

void Linksaf () {
    Mlinksvooruit=1;           // welke kant draait de linker motor op
    Mlinksachteruit=0;

    PWMlinks.period(0.001);   // Periode tijd instellen PWMlinks
    PWMlinks=0.3  ;           // Dutycycle instellen PWMlinks

    Mrechtsvooruit=1;
    Mrechtsachteruit=0;

    PWMrechts.period(0.001);
    PWMrechts=0.7 ;
}
void richtingcheck (){
    lijnsensor_variable_l1 = lijnsensorlinks;
    lijnsensor_variable_R1 = lijnsensorrechts;
    lijnsensor_variable_l2 = lijnsensorlinks;
    lijnsensor_variable_R2 = lijnsensorrechts;
    printf("%.3f\n",lijnsensor_variable_l1);
    printf("%.3f\n",lijnsensor_variable_R1);
    wait_us(2000);
}

void afleveren()
{   
    //omhoog en pakket afleveren
    Mlinksomhoog_aflevering=1;           // welke kant draaien de bijde motoren op voor de aflevermeganisme.
    Mrechtsomhoog_aflevering=0;

    PWMomhoog.period(1);   // Periode tijd instellen PWM (tijd timmen voor hoe snel over doet om omhoog te gaan) 
    PWMomhoog= 0.75 ;           // Dutycycle instellen PWMomhoog

    PWMaflever.period(1);  // Periode tijd instellen PWM (tijd timmen voor hoe snel over doet om aftelevern te gaan)
    PWMaflever=0.9 ;            // Dutycycle instellen PWMafleveren

    wait_us (1000000);
    //pakket terug draaien en naar benenden
    Mlinksomhoog_aflevering=0;           // welke kant draaien de bijde motoren op voor de aflevermeganisme.
    Mrechtsomhoog_aflevering=1;

    PWMaflever.period(1);
    PWMaflever=0.9 ;

    PWMomhoog.period(1);   // Periode tijd instellen PWM (tijd timmen voor hoe snel over doet om omhoog te gaan) 
    PWMomhoog= 0.75 ;           // Dutycycle instellen PWMomhoog




}

void begin_afleveren()
{
    afleveren_marker = AAN;

}
int walk =+ 1;

int main(){
    wait_us (2000000);
    richting = RECHTDOOR;
    uint16_t dist = 0;
    
    drukkonp_1.rise(begin_afleveren);

 while(1) 
 {
dist = Ultra_cm();
wait_us(100000 - timer.read_ms());
 if (afleveren_marker == AAN)   //afleveren van pakket handmatig
 {
     Stop();
     wait_us (1000000);
     afleveren();

     afleveren_marker = UIT;
 }
 else if( dist <= 10 ) // afstand berijkt met ultrasoon sensor
 {
     Stop();
     led1 = true;
 }
 else {
 switch(richting){ 
    case RECHTDOOR:
        printf("RECHTDOOR\n");
        richtingcheck();
        if(Halsensor < 0.6475    ||  Halsensor > 0.6750)
        {
            richting = AFLEVEREN;
        } 
        else if (lijnsensor_variable_l1 < 0.3f)
        {
            richting = LINKS;
        }
        else if (lijnsensor_variable_R1 < 0.3f)
        {
            richting = RECHTS;
        }
        else if (lijnsensor_variable_l2 >= 0.3f && lijnsensor_variable_R2 >= 0.3f)
        {
          Vooruit();
          printf("VOORUIT\n");  
        }

        break;

    case LINKS:
        printf("LINKS\n");
        richtingcheck();
        if(Halsensor < 0.6475    ||  Halsensor > 0.6750)
        {
            richting = AFLEVEREN;
        } 
        else if (lijnsensor_variable_R1 < 0.3f)
        {
            richting = RECHTS;
        }    
        else if(lijnsensor_variable_l2 >= 0.3f && lijnsensor_variable_R2 >= 0.3f)
        {
            richting = RECHTDOOR;
        }
      
        Linksaf();
        printf("linksaf\n");    
        break;
    case RECHTS:
        printf("RECHTS\n");
        richtingcheck();
        if(Halsensor < 0.6475    ||  Halsensor > 0.6750)
        {
            richting = AFLEVEREN;
        } 
        else if (lijnsensor_variable_l1 < 0.3f)
        {
            richting = LINKS;
        }    
        else if(lijnsensor_variable_l2 >= 0.3f && lijnsensor_variable_R2 >= 0.3f)
        {
            richting = RECHTDOOR;
        }  
        Rechtsaf();
        printf("REChtsaf\n");
        break;
    case AFLEVEREN:
        Stop();

        if (drukkonp_1 == 1)
        {
        led1 = 1;

        afleveren();
        /*
        
        */
        }
        break;
    case DEFAULT:  
        break;{}
    }
 }
 }
}    

float Ultra_cm(void)    // Start meten van afstand
{
    Ultra.startMeasurement();

    while(!Ultra.isNewDataReady());

    return Ultra.getDistance_cm();
}