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
Pin led(LED, portb);
Watchdog timer(TIMER, 1);
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
	pinMode(BUTTON, INPUT);
	pinMode(LED, OUTPUT);

	devices.add(led);
	devices.add(pir);
	devices.add(button);
	devices.add(ldr);
	devices.add(timer);
	devices.begin(false);

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
	serial.print(led.is_on());
	serial.print(" left=");
	serial.print(left);
	serial.print(" t=");
	serial.println(millis());
#endif
	switch (devices.select()) {
	case LDR:
		smoothed = sample(ldr.read());
		timer.enable();
		break;
	case TIMER:
		ldr.enable();	// sample ldr every second
		if (left > 0)
			left--;
		else if (led.is_on())
			digitalWrite(LED, LOW);
		break;
	case LED:
		if (led.is_on())
			left = ON_TIME;
		break;
	case PIR:
#ifdef DEBUG
		serial.print("pir=");
		serial.println(pir.is_high());
#endif
		if (pir.is_high() && smoothed < threshold)
			digitalWrite(LED, HIGH);
		break;		
	case BUTTON:
#ifdef DEBUG
		serial.print("button=");
		serial.println(button.is_high());
#endif
		if (button.is_low()) {
			digitalWrite(LED, HIGH);
			down = millis();
		} else {
			threshold = smoothed + 1;
			if (millis() - down > HOLD_TIME)
				EEPROM[0] = threshold / 4;
		}
		break;
	}
}
