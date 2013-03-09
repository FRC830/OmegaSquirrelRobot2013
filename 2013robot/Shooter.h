#ifndef SHOOTER_H_
#define SHOOTER_H_

#include "WPILib.h"
#include "LineBreakEncoder.h"

//class to hold all of the shooter stuff
class Shooter {
	//shooter PWM
	static const int FLYWHEEL_PWM = 2;
	static const int TIPPER_PWM = 6;
	static const int DISC_DEPLOYER_PWM = 8;
	//encoder channels
	static const int ENCODER_SHOOTER_ANGLE_A_CHANNEL = 5;
	static const int ENCODER_SHOOTER_ANGLE_B_CHANNEL = 6;
	//limit switch channels
	static const int DISC_DEPLOYER_MAX_SWITCH = 11; 
	static const int DISC_DEPLOYER_MIN_SWITCH = 12;
	bool moving_forward;
	bool firing;
	
public:
	Victor * flywheel;
	Victor * tipper;
	Victor * disc_deployer;
	DigitalInput * max;
	DigitalInput * min;
	Encoder * angle;
	LineBreakEncoder * speed;
	PIDController * speed_pid;
	PIDController * angle_pid;
	float p, i, d;
	
	Shooter();
	
	void set_pid_values(float p, float i = 0.0f, float d = 0.0f);
	bool ready_to_fire();
	void fire();
	void stop_firing();
	void update();
	void set_speed(float new_speed);
	void set_angle(float new_angle);
};

#endif
