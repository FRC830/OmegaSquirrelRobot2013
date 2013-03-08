 #include "WPILib.h"
#include "LineBreakEncoder.h"

LineBreakEncoder::LineBreakEncoder(int tpr){
	counter = new Counter(LINE_BREAK_DIO_CHANNEL);
	timer = new Timer();
	counter->Start();
	timer->Start();
}

double LineBreakEncoder::PIDGet(){
	//ticks_passed / ticks_per_rev = # revolutions
	//# revolutions / time elapsed in seconds = revs/sec
	double result = (counter->Get() / ticks_per_rev) / timer->Get();
	return result;
}


//you HAVE to call this every time you change target speeds
void LineBreakEncoder::reset(){
	counter->Reset();
	counter->Start();
	timer->Reset();
	timer->Start();
}
