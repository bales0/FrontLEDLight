/*
 * A demo of a simple AceButton used to handle the events from one button.
 * Similar to HelloButton with some additions;
 * - more comments
 * - prints out the button events to the Serial monitor
 * - enables the all button events, including LongPress and RepeatPress
 * - suppresses lower-level events when higher-level events are detected
 *   (e.g. Clicked suppressed Released, DoubleClicked suppresses the
 *   second Clicked, LongPressed suppressed the Released, etc.)
 */

#include <AceButton.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include "systemstatus.h"
using namespace ace_button;

// The pin number attached to the button.
const int BUTTON_PIN = 0;
bool led = 0;
const int ledVR = 1;
const int ledVG = 2;
unsigned long sleepMillis = millis();
int sensorValue = 4000;
const int plna = 3900;
const int polovina = 3600;
// zeslabuje pri 3400mV, puvodni plna 3800mV - 90 minut, 3800mV 90 minut
const int LED_PIN = 4;
bool power = 0;
const int lowpower = 50;

// One button wired to the pin at BUTTON_PIN. Automatically uses the default
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton button(BUTTON_PIN);

void handleEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  wdt_disable();
  // ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0); // enable ADC, prescaler 8
  // ADMUX = _BV(MUX3) | _BV(MUX2); // pro ATTiny85: měření Vbg (1.1V)
  pinMode(3, OUTPUT);
  digitalWrite(3, 0);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  pinMode(ledVR, OUTPUT);
  digitalWrite(ledVR, 1);
  pinMode(ledVG, OUTPUT);
  digitalWrite(ledVG, 1);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button uses the built-in pull up register.
// Configure the ButtonConfig with the event handler, and enable all higher
// level events.
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}

void loop() {
  // Should be called every 20ms or faster for the default debouncing time
  // of ~50ms.

sensorValue = SystemStatus().getVCC();
if (power == 1)
{
sleepMillis = millis();
if (led == 0) {
  analogWrite(LED_PIN, lowpower);
}
else {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 1);
}

if (sensorValue >= plna)
{
 digitalWrite(ledVG, 0);
 digitalWrite(ledVR, 1);
}
else if (sensorValue < plna && sensorValue > polovina)
{
 digitalWrite(ledVG, 0);
 digitalWrite(ledVR, 0);
}
else // (sensorValue <= polovina)
{
 digitalWrite(ledVG, 1);
 digitalWrite(ledVR, 0);
}
}

else
 {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  digitalWrite(ledVG, 1);
  digitalWrite(ledVR, 1);
  if (millis() - sleepMillis >= 5000)
  {
   sleepMillis = millis();
   sleep();
  }
 }

button.check();

}

// The event handler for the button.
void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t buttonState) {

  // Control the LED only for the Pressed and Released events.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  
  switch (eventType) {
	case AceButton::kEventClicked:
      led = !led;
      break;
    case AceButton::kEventLongPressed:
      power = !power;
      break;
  }

}

void sleep()
{
  cli();
  GIMSK |= (1 << PCIE);     // povol pin change interrupt
  PCMSK |= (1 << PCINT0);   // povol interrupt na PB0
  ADCSRA &= ~(1 << ADEN);  // vypnutí ADC
  power_timer0_disable();
  power_timer1_disable();
  power_all_disable();     // vypnutí všech periferií (podle potřeby)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  MCUCR |= (1 << BODS) | (1 << BODSE);  // povol změnu BOD
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS); // vypnutí BOD
  sleep_enable();
  sei(); //first instruction after SEI is guaranteed to execute before any interrupt
  sleep_cpu();

  sleep_disable();
  cli();
  power_all_enable();
  ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0); // enable ADC, prescaler 8
  ADMUX = _BV(MUX3) | _BV(MUX2); // pro ATTiny85: měření Vbg (1.1V)
  sei();
  sleepMillis = millis();
}

ISR(PCINT0_vect)
{
PCMSK &= ~(1 << PCINT0);
}
