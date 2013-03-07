#ifndef LINE_BREAK_ENCODER_H_
#define LINE_BREAK_ENCODER_H_

#include "WPILib.h"

class LineBreakEncoder : public PIDSource {
private:
	static const int LINE_BREAK_DIO_CHANNEL = 3; //TODO: determine this
	DigitalInput * linebreak;
	Timer * timer;
	int ticks_per_rev;
	int ticks_passed;
public:
	LineBreakEncoder(int tpr = 2);
	virtual double PIDGet();
	//you HAVE to call this every cRIO cycle
	void update();
	//you HAVE to call this every time you change target speeds
	void reset();
};

#endif
