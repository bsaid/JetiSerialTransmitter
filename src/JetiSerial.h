#include <Arduino.h>

uint8_t JETISerial_pin = 19;
uint16_t JETISerial_delay = 0;
QueueHandle_t JETISerial_queue;

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
        digitalWrite(JETISerial_pin, parity%2==0); // odd parity
        delayMicroseconds(JETISerial_delay);
        digitalWrite(JETISerial_pin, HIGH); // 2 stop bits
        delayMicroseconds(2*JETISerial_delay);
    }
public:
    void begin(uint16_t speed, uint8_t pin) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        JETISerial_pin = pin;
        JETISerial_delay = 1000000 / speed; // skarede

        JETISerial_queue = xQueueCreate( 512, sizeof(uint16_t) );
        xTaskCreatePinnedToCore(this->vTaskCode, "JetiSerial", 10000, NULL, 2, &jetiHandle, 0);
    }

    ~JETISerial() {
        vTaskDelete(jetiHandle);
    }

    void send(uint8_t ch, uint8_t bit9) {
        addToQueue(bit9*0x100+ch); // awful
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
        }
    }
};

JETISerial JetiSerial;
