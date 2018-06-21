#include <EEPROM.h>
#ifdef DEBUG
#include <TinyDebugSerial.h>
#endif
#include <Interrupted.h>

#define SAMPLES		30	// number of samples to smooth for dark level
#define THRESHOLD	80	// default dark level
#define ON_TIME		120	// how long to stay on for with no movement
#define HOLD_TIME	2000	// how long to hold button to store dark level

#define LDR	A1
#define LED	0
#define PIR	1
#define TIMER	2
#define FADER	3
#define BUTTON	4

#define FADE_ON		0
#define FADE_OFF	255

#ifdef DEBUG
TinyDebugSerial serial;
#endif
Analog ldr(LDR);
Port portb;
Pin button(BUTTON, portb, HIGH);
Pin pir(PIR, portb);
Watchdog timer(TIMER, 1);
Delay fader(FADER, 20);
Devices devices;

unsigned threshold = THRESHOLD, smoothed = threshold, ontime = 0;
byte v = FADE_OFF;
bool on = false;
long downtime;

unsigned sample(int curr) {
	static int samples[SAMPLES], pos = 0;
	static long total = 0, ns = 0;

	if (ns < SAMPLES) ns++;
	total += curr - samples[pos];
	samples[pos++] = curr;
	if (pos == SAMPLES) pos = 0;
	return (unsigned)(total / ns);
}

void setup() {
	pinMode(PIR, INPUT);
	pinMode(BUTTON, INPUT_PULLUP);
	pinMode(LED, OUTPUT);

	devices.add(pir);
	devices.add(button);
	devices.add(ldr);
	devices.add(timer);
	devices.add(fader);
	devices.begin();

	uint8_t t = EEPROM[0];
	unsigned d = 1000;
	if (t != 0xff) {
		d = 200;
		threshold = 4*t;
	}
	
	// enable PWM on PB0 (OCR0A)
	TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);
	TCCR0B = _BV(CS01);

	// flash at startup
	OCR0A = FADE_ON;
	delay(d);
	OCR0A = FADE_OFF;

	timer.enable();

#ifdef DEBUG
	serial.begin(115200);
#endif
}

static bool fading() {
	return (on && v != FADE_ON) || (!on && v != FADE_OFF);
}

void loop() {
#ifdef DEBUG
	serial.print("threshold=");
	serial.print(threshold);
	serial.print(" smoothed=");
	serial.print(smoothed);
	serial.print(" pir=");
	serial.print(pir.is_on());
	serial.print(" button=");
	serial.print(button.is_on());
	serial.print(" on=");
	serial.print(on);
	serial.print(" v=");
	serial.print(v);
	serial.print(" ontime=");
	serial.println(ontime);
#endif
	switch (devices.select()) {
	case LDR:
		smoothed = sample(ldr.read());
		timer.enable();
		break;
	case TIMER:
		ldr.enable();	// sample ldr every second
		if (ontime > 0)
			ontime--;
		else if (on) {
			fader.enable();	// fade out
			on = false;
		}
		break;
	case PIR:
#ifdef DEBUG
		serial.print("pir=");
		serial.println(pir.is_high());
#endif
		if (pir.is_high() && smoothed < threshold) {
			if (!on) {	// fade in
				fader.enable();
				on = true;
			}
			ontime = ON_TIME;
		}
		break;		
	case FADER:
		OCR0A = on? --v: ++v;
		fader.enable(fading());
		break;
	case BUTTON:
#ifdef DEBUG
		serial.print("button=");
		serial.println(button.is_high());
#endif
		// flip LED state
		on = !on;
		OCR0A = on? FADE_ON: FADE_OFF;
		if (button.is_low()) {
			pir.disable();
			timer.disable();
			ldr.disable();
			fader.disable();
			downtime = millis();
		} else {
			pir.enable();
			timer.enable();
			fader.enable(fading());
			threshold = smoothed + 1;
			if (millis() - downtime > HOLD_TIME)
				EEPROM[0] = threshold / 4;
		}
		break;
	}
}
