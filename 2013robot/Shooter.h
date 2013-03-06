#include "WPILib.h"

//class to hold all of the shooter stuff
class Shooter : public PIDSource, PIDOutput {
	//shooter PWM
	static const int FLYWHEEL_PWM = 5;
	static const int TIPPER_PWM = 6;
	//encoder channels
	static const int ENCODER_SHOOTER_ANGLE_A_CHANNEL = 5;
	static const int ENCODER_SHOOTER_ANGLE_B_CHANNEL = 6;
	static const int ENCODER_SHOOTER_SPEED_A_CHANNEL = 7;
	static const int ENCODER_SHOOTER_SPEED_B_CHANNEL = 8;
	//solenoid
	static const int DEPLOY_SHOOTER_SOLENOID_CHANNEL = 2;
	//constants:
	static const bool SHOOTER_DEPLOYED = true;
public:
	
	Victor * flywheel;
	Victor * tipper;
	Encoder * angle;
	Encoder * speed;
	Solenoid * deployer;
	PIDController * speed_pid;
	PIDController * angle_pid;
	float p, i, d;
	
	Shooter();
	
	void set_pid_values(float p, float i = 0.0f, float d = 0.0f);
	void deploy();
	void undeploy();
	void set_speed(float new_speed);
	void set_angle(float new_angle);
	
	
};

