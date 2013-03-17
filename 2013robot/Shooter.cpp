#include "WPILib.h"
#include "Shooter.h"
#include "LineBreakEncoder.h"

Shooter::Shooter(){
	p = 0.1f;
	i = 0.0f;
	d = 0.0f;
	max = new DigitalInput(DISC_DEPLOYER_MAX_SWITCH);
	min = new DigitalInput(DISC_DEPLOYER_MIN_SWITCH);
	flywheel = new Victor(FLYWHEEL_PWM);
	tipper = new Victor(TIPPER_PWM);
	feeder = new Victor(FEEDER_PWM);
	moving_forward = true;
	firing = false;
	angle_encoder = new Encoder(ENCODER_SHOOTER_ANGLE_A_CHANNEL, ENCODER_SHOOTER_ANGLE_B_CHANNEL);
	speed_encoder = new LineBreakEncoder(LINE_BREAK_DIO_CHANNEL, 2);
	speed_pid = new PIDController(p, i, d, speed_encoder, flywheel);
	speed_pid->Disable();
	angle_pid = new PIDController(p, i, d, angle_encoder, tipper);
	angle_pid->Disable();
}

void Shooter::set_pid_values(float p, float i, float d){
	this->p = p;
	this->i = i;
	this->d = d;
}

bool Shooter::ready_to_fire(){
	return (speed_pid->IsEnabled() && speed_pid->GetError() < 0.1);
}

// makes the shooter fire once
// won't do anything unless you're calling update() every cycle
void Shooter::fire(){
	firing = true;
}

//stops the shooter firing
//not normally necessary
void Shooter::stop_firing(){
	firing = false;
}

//call this every cycle
void Shooter::update(){
	if (firing){
		if(moving_forward){
			if (max->Get()){
				feeder->Set(0.5f);
			} else {
				moving_forward = false;
			}
		} else {
			if (min->Get()){
				feeder->Set(-0.5f);
			} else {
				moving_forward = true;
				firing = false; 
			}
		}
	} else {
		feeder->Set(0.0f);
	}
}
void Shooter::set_speed(float new_speed){
	speed_pid->SetSetpoint(new_speed);
	speed_pid->Enable();
}

void Shooter::set_angle(float new_angle){
	angle_pid->SetSetpoint(new_angle);
	angle_pid->Enable();
}
