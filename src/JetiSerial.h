#include <Arduino.h>

uint8_t JETISerial_pin = 19;
uint16_t JETISerial_delay = 0;
QueueHandle_t JETISerial_queue;
uint8_t JETISerial_parity = 0;
uint8_t JETISerial_stopBits = 0;

#define SERIAL_9N1 0x01
#define SERIAL_9N2 0x02
#define SERIAL_9E1 0x11
#define SERIAL_9E2 0x12
#define SERIAL_9O1 0x21
#define SERIAL_9O2 0x22

class JETISerial {
    TaskHandle_t jetiHandle = NULL;

    void addToQueue(uint16_t ch) {
        xQueueSend(JETISerial_queue, &ch, portMAX_DELAY);
    }

    static void sendFromQueue(uint16_t data) {
        uint8_t bit9 = data >> 8;
        uint8_t ch = data & 0xFF;
        digitalWrite(JETISerial_pin, LOW); // start
        delayMicroseconds(JETISerial_delay);
        uint8_t parity = 0;
        for(int i=0; i<8; i++) {
            parity += bitRead(ch, i);
            digitalWrite(JETISerial_pin, bitRead(ch, i)); // data
            delayMicroseconds(JETISerial_delay);
        }
        parity += bitRead(bit9, 0);
        digitalWrite(JETISerial_pin, bit9); // 9th bit
        delayMicroseconds(JETISerial_delay);
        if(JETISerial_parity == 1) {
            digitalWrite(JETISerial_pin, parity%2==1); // even parity
        }
        if(JETISerial_parity == 2) {
            digitalWrite(JETISerial_pin, parity%2==0); // odd parity
        }
        delayMicroseconds(JETISerial_delay);
        digitalWrite(JETISerial_pin, HIGH); // 1 - 2 stop bits
        delayMicroseconds(JETISerial_stopBits*JETISerial_delay);
    }
public:
    void begin(uint16_t speed, uint8_t pin = 19, uint8_t config = SERIAL_9O2) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        JETISerial_pin = pin;
        JETISerial_delay = 1000000 / speed; // skarede
        JETISerial_stopBits = config & 0x01;
        JETISerial_parity = (config & 0xF0) >> 4;

        JETISerial_queue = xQueueCreate( 512, sizeof(uint16_t) );
        xTaskCreatePinnedToCore(this->vTaskCode, "JetiSerial", 10000, NULL, 2, &jetiHandle, 0);
    }

    ~JETISerial() {
        vTaskDelete(jetiHandle);
    }

    /**
     * @brief Adds a byte of data to TX queue.
     * 
     * @param ch data byte
     * @param bit9 9th bit of the data
     */
    void send(uint8_t ch, uint8_t bit9) {
        addToQueue(bit9*0x100+ch); // awful
    }

    /**
     * @brief Returns the number of bytes in the TX queue.
     * 
     * @return uint16_t number of bytes in the TX queue.
     */
    uint16_t available() {
        return uxQueueMessagesWaiting(JETISerial_queue);
    }

    /**
     * @brief Waits until the TX is not empty.
     * 
     */
    void flush() {
        while(available()) {
            vTaskDelay(1);
            yield();
        }
    }

    static void vTaskCode( void * pvParameters )
    {
        for(;;) {
            while(uxQueueMessagesWaiting(JETISerial_queue) > 0) {
                uint16_t ch;
                xQueueReceive(JETISerial_queue, &ch, portMAX_DELAY);
                sendFromQueue(ch);
            }
            vTaskDelay(1);
            yield();
        }
    }
};
