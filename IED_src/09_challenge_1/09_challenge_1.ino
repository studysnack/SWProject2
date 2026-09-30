#include <stdlib.h>

// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.35    // EMA weight of new sample (range: 0 to 1)
						  // Setting EMA to 1 effectively disables EMA filter.

#define MEDIAN_N 3


class MedianArray {
private:
	int* distances;
	int* temp;

	int capacity;
	int size;
	int index;

	static int compare(const void* a, const void* b) {
		int value_a = *(const int*)a;
		int value_b = *(const int*)b;

		if (value_a < value_b)
			return -1;

		if (value_a > value_b)
			return 1;

		return 0;
	}

public:
	MedianArray(int n) {
		capacity = n;
		size = 0;
		index = 0;

		distances = new int[capacity];
		temp = new int[capacity];
	}

	~MedianArray() {
		delete[] distances;
		delete[] temp;
	}

	void add(int value) {
		distances[index] = value;

		index = (index + 1) % capacity;

		if (size < capacity)
			size++;
	}

	float median() {
		if (size == 0)
			return 0;

		for (int i = 0; i < size; i++)
			temp[i] = distances[i];

		qsort(temp, size, sizeof(int), compare);

		if (size % 2 == 1)
			return temp[size / 2];

		return (temp[size / 2 - 1] + temp[size / 2]) / 2.0;
	}
};


// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_ema = _DIST_MAX;        // EMA distance

MedianArray median_filter(MEDIAN_N);


void setup() {
	// initialize GPIO pins
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_TRIG, OUTPUT);
	pinMode(PIN_ECHO, INPUT);
	digitalWrite(PIN_TRIG, LOW);

	// initialize serial port
	Serial.begin(57600);
}

void loop() {
	float dist_raw, dist_median;

	// wait until next sampling time.
	// millis() returns the number of milliseconds since the program started.
	// will overflow after 50 days.
	if (millis() < last_sampling_time + INTERVAL)
		return;

	// get a distance reading from the USS
	dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

	// EMA filter
	dist_ema =
		_EMA_ALPHA * dist_raw +
		(1.0 - _EMA_ALPHA) * dist_ema;

	// Median filter
	median_filter.add((int)dist_raw);
	dist_median = median_filter.median();

	// output the read value to the serial port
	Serial.print("Min:");     Serial.print(_DIST_MIN);
	Serial.print(",raw:");    Serial.print(dist_raw);
	Serial.print(",ema:");    Serial.print(dist_ema);
	Serial.print(",median:"); Serial.print(dist_median);
	Serial.print(",Max:");    Serial.print(_DIST_MAX);
	Serial.println("");

	// do something here
	if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
		digitalWrite(PIN_LED, 1);       // LED OFF
	else
		digitalWrite(PIN_LED, 0);       // LED ON

	// update last sampling time
	last_sampling_time += INTERVAL;
}


// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(PULSE_DURATION);
	digitalWrite(TRIG, LOW);

	return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm

	// Pulse duration to distance conversion example (target distance = 17.3m)
	// - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
	// - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
	//   mm 단위로 하려면 * 1,000이 필요 ==> SCALE = 0.001 * 0.5 * SND_VEL
	//
	// - 예, pulseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
	//        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
	//        = 100,000 * 0.001 * 0.5 * 346
	//        = 17,300 mm ==> 17.3m
}