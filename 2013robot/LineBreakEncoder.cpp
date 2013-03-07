#include "WPILib.h"
#include "LineBreakEncoder.h"

LineBreakEncoder::LineBreakEncoder(int tpr){
	linebreak = new DigitalInput(LINE_BREAK_DIO_CHANNEL);
	timer = new Timer();
	ticks_passed = 0;
}

double LineBreakEncoder::PIDGet(){
	//ticks_passed / ticks_per_rev = # revolutions
	//# revolutions / time elapsed in seconds = revs/sec
	double result = (ticks_passed / ticks_per_rev) / timer->Get();
	return result;
}

//you HAVE to call this every cRIO cycle
void LineBreakEncoder::update(){
	if (!linebreak->Get()){
		ticks_passed++;
	}
}

//you HAVE to call this every time you change target speeds
void LineBreakEncoder::reset(){
	ticks_passed = 0;
	timer->Reset();
	timer->Start();
}
