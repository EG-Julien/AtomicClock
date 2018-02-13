#include <Arduino.h>
#include <Wire.h>
#include <decodeurDCF77.h>
#include <RealTimeClockDS1307.h>
#include <Time.h>
#include <PinChangeInterrupt.h>

#define SER_Pin   8
#define RCLK_Pin  9
#define SRCLK_Pin 10
#define OE        11
#define ROE       3
#define MR        13

#define DCF77_DC  12

#define TEMP      A0
#define LIGHT     A1

#define BP1       4
#define BP2       5
#define BP3       6
#define BP4       7

#define SQW       2

#define number_of_74hc595s 12

#define numOfRegisterPins number_of_74hc595s * 8

boolean registers[numOfRegisterPins];

int LEDS[11][7] = {
        {
                1, 1, 1, 0, 1, 1, 1 // 0
        },
        {
                0, 0, 1, 0, 0, 1, 0 // 1
        },
        {
                1, 0, 1, 1, 1, 0, 0 // 2
        },
        {
                1, 0, 1, 1, 0, 1, 1 // 3
        },
        {
                0, 1, 1, 1, 0, 1, 0 // 4
        },
        {
                1, 1, 0, 1, 0, 1, 1 // 5
        },
        {
                1, 1, 0, 1, 1, 1, 1 // 6
        },
        {
                1, 0, 1, 0, 0, 1, 0 // 7
        },
        {
                1, 1, 1, 1, 1, 1, 1 // 8
        },
        {
                1, 1, 1, 1, 0, 1, 1 // 9
        },
        {
                1, 1, 1, 1, 0, 0, 0 // °
        }
};

int Display_1[7] = {96, 95, 94, 93, 92, 91, 90};
int Display_2[7] = {89, 87, 88, 87, 86, 85, 84};
int Display_3[7] = {83, 82, 81, 80, 79, 78, 77};
int Display_4[7] = {76, 75, 74, 73, 72, 71, 70};

#define SEMICONLON 69

struct Time {
    int hourFirstDigit = 0;
    int hourSecondDigit = 0;
    int minuteFirstDigit = 0;
    int minuteSecondDigit = 0;
    int secondes = 0;
};

struct DCFTime {
    int year = 0;
    int mounth = 0;
    int day = 0;
    int hours = 0;
    int minutes = 0;
    int secondes = 0;
};

Time mainTime;
DCFTime DcfTime;

boolean SEM = false;

void clearRegisters(void);
void resetRegisters(void);
void writeRegisters(void);
void setRegisterPin(int index, int value);

void buttonOne(void);
void buttonTwo(void);
void buttonThree(void);
void buttonFour(void);

void displayTemp(void);

void refreshTime(void);
void Serial_print99(uint8_t nombre);
void Serial_printDCF77(void);

void setup() {

    Serial.begin(115200);
    RTC.start();

    RTC.sqwEnable(RTC.SQW_1Hz);
    RTC.switchTo24h();

    attachInterrupt(digitalPinToInterrupt(SQW), refreshTime, RISING);

    attachPCINT(digitalPinToPCINT(BP1), buttonOne,   FALLING);
    attachPCINT(digitalPinToPCINT(BP2), buttonTwo,   FALLING);
    attachPCINT(digitalPinToPCINT(BP3), buttonThree, FALLING);
    attachPCINT(digitalPinToPCINT(BP4), buttonFour,  FALLING);

    RTC.start();

    pinMode(SER_Pin,   OUTPUT);
    pinMode(RCLK_Pin,  OUTPUT);
    pinMode(SRCLK_Pin, OUTPUT);
    pinMode(MR,        OUTPUT);
    pinMode(OE,        OUTPUT);
    pinMode(ROE,       OUTPUT);
    pinMode(BP1,       INPUT_PULLUP);
    pinMode(BP2,       INPUT_PULLUP);
    pinMode(BP3,       INPUT_PULLUP);
    pinMode(BP4,       INPUT_PULLUP);
    pinMode(DCF77_DC,  INPUT);

    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = 31249;
    OCR1B = 15624;

    TCCR1B |= (1<<CS12) | (1<<WGM12);
    TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);

    resetRegisters();
    clearRegisters();
    writeRegisters();
}

void loop() {
    static uint8_t longueur = 0;

    bool trame_decodee = decodeurDCF77.traiterSignal(digitalRead(DCF77_DC), millis());

    if (trame_decodee) {
        Serial.print(' ');
        Serial_printDCF77();
        RTC.setMinutes(DcfTime.minutes);
        RTC.setHours(DcfTime.hours);
        RTC.setSeconds(DcfTime.secondes);
        RTC.setYear(DcfTime.year);
        RTC.setMonth(DcfTime.mounth);
        RTC.setDay(DcfTime.day);
    }

    if (longueur > decodeurDCF77.longueur_trame_en_cours()) {
        longueur = 0;
        Serial.println();
    }

    while (longueur < decodeurDCF77.longueur_trame_en_cours()) {
        Serial.print(decodeurDCF77.bit_trame(longueur++));
    }
}

void refreshTime(void) {
    RTC.readClock();
    int __temp;

    mainTime.secondes = RTC.getSeconds();

    __temp = RTC.getHours();
    __temp = __temp % 10;
    mainTime.hourFirstDigit  = __temp;
    mainTime.hourSecondDigit = __temp - mainTime.hourFirstDigit * 10;

    __temp = RTC.getMinutes();
    __temp = __temp % 10;
    mainTime.minuteFirstDigit  = __temp;
    mainTime.minuteSecondDigit = __temp - mainTime.minuteFirstDigit * 10;
}

ISR(TIMER_COMPA_vect) { // 500 ms
    if (mainTime.minuteSecondDigit == 5) {
        displayTemp();
    } else {
        for (int i = 0; i < 7; i++) {
            setRegisterPin(Display_1[i], LEDS[mainTime.hourFirstDigit][i]);
        }

        for (int i = 0; i < 7; i++) {
            setRegisterPin(Display_2[i], LEDS[mainTime.hourSecondDigit][i]);
        }

        for (int i = 0; i < 7; i++) {
            setRegisterPin(Display_3[i], LEDS[mainTime.minuteFirstDigit][i]);
        }

        for (int i = 0; i < 7; i++) {
            setRegisterPin(Display_4[i], LEDS[mainTime.minuteSecondDigit][i]);
        }
    }

    SEM = !SEM;
    setRegisterPin(SEMICONLON, SEM);

    setRegisterPin(mainTime.secondes, HIGH);
    setRegisterPin((((mainTime.secondes - 1) < 0) ? 59 : (mainTime.secondes - 1)), LOW);

    writeRegisters();
}

ISR(TIMER_COMPB_vect) { // 250 ms
    analogWrite(OE, 255 - (int)map(analogRead(LIGHT), 0, 1023, 0, 255));
}

void clearRegisters(){
    for(int i = numOfRegisterPins - 1; i >=  0; i--){
        registers[i] = LOW;
    }
}

void resetRegisters(void) {
    pinMode(MR, LOW);
    delayMicroseconds(100);
    pinMode(MR, HIGH);
}

void writeRegisters(){

    digitalWrite(RCLK_Pin, LOW);

    for(int i = numOfRegisterPins - 1; i >=  0; i--){
        digitalWrite(SRCLK_Pin, LOW);

        int val = registers[i];

        digitalWrite(SER_Pin, val);
        digitalWrite(SRCLK_Pin, HIGH);

    }
    digitalWrite(RCLK_Pin, HIGH);

}

void setRegisterPin(int index, int value){
    registers[index] = value;
}

void displayTemp(void) {
    double currentTemp = analogRead(TEMP) * 5.00 / 1024.00;
    int decade  = (int)currentTemp % 10;
    int unit    = (int)currentTemp - decade * 10;
    currentTemp = currentTemp - decade - unit;
    currentTemp *= 10;
    int decimal = (int)currentTemp;

    for (int i = 0; i < 7; ++i) {
        setRegisterPin(Display_1[i], LEDS[decade][i]);
    }

    for (int i = 0; i < 7; ++i) {
        setRegisterPin(Display_2[i], LEDS[unit][i]);
    }

    for (int i = 0; i < 7; ++i) {
        setRegisterPin(Display_3[i], LEDS[decimal][i]);
    }

    for (int i = 0; i < 7; ++i) {
        setRegisterPin(Display_4[i], LEDS[10][i]);
    }
}

void Serial_printDCF77() {
    switch (decodeurDCF77.joursem()) {
        case 0 : Serial.println("(vide)"); return;
        case 1 : Serial.print("Lundi"); break;
        case 2 : Serial.print("Mardi"); break;
        case 3 : Serial.print("Mercredi"); break;
        case 4 : Serial.print("Jeudi"); break;
        case 5 : Serial.print("Vendredi"); break;
        case 6 : Serial.print("Samedi"); break;
        case 7 : Serial.print("Dimanche"); break;
    }
    Serial.print(' ');
    Serial_print99(decodeurDCF77.jour());
    Serial.print('/');
    Serial_print99(decodeurDCF77.mois());
    Serial.print("/20");
    Serial_print99(decodeurDCF77.annee());
    Serial.print(' ');
    Serial_print99(decodeurDCF77.heure());
    Serial.print(':');
    Serial_print99(decodeurDCF77.minute());
    Serial.print(' ');
    if (decodeurDCF77.heure_ete()) {
        Serial.print("(heure d'ete)");
    } else {
        Serial.print("(heure d'hiver)");
    }

    DcfTime.year     = decodeurDCF77.annee() + 2000;
    DcfTime.mounth   = decodeurDCF77.mois();
    DcfTime.day      = decodeurDCF77.jour();
    DcfTime.minutes  = decodeurDCF77.minute();
    DcfTime.hours    = decodeurDCF77.heure();
    DcfTime.secondes = 0;
}

void Serial_print99(uint8_t nombre) {
    if (nombre < 10) Serial.print('0');
    Serial.print(nombre);
}

void buttonOne(void) {
    mainTime.hourFirstDigit++;
    RTC.setHours(mainTime.hourFirstDigit * 10 + mainTime.hourSecondDigit);
}

void buttonTwo(void) {
    mainTime.hourSecondDigit++;
    RTC.setHours(mainTime.hourFirstDigit * 10 + mainTime.hourSecondDigit);
}

void buttonThree(void) {
    mainTime.minuteFirstDigit++;
    RTC.setMinutes(mainTime.minuteFirstDigit * 10 + mainTime.minuteSecondDigit);
}

void buttonFour(void) {
    mainTime.minuteSecondDigit++;
    RTC.setMinutes(mainTime.minuteFirstDigit * 10 + mainTime.minuteSecondDigit);
}