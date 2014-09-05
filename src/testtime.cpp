// Testing the time precision
//

#include "utils.h"
#include "utilsTime.h"
#include "utilsStats.h"
#include <iostream>

const int  kNumSamples = 30;

void TestTime(int sleepTime)
{
	// First, test the time between adjacent sames
	StatsCollector<uint32> stats(kNumSamples);
	Micro_t lastTime = Microseconds();
	for (int i = 0; i < kNumSamples; ++i) {
		if (sleepTime > 0 && sleepTime < 1000)
			SleepMicro(sleepTime);
		else if (sleepTime >= 1000)
			SleepMilli(sleepTime / 1000);
		Micro_t t = Microseconds();
		stats.Record(MicroDiff(t, lastTime));
		lastTime = t;
	}
	// Show times
	if (sleepTime == 0)
		cout << "Time between calls to Microseconds() [No sleep]" << endl;
	else if (sleepTime < 1000)
		cout << "Time between calls to SleepMicro(" << sleepTime << ")" << endl;
	else
		cout << "Time between successive calls to SleepMilli(" << sleepTime/1000 << ")" << endl;
	cout << "   " << stats.GetSummaryString() << endl;
	cout << "   Samples: " << stats.GetSamplesString() << endl;
	cout << endl;
}

int main () {
	TestTime(0);
	TestTime(10);
	TestTime(100);
	TestTime(1000);
	return 0;
}