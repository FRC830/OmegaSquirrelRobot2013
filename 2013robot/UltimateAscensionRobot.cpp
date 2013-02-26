#include "WPILib.h"
#include "Gamepad.h"
#include <cmath>

class UltimateAscensionRobot : public IterativeRobot {

	//Drive train PWM channels:
	static const int DRIVE_RIGHT = 1;
	static const int DRIVE_LEFT = 2;
	
	//Elevator motor PWM channels:
	static const int LIFT_TOP = 3;
	static const int LIFT_BOTTOM = 4;
	
	//Roller PWM channels:
	static const int BUMP_UP_CHANNEL = 5;
	static const int INTAKE_CHANNEL = 6;
	
	//shooter PWM
	static const int FLYWHEEL_CHANNEL = 7;
	
	//These let us change the direction of the rollers (1 or -1):
	static const int BUMP_UP_DIRECTION = 1;
	static const int INTAKE_DIRECTION = 1;
	

	//Encoder channels (duh):
	static const int ENCODER_1_A_CHANNEL = 1;
	static const int ENCODER_1_B_CHANNEL = 2;
	static const int ENCODER_2_A_CHANNEL = 3;
	static const int ENCODER_2_B_CHANNEL = 4;
	
	//analog channel for gyro
	//needs to be 1 or 2 or it WON'T WORK!
	static const int GYRO_CHANNEL = 1;
	
	//These let us change the direction of the elevator (1 or -1):
	static const int ELEVATOR_TOP_DIRECTION = 1;
	static const int ELEVATOR_BOTTOM_DIRECTION = 1;
	
	//These buttons control pick-up roller speed: 
	static const int ROLLER_SPEED_UP = 6;
	static const int ROLLER_SPEED_DOWN = 8;

	//Buttons used to shift into high and low gear:
	static const int SHIFT_HIGH_BUTTON = 5;
	static const int SHIFT_LOW_BUTTON = 7;
	
	//shooter control button(s):
	static const int FIRE_SHOOTER_BUTTON = 2;
	
	//Solenoid channel:
	static const int GEAR_SHIFT_SOLENOID_CHANNEL = 1;
	
	//Changing gear states:
	static const bool HIGH_GEAR = false;
	static const bool LOW_GEAR = true;
	
	
	//Floats which are used for roller and elevator speeds:
	float rollerSpeed;
	float elevatorSpeed;

	//These are creating objects for motors & such
	Solenoid * gear_shift;
	
	Encoder * encoder1;
	Encoder * encoder2;
	
	Gyro * gyro;
	
	RobotDrive * drive;
	Gamepad * gamepad;
	DriverStationLCD * lcd;
	
	Victor * left_drive;
	Victor * right_drive;
	
	Victor * bump_up;
	Victor * intake;
	
	Victor * lift_top;
	Victor * lift_bottom;
	
	Victor * shooter_wheel;
	float throttle;
	
	//Here we get to the REAL code
public:
	UltimateAscensionRobot() {

	}
	
	void RobotInit(){
		//"Naming" refers to initializing (like giving a PWM port)
		//Drive train motors are named
		left_drive = new Victor(DRIVE_LEFT);
		right_drive = new Victor(DRIVE_RIGHT);
		
		//I have no clue what this does (probably states what left & right are)
		drive = new RobotDrive(
				left_drive,
				right_drive
				);
		
		//Names lift victors
		lift_top = new Victor(LIFT_TOP);
		lift_bottom = new Victor(LIFT_BOTTOM);
		
		//Names the rollers
		bump_up = new Victor(BUMP_UP_CHANNEL);
		intake = new Victor(INTAKE_CHANNEL);
		
		shooter_wheel = new Victor(FLYWHEEL_CHANNEL);
		throttle = 0.7f;
		
		//Set the roller/elevator motors to 0 just in case
		rollerSpeed = 0.0;
		
		elevatorSpeed = 0.0;
		
		//Names our fancy shifter
		gear_shift = new Solenoid(GEAR_SHIFT_SOLENOID_CHANNEL);
		

		//Naes encoders
		encoder1 = new Encoder(ENCODER_1_A_CHANNEL, ENCODER_1_B_CHANNEL);
		encoder2 = new Encoder(ENCODER_2_A_CHANNEL, ENCODER_2_B_CHANNEL);

		gyro = new Gyro(GYRO_CHANNEL);
		
		//Names the gamepad
		gamepad = new Gamepad(1);
		
		//Names the Driver Station
		lcd = DriverStationLCD::GetInstance();
	}
	
	//Stops driving in disabled
	void DisabledInit(){
		drive->ArcadeDrive(0.0f, 0.0f);
	}
	
	void AutonInit(){
		
	}
	
	void TeleopInit(){
		//Starts the encoders
		encoder1->Start();
		encoder2->Start();
	}
	
	void DisabledPeriodic(){
		//Constantly stops the drive (failsafe. Maybe.)
		drive->ArcadeDrive(0.0f, 0.0f);
	}
	
	void AutonPeriodic(){
		//WE RIDE in autonomous mode (Cue horse hooves)
		drive->ArcadeDrive(0.5f, 0.0f);
	}
	
	void TeleopPeriodic(){
		//float forwardSpeed = gamepad->GetLeftY();
		//float sideSpeed = gamepad->GetRightX();
		//drive->ArcadeDrive(CurveAcceleration(forwardSpeed), sideSpeed);
		
		//This code takes the joystick values:
		float leftSpeed = gamepad->GetLeftY();
		float rightSpeed = -gamepad->GetRightY();
		//This code limits the speed:
		if (leftSpeed > 1.0f)
			leftSpeed = 1.0f;
		if (leftSpeed < -1.0f)
			leftSpeed = -1.0f;
		if (rightSpeed > 1.0f)
			rightSpeed = 1.0f;
		if (rightSpeed < -1.0f)
			rightSpeed = -1.0f;
		//Gives the values to the victors:
		drive->TankDrive(leftSpeed, rightSpeed);
		
		//starts shooter when button is pressed:
		if (gamepad->GetNumberedButton(FIRE_SHOOTER_BUTTON)){
			shooter_wheel->Set(throttle);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Throttle at: %d%%", (int) (throttle * 100));
			
		}
		
		//The Fancy Roller Code:
		//Speeds & slows the rollers:
		if (rollerSpeed <= 0.99 && gamepad->GetNumberedButton(ROLLER_SPEED_UP)){
			rollerSpeed += .01;
		} else if (rollerSpeed >= .01 && gamepad->GetNumberedButton(ROLLER_SPEED_DOWN)){
			rollerSpeed -= .01;
		}
		
		//Set the victors to the values
		bump_up->Set(rollerSpeed * BUMP_UP_DIRECTION);
		intake->Set(rollerSpeed * INTAKE_DIRECTION);
		

		//Speeds/slows the elevator based on the D-pad
		int pressed = 0; 
		pressed = gamepad->GetDPad();
		if (elevatorSpeed <= 0.99 && pressed == Gamepad::kUp)// || pressed == Gamepad::kUpLeft || pressed == Gamepad::kUpRight)
		{
			elevatorSpeed += 0.01;
		}
		if (elevatorSpeed >= 0.01 &&pressed  == Gamepad::kDown)// || pressed == Gamepad::kDownLeft || pressed == Gamepad::kDownRight)
		{
			elevatorSpeed -= 0.01;
		}

		
		//Sets the elevator victors
		lift_top->Set(elevatorSpeed*ELEVATOR_TOP_DIRECTION);
		lift_bottom->Set(elevatorSpeed*ELEVATOR_BOTTOM_DIRECTION);		
		
		//Shifts them geers:
		if (gamepad->GetNumberedButton(SHIFT_LOW_BUTTON)){
			gear_shift->Set(LOW_GEAR);
		} else if (gamepad->GetNumberedButton(SHIFT_HIGH_BUTTON)){
			gear_shift->Set(HIGH_GEAR);
		}
		
		//Displays speeds to Driver Station
		lcd->PrintfLine(DriverStationLCD::kUser_Line1,"l/r: %f/%f",leftSpeed, rightSpeed);
		
		//Displays gears
		if (gear_shift->Get() == HIGH_GEAR)
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in high gear");
		else if (gear_shift->Get() == LOW_GEAR)
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in low gear");
		
		//Displays elevator speeds
		lcd->PrintfLine(DriverStationLCD::kUser_Line3, "elevator at %f", elevatorSpeed);

		//Displays encoder values
		//uncomment if you want to see encoder values, we removed them to get more lines
		/*
		int encoder_value1 = encoder1->Get();
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "encoder at %i", encoder_value1);
		
		int encoder_value2 = encoder2->Get();
		lcd->PrintfLine(DriverStationLCD::kUser_Line6, "encoder at %i", encoder_value2);
		*/
		
		//Displays gyro value
		float gyro_angle = gyro->GetAngle();
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "Gyro at %f", gyro_angle);
		
		//Updates Driver Station
		lcd->UpdateLCD();		
	}
	
	//Later this will be PID-ish
	float CurveAcceleration(float input) {
		//TODO: apparently we're going to do something here sometime
		return input;
	}
};

START_ROBOT_CLASS(UltimateAscensionRobot)