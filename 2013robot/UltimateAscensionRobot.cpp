#include "WPILib.h"
#include "Gamepad.h"
#include "PIDDrive.h"
#include "Shooter.h"
#include <cmath>

class UltimateAscensionRobot : public IterativeRobot {
	
	//Drive train PWM channels:
	static const int DRIVE_RIGHT_PWM = 10;
	static const int DRIVE_LEFT_PWM = 1;
	
	//Roller PWM channels:
	static const int ELEVATOR_FRONT_PWM = 10;
	static const int ELEVATOR_BACK_PWM = 5;
	static const int FEEDER_PWM = 8;
	static const int PICK_UP_PWM = 9;
	static const int BUMP_UP_PWM = 7;
	
	//Encoder channels:
	static const int ENCODER_DRIVE_RIGHT_A_CHANNEL = 1;
	static const int ENCODER_DRIVE_RIGHT_B_CHANNEL = 2;
	static const int ENCODER_DRIVE_LEFT_A_CHANNEL = 3;
	static const int ENCODER_DRIVE_LEFT_B_CHANNEL = 4;
	
	//analog channel for gyro
	//needs to be 1 or 2 or it WON'T WORK!
	static const int GYRO_CHANNEL = 2;
	
	//constants for which elevator directions are forwards
	static const int ELEVATOR_FRONT_DIRECTION = 1;
	static const int ELEVATOR_BACK_DIRECTION = 1;
	static const int ELEVATOR_TOP_DIRECTION = 1;
	static const int PICKUP_DIRECTION = 1;
	
	//spike to turn on compressor
	static const int COMPRESSOR_DIO_CHANNEL = 14;
	static const int PRESSURE_SENSOR_DIO_CHANNEL = 13;
	
	//Solenoid channels:
	static const int GEAR_SHIFT_SOLENOID_CHANNEL = 1;
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
	 *	Pressing button 3 (the B button) fires a disc out of the shooter if it's up to speed.	*
	 *	Pressing a lower shoulder button slightly increases the roller speed (hold to spin).	*
	 ********************************************************************************************/

	static const int FIRE_SHOOTER_BUTTON = 3;
	static const int SPIN_SHOOTER_BUTTON_1 = 5;
	static const int SPIN_SHOOTER_BUTTON_2 = 6;
	static const int RUN_ROLLERS_1 = 7;
	static const int RUN_ROLLERS_2 = 8;
	
	//Floats which are used for roller and elevator speeds:
	float rollerSpeed;
	float elevatorSpeed;

	DigitalOutput * compressor;
	DigitalInput * pressure_sensor;
	
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

	AxisCamera * camera;
	
	float throttle;
	//victor value for auton mode
	static const float AUTON_SHOOTER_THROTTLE = 0.7f; //TODO: determine this
	//shooter speed in rev/s in auton if we get PID working
	static const int AUTON_SHOOTER_SPEED = 0;//TODO: determine this
	//whether we've fired yet in auton
	bool fired_in_auton;
	Timer * timer;
	
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
		
		//Invert left drive motor, it runs backwards
		drive->SetInvertedMotor(RobotDrive::kFrontLeftMotor, true);
		drive->SetInvertedMotor(RobotDrive::kRearLeftMotor, true);
		
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
		
		compressor = new DigitalOutput(COMPRESSOR_DIO_CHANNEL);
		pressure_sensor = new DigitalInput(PRESSURE_SENSOR_DIO_CHANNEL);
		
		//Names our fancy shifter
		gear_shift = new Solenoid(GEAR_SHIFT_SOLENOID_CHANNEL);
		deploy_feeder = new Solenoid(DEPLOY_FEEDER_SOLENOID_CHANNEL);
		
		//gyro = new Gyro(GYRO_CHANNEL);
		//gyro->Reset();
		//Names the gamepad
		pilot = new Gamepad(1);
		copilot = new Gamepad(2);
		
		timer = new Timer();
		
		//our variable is a pointer and GetInstance() returns an object, so we take the address
		camera = &(AxisCamera::GetInstance());
		
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
		//reset linebreak encoder
		shooter->speed->reset();
		fired_in_auton = false;
		timer->Start();
	}
	
	void TeleopInit(){
		//strictly speaking, the shooter should already have deployed in autonomous when teleop starts
		shooter->deploy();
		//reset linebreak encoder
		shooter->speed->reset();
	}
	
	void DisabledPeriodic(){
		
		compressor->Set(true);
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
		shooter->flywheel->Set(AUTON_SHOOTER_THROTTLE);
		if (timer->Get() > 3 && !fired_in_auton){
			shooter->fire();
			fired_in_auton = true;
		}
		
	}
	
	void TeleopPeriodic(){
		shooter->deploy();
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
		if (copilot->GetNumberedButton(SPIN_SHOOTER_BUTTON_1) || copilot->GetNumberedButton(SPIN_SHOOTER_BUTTON_2)){
			//shooter spins backward, so we need to negate shooter throttle
			shooter->flywheel->Set(-throttle);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Engaged at: %d%%", (int) (throttle * 100));
		} else {
			shooter->flywheel->Set(0.0f);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Disengaged at: %d%%", (int) (throttle * 100));
		}

		if (copilot->GetNumberedButtonPressed(FIRE_SHOOTER_BUTTON)){
			if (shooter->ready_to_fire()){
				lcd->PrintfLine(DriverStationLCD::kUser_Line5, "shooter ready to fire");
				shooter->fire();
			} else {
				lcd->PrintfLine(DriverStationLCD::kUser_Line5, "shooter not ready to fire");
			}
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
		
		if (pilot->GetNumberedButton(10) && !pressure_sensor->Get()){
			compressor->Set(false);
		} else {
			compressor->Set(true);
		}
		
		
		//Displays elevator speeds
		lcd->PrintfLine(DriverStationLCD::kUser_Line3, "elevator at %f", elevatorSpeed);
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "encoder says: %f", shooter->speed->PIDGet());
		
		//don't know whether this is necessary to display the image on the dashboard, but it probably won't hurt
		camera->GetImage();
		
		//Updates Driver Station
		lcd->UpdateLCD();		
	}
	
};

START_ROBOT_CLASS(UltimateAscensionRobot)
