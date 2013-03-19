#include "WPILib.h"
#include "Shooter.h"
#include "LineBreakEncoder.h"

Shooter::Shooter(){
	p = 0.1f;
	i = 0.0f;
	d = 0.0f;
	limit_switch = new DigitalInput(DISC_DEPLOYER_LIMIT_SWITCH);
	flywheel = new Victor(FLYWHEEL_PWM);
	tipper = new Victor(TIPPER_PWM);
	feeder = new Victor(FEEDER_PWM);
	firing = false;
	passed_switch = false;
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
	speed_pid->SetPID(p, i, d);
}

bool Shooter::ready_to_fire(){
	return (speed_pid->IsEnabled() && speed_pid->GetError() < 0.1);
}

// makes the shooter fire once
// won't do anything unless you're calling update() every cycle
//doesn't spin the flywheel; firing doesn't really work if the flywheel isn't spinning
void Shooter::fire(){
	firing = true;
}

//stops the shooter firing
//not normally necessary, shooters stop firing automatically
void Shooter::stop_firing(){
	firing = false;
}

//call this every cycle
//will spin the disc feeder if we're firing
//stops firing when we've fired once
void Shooter::update(){
	if (firing){
		//if we're still hitting the switch or if we haven't hit it a second time, move forward
		feeder->Set(0.1f);	//TODO: determine good speed for this
		//if we've moved away from the switch and then hit it again, stop firing
		if (passed_switch && !limit_switch->Get()){
			firing = false;
		}
		passed_switch = limit_switch->Get();	//I think these switches return true when not pressed
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
