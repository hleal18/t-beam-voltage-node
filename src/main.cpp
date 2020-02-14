#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x48);
float Voltage = 0.0;

void enable_sleep()
{ //TODO implement  UBX-ACK
    do
    { //We cannot read UBX ack therefore try to sleep gps until it does not send data anymore
        Serial.println("try to sleep gps!");
        byte CFG_RST[12] = {0xb5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x66};
        delay(600);                                                                                                                  //give some time to restart //TODO wait for ack
        const byte RXM_PMREQ[16] = {0xb5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4d, 0x3b}; //power off until wakeup call
        Serial1.write(RXM_PMREQ, sizeof(RXM_PMREQ));
        unsigned long startTime = millis();
        unsigned long offTime = 1;
        Serial.println(offTime);

        while (millis() - startTime < 1000)
        { //wait for the last command to finish
            int c = Serial1.read();
            if (offTime == 1 && c == -1)
            { //check  if empty
                offTime = millis();
            }
            else if (c != -1)
            {
                offTime = 1;
            }
            if (offTime != 1 && millis() - offTime > 100)
            { //if gps chip does not send any commands for .. seconds it is sleeping
                Serial.println("sleeping gps!");
                return;
            }
        }
    } while (1);
}


void setup(void)
{
    Serial.begin(9600);
    ads.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
                                 // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    Serial1.begin(9600, SERIAL_8N1, 12, 15);
    enable_sleep();
    
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);
    ads.begin();
}

void loop(void)
{
    int16_t adc0; // we read from the ADC, we have a sixteen bit integer as a result

    adc0 = ads.readADC_SingleEnded(0);
    Voltage = (adc0 * 0.1875f) / 1000;

    // Applying conversion required, given the usage of
    // 4.2V V max batt to 3.3V voltage division.
    // Valid for GAIN of 2/3 = 6.144V
    Voltage = Voltage * 4.2 / 3.233;

    Serial.print("AIN0: ");
    Serial.print(adc0);
    Serial.print("\tVoltage: ");
    Serial.println(Voltage, 7);
    Serial.println();

    delay(1000);
}