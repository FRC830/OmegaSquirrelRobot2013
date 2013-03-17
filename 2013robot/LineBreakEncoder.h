#ifndef LINE_BREAK_ENCODER_H_
#define LINE_BREAK_ENCODER_H_

#include "WPILib.h"

class LineBreakEncoder : public PIDSource {
private:
	Counter * counter;	//counter to track linebreak signals
	int ticks_per_rev;	//number of ticks in a revolution
public:
	LineBreakEncoder(int dio_channel, int tpr);
	virtual double PIDGet();
};

#endif
