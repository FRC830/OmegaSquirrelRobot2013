#ifndef ULTIMATE_DRIVE_H_
#define ULTIMATE_DRIVE_H_

#include "WPILib.h"
/*
 * Class to allow us to PID output to the drivetrain. It contains a speed variable which a PIDController can both
 * read from and write to, so hopefully we simulate having encoders on the wheel.
 */
class PIDDrive : public RobotDrive, public PIDSource, public PIDOutput, public PIDController {
private:
	float speed;
public:
	PIDDrive(Victor * left, Victor * right, float p, float i, float d);
	virtual void PIDWrite(float output);
	virtual double PIDGet();
	void go(float rotate_value);
	
};


#endif
