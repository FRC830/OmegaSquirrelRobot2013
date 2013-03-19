#ifndef LINE_BREAK_ENCODER_H_
#define LINE_BREAK_ENCODER_H_

#include "WPILib.h"

class LineBreakEncoder : public PIDSource {
private:
	int ticks_per_rev;	//number of ticks in a revolution
public:
	Counter * counter;	//counter to track linebreak signals
	LineBreakEncoder(int dio_channel, int tpr);
	virtual double PIDGet();
};

#endif
