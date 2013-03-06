#include "Shooter.h"

Shooter::Shooter(){
	p = 0.1f;
	i = 0.0f;
	d = 0.0f;
	flywheel = new Victor(FLYWHEEL_PWM);
	tipper = new Victor(TIPPER_PWM);
	angle = new Encoder(ENCODER_SHOOTER_ANGLE_A_CHANNEL, ENCODER_SHOOTER_ANGLE_B_CHANNEL);
	speed = new Encoder(ENCODER_SHOOTER_SPEED_A_CHANNEL, ENCODER_SHOOTER_SPEED_B_CHANNEL);
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

void Shooter::set_speed(float new_speed){
	speed_pid->SetSetpoint(new_speed);
	speed_pid->Enable();
}

void Shooter::set_angle(float new_angle){
	angle_pid->SetSetpoint(new_angle);
	angle_pid->Enable();
}

