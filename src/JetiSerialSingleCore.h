#include <Arduino.h>

class JETISerial {
    uint8_t m_pin = 19;
    uint16_t m_delay = 0;
public:
    void begin(uint16_t speed, uint8_t pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        m_pin = pin;
        m_delay = 1000000 / speed; // awful
    }

    void send(uint8_t ch, uint8_t bit9) {
        digitalWrite(m_pin, LOW); // start
        delayMicroseconds(m_delay);
        uint8_t parity = 0;
        for(int i=0; i<8; i++) {
            parity += bitRead(ch, i);
            digitalWrite(m_pin, bitRead(ch, i)); // data
            delayMicroseconds(m_delay);
        }
        parity += bitRead(bit9, 0);
        digitalWrite(m_pin, bit9); // 9th bit
        delayMicroseconds(m_delay);
        digitalWrite(m_pin, parity%2==0); // odd parity
        delayMicroseconds(m_delay);
        digitalWrite(m_pin, HIGH); // 2 stop bits
        delayMicroseconds(2*m_delay);
    }
};
