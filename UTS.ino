#include <avr/io.h>
#include <avr/interrupt.h>

volatile bool tick100ms = false;
volatile uint8_t tickCount = 0;
volatile bool blinkState = false;

bool systemEnabled = true;
bool lastButtonStable = true;
bool lastButtonRead = true;
uint8_t debounceCount = 0;

int analogValue = 0;
uint8_t pwmValue = 0;

void setupTimer1CTC_100ms() {
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  TCNT1 = 0;

  OCR1A = 24999;

  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);

  TIMSK1 |= (1 << OCIE1A);
}

void setupPWM_Timer2_D3() {
  DDRD |= (1 << DDD3);

  TCCR2A = 0x00;
  TCCR2B = 0x00;

  TCCR2A |= (1 << COM2B1);
  TCCR2A |= (1 << WGM21) | (1 << WGM20);
  TCCR2B |= (1 << CS22);

  OCR2B = 0;
}

void setupDigitalIO() {
  DDRD &= ~(1 << DDD2);
  PORTD |= (1 << PORTD2);

  DDRD |= (1 << DDD7);
  PORTD &= ~(1 << PORTD7);
}

ISR(TIMER1_COMPA_vect) {
  tick100ms = true;
  tickCount++;

  if (tickCount >= 5) {
    tickCount = 0;
    blinkState = !blinkState;
  }
}

void setup() {
  setupDigitalIO();
  setupPWM_Timer2_D3();
  setupTimer1CTC_100ms();
  sei();
}

void loop() {
  if (tick100ms) {
    tick100ms = false;

    bool currentButton = (PIND & (1 << PIND2)) != 0;

    if (currentButton == lastButtonRead) {
      if (debounceCount < 2) debounceCount++;
    } else {
      debounceCount = 0;
    }

    if (debounceCount >= 2) {
      if (currentButton != lastButtonStable) {
        lastButtonStable = currentButton;

        if (lastButtonStable == false) {
          systemEnabled = !systemEnabled;
        }
      }
    }

    lastButtonRead = currentButton;

    if (systemEnabled) {
      analogValue = analogRead(A0);

      pwmValue = map(analogValue, 0, 1023, 0, 255);
      OCR2B = pwmValue;

      if (analogValue > 700) {
        if (blinkState) {
          PORTD |= (1 << PORTD7);
        } else {
          PORTD &= ~(1 << PORTD7);
        }
      } else {
        PORTD &= ~(1 << PORTD7);
      }

    } else {
      OCR2B = 0;
      PORTD &= ~(1 << PORTD7);
    }
  }
}