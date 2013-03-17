 #include "WPILib.h"
#include "LineBreakEncoder.h"

LineBreakEncoder::LineBreakEncoder(int dio_channel, int tpr){
	ticks_per_rev = tpr;
	counter = new Counter(dio_channel);
	counter->Start();
}

double LineBreakEncoder::PIDGet(){
	//period measures time in seconds between signals from the line break
	//if there are n signals per revolution, period * n = seconds per revolution
	//we want revolutions per second, so we take the reciprocal of this 
	return 1.0 / (counter->GetPeriod() * ticks_per_rev);
}
