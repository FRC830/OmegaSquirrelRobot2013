#include "ControlledVariable.h"
#include <cmath>

ControlledVariable::ControlledVariable(float * value, float max_change, float max, float min){
	this->value = value;
	this->max_change = max_change;
	this->max = max;
	this->min = min;
	if (*value > max)
		*value = max;
	if (*value < min)
		*value = min;
}

float ControlledVariable::operator=(float new_value){
	float dif = *value - new_value;
	if ((dif > 0 ? dif : -dif) < max_change)
		*value = new_value;
	else if (dif > 0)
		*value += max_change;
	else
		*value -= max_change;
	if (*value > max)
		*value = max;
	if (*value < min)
		*value = min;
	return *value;
}

float ControlledVariable::operator*(){
	return *value;
}

float ControlledVariable::operator++(){
	*value += max_change;
	if (*value > max)
		*value = max;
	if (*value < min)
		*value = min;
	return *value;
}

float ControlledVariable::operator--(){
	*value -= max_change;
	if (*value > max)
		*value = max;
	if (*value < min)
		*value = min;
	return *value;
}


//float roller_speed = 0.0;
//ControlledVariable cv_roller_speed(&roller_speed);

