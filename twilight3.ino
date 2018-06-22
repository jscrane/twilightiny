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
#define BUTTON	4

#ifdef DEBUG
TinyDebugSerial serial;
#endif
Analog ldr(LDR);
Port portb;
Pin button(BUTTON, portb, HIGH);
Pin pir(PIR, portb);
Delay timer(TIMER, 1000);
Devices devices;

unsigned threshold = THRESHOLD, smoothed = threshold, left = 0;
long down;

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
#ifdef DEBUG
	serial.begin(115200);
	serial.println("starting");
#endif

	pinMode(PIR, INPUT);
	pinMode(BUTTON, INPUT_PULLUP);
	pinMode(LED, OUTPUT);

	devices.add(pir);
	devices.add(button);
	devices.add(ldr);
	devices.add(timer);
	devices.begin();

	uint8_t t = EEPROM[0];
	unsigned d = 1000;
	if (t != 0xff) {
		d = 200;
		threshold = 4*t;
	}
	
	digitalWrite(LED, HIGH);
	delay(d);
	digitalWrite(LED, LOW);

	timer.enable();
}

void loop() {
	static bool on;

	switch (devices.select()) {
	case LDR:
		smoothed = sample(ldr.read());
		timer.enable();
		break;
	case TIMER:
#ifdef DEBUG
		serial.print("threshold=");
		serial.print(threshold);
		serial.print(" smoothed=");
		serial.print(smoothed);
		serial.print(" pir=");
		serial.print(pir.is_on());
		serial.print(" button=");
		serial.print(button.is_on());
		serial.print(" led=");
		serial.print(on);
		serial.print(" left=");
		serial.print(left);
		serial.print(" t=");
		serial.println(millis());
#endif
		ldr.enable();	// sample ldr every second
		if (left > 0)
			left--;
		else if (on) {
			digitalWrite(LED, LOW);
			on = false;
		}
		break;
	case PIR:
		if (pir.is_high() && smoothed < threshold) {
			digitalWrite(LED, HIGH);
			left = ON_TIME;
			on = true;
		}
		break;
	case BUTTON:
		if (button.is_low()) {
			digitalWrite(LED, HIGH);
			left = ON_TIME;
			on = true;
			down = millis();
		} else if (millis() - down > HOLD_TIME) {
			threshold = smoothed + 1;
			EEPROM[0] = threshold / 4;
		}
		break;
	}
}
