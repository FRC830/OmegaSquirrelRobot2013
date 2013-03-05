#include "WPILib.h"
#include "Gamepad.h"
#include "PIDDrive.h"
#include <cmath>

class UltimateAscensionRobot : public IterativeRobot {

	
	//Drive train PWM channels:
	static const int DRIVE_RIGHT_PWM = 10;
	static const int DRIVE_LEFT_PWM = 1;
	
	//Roller PWM channels:
	static const int ELEVATOR_FRONT_PWM = 2;
	static const int ELEVATOR_BACK_PWM = 3;
	static const int FEEDER_PWM = 8;
	static const int PICK_UP_PWM = 9;
	static const int BUMP_UP_PWM = 7;
	
	//shooter PWM
	static const int FLYWHEEL_PWM = 5;
	static const int TIPPER_PWM = 6;
	
	//Encoder channels:
	static const int ENCODER_DRIVE_RIGHT_A_CHANNEL = 1;
	static const int ENCODER_DRIVE_RIGHT_B_CHANNEL = 2;
	static const int ENCODER_DRIVE_LEFT_A_CHANNEL = 3;
	static const int ENCODER_DRIVE_LEFT_B_CHANNEL = 4;
	static const int ENCODER_SHOOTER_ANGLE_A_CHANNEL = 5;
	static const int ENCODER_SHOOTER_ANGLE_B_CHANNEL = 6;
	static const int ENCODER_SHOOTER_SPEED_A_CHANNEL = 7;
	static const int ENCODER_SHOOTER_SPEED_B_CHANNEL = 8;
	
	//analog channel for gyro
	//needs to be 1 or 2 or it WON'T WORK!
	static const int GYRO_CHANNEL = 2;
	
	//constants for which elevator directions are forwards
	static const int ELEVATOR_FRONT_DIRECTION = 1;
	static const int ELEVATOR_BACK_DIRECTION = 1;
	static const int ELEVATOR_TOP_DIRECTION = 1;
	static const int PICKUP_DIRECTION = 1;
	
	//Solenoid channels:
	static const int GEAR_SHIFT_SOLENOID_CHANNEL = 1;
	static const int DEPLOY_SHOOTER_SOLENOID_CHANNEL = 2;
	static const int DEPLOY_FEEDER_SOLENOID_CHANNEL = 3;
	
	//Solenoid states:
	static const bool HIGH_GEAR = true;
	static const bool LOW_GEAR = false;
	
	static const bool SHOOTER_DEPLOYED = true;
	static const bool FEEDER_DEPLOYED = true;
	
	/********************************* PILOT CONTROLS ******************************************* 
	 *	During disabled mode, pressing the upper shoulder buttons switches to arcade drive,		*
	 *	and the lower shoulder buttons switch to tank drive.									*
	 *	Tank drive uses the left-y and right-y for left and right speed, respectively.			*
	 *	Arcade drive uses the left-y and right-x for forward speed and direction, respectively.	*
	 *	Pressing one of the upper shoulder buttons shifts to high gear, a lower one to low gear.*
	 ********************************************************************************************/
	
	static const int SHIFT_HIGH_BUTTON_1 = 5;
	static const int SHIFT_HIGH_BUTTON_2 = 6;
	static const int SHIFT_LOW_BUTTON_1 = 7;
	static const int SHIFT_LOW_BUTTON_2 = 8;
	
	/********************************* COPILOT CONTROLS ***************************************** 
	 *	Pressing up or down on the dpad raises or lowers throttle by 5% (default is 70%).		*
	 *	Pressing an upper shoulder button spins the flywheel at the current throttle speed.		*
	 *	Pressing a lower shoulder button slightly increases the roller speed (hold to spin).	*
	 ********************************************************************************************/

	static const int FIRE_SHOOTER_BUTTON_1 = 5;
	static const int FIRE_SHOOTER_BUTTON_2 = 6;
	static const int RUN_ROLLERS_1 = 7;
	static const int RUN_ROLLERS_2 = 8;
	
	//Floats which are used for roller and elevator speeds:
	float rollerSpeed;
	float elevatorSpeed;

	//These are creating objects for motors & such
	Solenoid * gear_shift;
	Solenoid * deploy_shooter;
	Solenoid * deploy_feeder;
	
	Encoder * right_drive_encoder;
	Encoder * left_drive_encoder;
	
	Gyro * gyro;
	
	bool arcade_drive;
	static const float SMALL_ADJUSTMENT_UNIT = 0.1;
	RobotDrive * drive;
	Gamepad * pilot;
	Gamepad * copilot;
	DriverStationLCD * lcd;
	
	Victor * left_drive;
	Victor * right_drive;
	
	Victor * elevator_front;
	Victor * elevator_back;
	Victor * pickup_roller;
	Victor * feeder;

	
	float throttle;
	
	//class to hold all of the shooter stuff
	class Shooter {
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
		
		Shooter(){
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
		void set_pid_values(float p, float i = 0.0f, float d = 0.0f){
			this->p = p;
			this->i = i;
			this->d = d;
		}
		void deploy(){
			deployer->Set(SHOOTER_DEPLOYED);
		}
		void undeploy(){
			deployer->Set(!SHOOTER_DEPLOYED);
		}
		void set_speed(float new_speed){
			speed_pid->SetSetpoint(new_speed);
			speed_pid->Enable();
		}
		void set_angle(float new_angle){
			angle_pid->SetSetpoint(new_angle);
			angle_pid->Enable();
		}
	};
	
	
	Shooter * shooter;
	
	//Here we get to the REAL code
public:
	UltimateAscensionRobot() {

	}
	
	void RobotInit(){
		//"Naming" refers to initializing (like giving a PWM port)
		//Drive train motors are named
		left_drive = new Victor(DRIVE_LEFT_PWM);
		right_drive = new Victor(DRIVE_RIGHT_PWM);
		
		drive = new RobotDrive(
				left_drive,
				right_drive
				);
		arcade_drive = true;
		//Names lift victors
		
		//Names the rollers
		elevator_front = new Victor(ELEVATOR_FRONT_PWM);
		elevator_back = new Victor(ELEVATOR_BACK_PWM);
		feeder = new Victor(FEEDER_PWM);
		pickup_roller = new Victor(PICK_UP_PWM);
		
		shooter = new Shooter();
		throttle = 0.7f;
		
		//Set the roller/elevator motors to 0 just in case
		rollerSpeed = 0.0;
		
		elevatorSpeed = 0.0;
		
		//Names our fancy shifter
		gear_shift = new Solenoid(GEAR_SHIFT_SOLENOID_CHANNEL);
		deploy_shooter = new Solenoid(DEPLOY_SHOOTER_SOLENOID_CHANNEL);
		deploy_feeder = new Solenoid(DEPLOY_FEEDER_SOLENOID_CHANNEL);
		
		//gyro = new Gyro(GYRO_CHANNEL);
		//gyro->Reset();
		//Names the gamepad
		pilot = new Gamepad(1);
		copilot = new Gamepad(2);
		
		//Names the Driver Station
		lcd = DriverStationLCD::GetInstance();
	}
	
	//Stops driving in disabled
	void DisabledInit(){
		drive->ArcadeDrive(0.0f, 0.0f);
		shooter->undeploy();
		deploy_feeder->Set(!FEEDER_DEPLOYED);
	}
	
	void AutonInit(){
		shooter->deploy();
	}
	
	void TeleopInit(){

	}
	
	void DisabledPeriodic(){
		//Switches between arcade drive and tank drive
		drive->ArcadeDrive(0.0f, 0.0f);
		if (pilot->GetNumberedButton(5) || pilot->GetNumberedButton(6)){
			arcade_drive = true;
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In arcade drive");
			lcd->UpdateLCD();
		}
		if (pilot->GetNumberedButton(7) || pilot->GetNumberedButton(8)){
			arcade_drive = false;
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In tank drive");
			lcd->UpdateLCD();
		}
	}
	
	void AutonPeriodic(){
		//WE RIDE in autonomous mode (Cue horse hooves)
		drive->ArcadeDrive(0.5f, 0.0f);
	}
	
	void TeleopPeriodic(){
		if(arcade_drive){
			drive->ArcadeDrive(pilot->GetLeftY(), pilot->GetRightX());
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In arcade drive");
		} else {
			drive->TankDrive(pilot->GetLeftY(), pilot->GetRightY());
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In tank drive");
		}
		
		if (pilot->GetNumberedButton(SHIFT_LOW_BUTTON_1) || 
				pilot->GetNumberedButton(SHIFT_LOW_BUTTON_2)){
			gear_shift->Set(LOW_GEAR);
		} else if (pilot->GetNumberedButton(SHIFT_HIGH_BUTTON_1) || 
				pilot->GetNumberedButton(SHIFT_HIGH_BUTTON_2)){
			gear_shift->Set(HIGH_GEAR);
		}
		
		if (gear_shift->Get() == HIGH_GEAR)
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in high gear");
		//Small adjustments are only allowed if we're in low gear
		else if (gear_shift->Get() == LOW_GEAR) {
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in low gear");
			/* for some reason the small adjustment code breaks everything right now
			Gamepad::DPadDirection dpad = pilot->GetDPad();
			if (dpad == Gamepad::kRight){
				drive->ArcadeDrive(0.0f, SMALL_ADJUSTMENT_UNIT);
			}
			if (dpad == Gamepad::kLeft){
				drive->ArcadeDrive(0.0f, -SMALL_ADJUSTMENT_UNIT);
			}
			if (dpad == Gamepad::kUp){
				drive->ArcadeDrive(SMALL_ADJUSTMENT_UNIT, 0.0f);
			}
			if (dpad  == Gamepad::kDown){
				drive->ArcadeDrive(-SMALL_ADJUSTMENT_UNIT, 0.0f);
			}
			*/
		}
		

		//Starts shooter when button is pressed:
		if (copilot->GetNumberedButton(FIRE_SHOOTER_BUTTON_1) || copilot->GetNumberedButton(FIRE_SHOOTER_BUTTON_2)){
			shooter->flywheel->Set(throttle);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Engaged at: %d%%", (int) (throttle * 100));
		} else {
			shooter->flywheel->Set(0);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Disengaged at: %d%%", (int) (throttle * 100));
		}

		//Gives the copilot control of the shooter throttle
		if (copilot->GetDPad() == Gamepad::kUp && throttle <= 0.95){
			throttle += 0.05;
		}
		if (copilot->GetDPad() == Gamepad::kDown && throttle >= 0.5){
			throttle -= 0.05;
		}

		//The Fancy Roller Code:
		//Speeds & slows the rollers:
		if (rollerSpeed <= 0.99 && copilot->GetNumberedButton(RUN_ROLLERS_1) || copilot->GetNumberedButton(RUN_ROLLERS_2)){
			rollerSpeed += .01;
		} else if (rollerSpeed >= .01){
			rollerSpeed -= .01;
		}
		
		//Set the victors to the values
		elevator_front->Set(rollerSpeed * ELEVATOR_FRONT_DIRECTION);
		elevator_back->Set(rollerSpeed * ELEVATOR_BACK_DIRECTION);
		pickup_roller->Set(rollerSpeed * PICKUP_DIRECTION);
		

		//Displays elevator speeds
		lcd->PrintfLine(DriverStationLCD::kUser_Line3, "elevator at %f", elevatorSpeed);
	
		//Updates Driver Station
		lcd->UpdateLCD();		
	}
	
};

START_ROBOT_CLASS(UltimateAscensionRobot)
