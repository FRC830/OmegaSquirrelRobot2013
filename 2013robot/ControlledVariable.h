#ifndef CONTROLLED_VARIABLE_H_
#define CONTROLLED_VARIABLE_H_

class ControlledVariable {
private:
	float max_change;
	float max;
	float min;
	float * value;
public:
	ControlledVariable(float * value, float max_change = 0.1f, float max = 1.0, float min = 0.1);
	float operator=(float new_value);
	float operator++();
	float operator--();
	float operator*();
};

#endif
