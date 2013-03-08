#include "WPILib.h"
#include "Shooter.h"
#include "LineBreakEncoder.h"

Shooter::Shooter(){
	p = 0.1f;
	i = 0.0f;
	d = 0.0f;
	flywheel = new Victor(FLYWHEEL_PWM);
	tipper = new Victor(TIPPER_PWM);
	disc_deployer = new Victor(DISC_DEPLOYER_PWM);
	firing = false;
	angle = new Encoder(ENCODER_SHOOTER_ANGLE_A_CHANNEL, ENCODER_SHOOTER_ANGLE_B_CHANNEL);
	speed = new LineBreakEncoder();
	speed->reset();
	deployer = new Solenoid(DEPLOY_SHOOTER_SOLENOID_CHANNEL);
	speed_pid = new PIDController(p, i, d, speed, flywheel);
	speed_pid->Disable();
	angle_pid = new PIDController(p, i, d, angle, tipper);
	angle_pid->Disable();
}

void Shooter::set_pid_values(float p, float i, float d){
	this->p = p;
	this->i = i;
	this->d = d;
}

void Shooter::deploy(){
	deployer->Set(SHOOTER_DEPLOYED);
}

void Shooter::undeploy(){
	deployer->Set(!SHOOTER_DEPLOYED);
}


bool Shooter::ready_to_fire(){
	return (flywheel->Get() < 0.1 || (speed_pid->IsEnabled() && speed_pid->GetError() > 0.1));
}

//TODO: this is prolly also gonna wanna move the elevator
//returns whether or not we actually fired
bool Shooter::fire(){
	//don't let us shoot if the flywheel isn't spinning or if we're not up to speed or if we're already firing
	if (flywheel->Get() < 0.1 || 
			(speed_pid->IsEnabled() && speed_pid->GetError() > 0.1) || firing){
		return false;
	}
	firing = true;
	//move the deployer out until it hits the first switch...
	while(!max->Get()){
		disc_deployer->Set(0.1f);
	}
	//then move it back again until it hits the second one
	while(!min->Get()){
		disc_deployer->Set(-0.1f);
	}
	firing = false;
	return true;
}
void Shooter::set_speed(float new_speed){
	speed_pid->SetSetpoint(new_speed);
	speed->reset();
	speed_pid->Enable();
}

void Shooter::set_angle(float new_angle){
	angle_pid->SetSetpoint(new_angle);
	angle_pid->Enable();
}
