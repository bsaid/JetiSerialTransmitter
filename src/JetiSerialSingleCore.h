#include <Arduino.h>

#define SERIAL_9N1 0x01
#define SERIAL_9N2 0x02
#define SERIAL_9E1 0x11
#define SERIAL_9E2 0x12
#define SERIAL_9O1 0x21
#define SERIAL_9O2 0x22

class JETISerial {
    uint8_t m_pin = 19;
    uint16_t m_delay = 0;
    uint8_t m_parity = 0;
    uint8_t m_stopBits = 0;
public:
    void begin(uint16_t speed, uint8_t pin = 19, uint8_t config = SERIAL_9O2) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        m_pin = pin;
        m_delay = 1000000 / speed; // awful
        m_stopBits = config & 0x01;
        m_parity = (config & 0xF0) >> 4;
    }

    /**
     * @brief Adds a byte of data to TX queue.
     * 
     * @param ch data byte
     * @param bit9 9th bit of the data
     */
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
        if(m_parity == 1) {
            digitalWrite(m_pin, parity%2==1); // even parity
        }
        if(m_parity == 2) {
            digitalWrite(m_pin, parity%2==0); // odd parity
        }
        delayMicroseconds(m_delay);
        digitalWrite(m_pin, HIGH); // 1 - 2 stop bits
        delayMicroseconds(m_stopBits*m_delay);
    }
};
