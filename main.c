#include <math.h>
#include <stdio.h>
#include <msp430.h>

#define SERVER "api.thingspeak.com" //Thingspeak web address
#define PORT 80                     // Thingspeak port #
#define API "O66EMWU2XZ8P7WGC"      // Thingspeak Write Key
#define IP "184.106.153.149"        // IP address of Thingspeak

volatile float tempRaw, lightRaw, humidityRaw;
volatile float temp, light, humidity;
char result[100];
volatile long sample[100];
void uart_init(void);
void ConfigClocks(void);
void ConfigureAdc_light();
void ConfigureAdc_temp();
void ConfigureAdc_humidity();
void port_init();
void itoa(int value, char* str, int base);
void strreverse(char* begin, char* end);

unsigned int adc[10];
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Set up ports, UART, and configure the input values to that of the correct measurement readings
    ConfigClocks();
    port_init();
    uart_init();
    ConfigureAdc_light(); // Configure the ADC for light values
    ConfigureAdc_temp(); // Configure the ADC for temperature values
    ConfigureAdc_humidity(); // Configure the ADC for humidity values

    __delay_cycles(5);    // set up adequate delay

    // UART TX and RX pin set up
    UCA0CTL1 |= UCSWRST;  // enable reset
    UCA0CTL1 |= UCSSEL_2; // SMCLK

    // USCI register set up
    UCA0BR0 = 8; UCA0BR1 = 0;
    UCA0MCTL |= UCBRS_6;
    //UCA0IE |= UCRXIE;           // Enable USCI_A0 RX interrupt
    _enable_interrupts(); // enabling of interrupts

    while(1){

        ConfigureAdc_temp(); // Call the configuration for ADC temp value
        ConfigureAdc_light(); // Call the configuration for ADC light value
        ConfigureAdc_humidity(); // Call the configuration for ADC humidity value

        ADC10CTL0 |= ENC + ADC10SC + MSC;

        while ((ADC10CTL1 & ADC10IFG) == 0);
        ADC10CTL0 |= ENC + ADC10SC;
        ADC10SA = (unsigned int)adc;

        tempRaw = 1023/adc[1]; // Convert ADC temperature output to voltage
        float tempLog = log((tempRaw)-1); //obtain log of tempRaw * 10^-4
        temp = 1/(0.003354 + (0.0002958) * tempLog); // obtain actual reading of temperature
        lightRaw = adc[2]*3.3/1023; // Convert ADC light output to voltage
        float lightOut = (lightRaw*0.0048828125); //obtain ADC light * 0.0048828
        light = 500/lightOut; // obtain actual reading of light
        humidityRaw = adc[3]*3.3/1023; // Convert ADC humidity output to voltage
        float humidityOut = humidityRaw*0.0879765; //obtain ADC humidity * 0.0879765
        humidity = humidityOut*1000 - 1000; // obtain actual reading of humidity

        int acount =0;
        result[acount] = 'A';

        itoa((adc[1]),result,10); // conversion of temperature
        while(result[acount]!='\0')
        {

            while((IFG2 & UCA0TXIFG)==0);

            UCA0TXBUF = result[acount++] ;                   //Transmit the received data.
        }

        _delay_cycles(150000*15);

        itoa((adc[2]),result,10); // conversion of light
        acount =0;
        while(result[acount]!='\0')
        {
            while((IFG2 & UCA0TXIFG)==0);                  //Wait Unitl the UART transmitter is ready
            UCA0TXBUF = result[acount++] ;                   //Transmit the received data.
        }
        _delay_cycles(150000*15);

        itoa((adc[3]),result,10); // conversion of humidity
        acount =0;
        while(result[acount]!='\0')
        {
            while((IFG2 & UCA0TXIFG)==0);                  //Wait Unitl the UART transmitter is ready
            UCA0TXBUF = result[acount++] ;                   //Transmit the received data.
        }
        _delay_cycles(150000*15);
    }
}

void uart_init(void){
    //clock//baudrate//modulation//sw rest//TXRX interrupt
    UCA0CTL1 |= UCSWRST;                     //Disable the UART state machine
    UCA0CTL1 |= UCSSEL_3;                    //Select SMCLK as the baud rate generator source
    UCA0BR1 =0;
    UCA0BR0 = 104;                           //Produce a standard 9,600 Baud UART rate
    UCA0MCTL = 0x02;                         //Get correct value for UCA0MCTL from data table
    UCA0CTL1 &= ~UCSWRST;                    //Enable the UART state
    IE2 |= UCA0RXIE;                         //Enable the UART receiver Interrupt
}
void ConfigureAdc_light(){
    ADC10CTL1 = INCH_6 + ADC10DIV_0 + CONSEQ_3 + SHS_0; // read inputs from A0 to A6
    ADC10CTL0 = SREF_2 + ADC10SHT_2 + MSC + ADC10ON;    // using 3.3V reference
    ADC10AE0 = BIT3 + BIT0;
    __delay_cycles(5);
    ADC10DTC1 = 4;
}
void ConfigureAdc_temp(){
    ADC10CTL1 = INCH_5 + ADC10DIV_0 + CONSEQ_2; // read inputs from A0 to A6
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;    // using 3.3V reference
    __delay_cycles(5);
    ADC10CTL0 |= ENC | MSC;
}
void ConfigureAdc_humidity(){
    ADC10CTL1 = INCH_4 + ADC10DIV_0 + CONSEQ_2; // read inputs from A0 to A6
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON;    // using 3.3V reference
    __delay_cycles(5);
    ADC10DTC1 = 4;
}

void ConfigClocks(void)
{
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL3 |= LFXT1S_2;
    BCSCTL2 = 0;
}

void port_init(){
    P1SEL |= BIT1 + BIT2;            // select non-GPIO  usage for Pins 1 and 2
    P1SEL2 |= BIT1 + BIT2;           // Select UART usage of Pins 1 and 2
}

void itoa(int value, char* str, int base) {  //Function to convert the signed int to an ASCII char array

    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr=str;
    int sign;

    // Validate that base is between 2 and 35 (inlcusive)
    if (base<2 || base>35){
        *wstr='\0';
        return;
    }

    // Get magnitude and the value
    sign=value;
    if (sign < 0)
        value = -value;

    do // Perform integer-to-string conversion.
        *wstr++ = num[value%base]; //create the next number in converse by taking the modolus
    while(value/=base);  // stop when you get  a 0 for the quotient


    if(sign<0) //attach sign character, if needed
          *wstr++='-';
      *wstr='\0'; //Attach a null character at end of char array. The string is in reverse order at this point
      strreverse(str,wstr-1); // Reverse string

  }

void strreverse(char* begin, char* end)      // Function to reverse the order of the ASCII char array elements
{
    char aux;
    while(end>begin)
        aux=*end, *end--=*begin, *begin++=aux;
}


