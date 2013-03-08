#ifndef LINE_BREAK_ENCODER_H_
#define LINE_BREAK_ENCODER_H_

#include "WPILib.h"

class LineBreakEncoder : public PIDSource {
private:
	static const int LINE_BREAK_DIO_CHANNEL = 1; //TODO: determine this
	Counter * counter;
	Timer * timer;
	int ticks_per_rev;
public:
	LineBreakEncoder(int tpr = 2);
	virtual double PIDGet();
	//you HAVE to call this every time you change target speeds
	void reset();
};

#endif
