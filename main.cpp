#include "mbed.h"
#include "uLCD_4DGL.h"

DigitalIn D_3(D3);
DigitalIn D_5(D5);
DigitalIn D_6(D6);
AnalogIn Ain(A0);
AnalogOut Aout(D7);
DigitalOut led(LED3);
uLCD_4DGL uLCD(D1, D0, D2);
Thread dac;

// make the button work once a time
int one_pulse(int x, int last_in);
// display the frequency on the LCD 
void LCD(int freq);
// transfer the digital data to analog   
void DAC_f(int *f);
// transfer the analog data to digital                
void ADC_f(void);                   

int main(void)
{
    int last_D3 = 0;
    int last_D5 = 0;
    int last_D7 = 0;
    int up = 0;
    int down = 0; 
    int sel = 0;
    int dis_freq = 0;
    int use_freq = 0;
    led = 0;

    // let the DAC be a thread to make it independent to the main function
    dac.start(callback(&DAC_f, &use_freq));

    while(1)
    {
        up = one_pulse(D_3, last_D3);
        last_D3 = D_3;

        down = one_pulse(D_5, last_D5);
        last_D5 = D_5;

        sel = one_pulse(D_6, last_D7);
        last_D7 = D_6;

        if(up)
        {
            // when the button is pressed, the led will change
            led = !led;
            // if frequency < 10, make it increase 1Hz once a time
            if (dis_freq < 10)
                dis_freq += 1;
            // if frequency < 10, make it increase 10Hz once a time     
            else if (dis_freq < 100)
                dis_freq += 10;
            // if frequency < 10, make it increase 100Hz once a time
            else if (dis_freq != 300)
                dis_freq += 100;
            // if frequency = 300, make it up to 3000Hz    
            else
                dis_freq = 3000;         
        }
        else if(down)
        {
            led = !led;
            if (dis_freq == 0)
                dis_freq = 0;
            else if (dis_freq <= 10)
                dis_freq -= 1;
            else if (dis_freq <= 100)
                dis_freq -= 10;
            else if (dis_freq <= 300)
                dis_freq -= 100;
            // if frequency = 3000, make it down to 0Hz 
            else
                 dis_freq = 0;                                
        }
        // display the frequency now
        LCD(dis_freq);

        if (sel) 
        {
            // select the frequency
            use_freq = dis_freq;
            // start the ADC
            ADC_f();
        } 
    }
}

int one_pulse(int x, int last_in)
{
    if (x)
        if (x == last_in)
            return 0;
        else
            return 1;
    else
        return 0;            
}

void LCD(int freq)
{
    uLCD.text_width(4);
    uLCD.text_height(4);
    uLCD.locate(0,1);
    uLCD.printf("%4d", freq);
    uLCD.text_width(4);
    uLCD.text_height(4);    
    uLCD.locate(2,2);
    uLCD.printf("Hz");
}

void DAC_f(int *f)
{
    while(1)
    {
        if (*f != 0)
            // the number 13000 is come from that I use timer to test time and make the time meet the period I want
            // because the time of for loop may not linear, so there is little error
            for (int i = 0; i < 13000 / *f ; i++)
            {
                if (i < ( 2.0 / (5.0 * *f)) * 13000)
                    // the fucnction of wave
                    Aout = ((3.0 / (2.0 / (5.0 * *f))) * (i / 13000.0)) / 3.3;
                else
                    // the fucnction of wave
                    Aout = (3.0 - 5 * *f * (i / 13000.0 - 2.0 / (5 * *f))) / 3.3;
            }
        else
            Aout = 0.0f;    
    }   
}

void ADC_f()
{
    float ADCdata[600];
    // the sample frequency I use here is not 600 / 0.6, need to use timer to test the actual time
    // and the frequency is 600 / actual time 
    for (int i = 0; i < 600; i++)
    {
        ADCdata[i] = Ain;         
        ThisThread::sleep_for(1ms);
    }

    for (int i = 0; i < 600; i++)
    {
        printf("%f\r\n", ADCdata[i]);
    }
}