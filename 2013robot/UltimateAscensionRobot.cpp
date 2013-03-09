#include "WPILib.h"
#include "Gamepad.h"
#include "PIDDrive.h"
#include "Shooter.h"
#include <cmath>

class UltimateAscensionRobot : public IterativeRobot {
	
	//Drive train PWM channels:
	static const int DRIVE_RIGHT_PWM = 10;
	static const int DRIVE_LEFT_PWM = 1;
	
	//Encoder channels:
	static const int ENCODER_DRIVE_RIGHT_A_CHANNEL = 1;
	static const int ENCODER_DRIVE_RIGHT_B_CHANNEL = 2;
	static const int ENCODER_DRIVE_LEFT_A_CHANNEL = 3;
	static const int ENCODER_DRIVE_LEFT_B_CHANNEL = 4;
	
	//analog channel for gyro
	//needs to be 1 or 2 or it WON'T WORK!
	static const int GYRO_CHANNEL = 2;

	//spike to turn on compressor
	static const int COMPRESSOR_DIO_CHANNEL = 14;
	static const int PRESSURE_SENSOR_DIO_CHANNEL = 13;
	
	//Solenoid channels:
	static const int GEAR_SHIFT_SOLENOID_CHANNEL = 1;
	static const int DEPLOY_FEEDER_SOLENOID_CHANNEL = 3;
	
	//Solenoid states:
	static const bool HIGH_GEAR = true;
	static const bool LOW_GEAR = false;
	
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
	 *	Pressing the B button fires a disc out of the shooter if it's up to speed.				*
	 *	Pressing a lower shoulder button slightly increases the roller speed (hold to spin).	*
	 ********************************************************************************************/

	//b button
	static const int FIRE_SHOOTER_BUTTON = 2;
	static const int SPIN_SHOOTER_BUTTON_1 = 5;
	static const int SPIN_SHOOTER_BUTTON_2 = 6;
	static const int RUN_ROLLERS_1 = 7;
	static const int RUN_ROLLERS_2 = 8;

	DigitalOutput * compressor;
	DigitalInput * pressure_sensor;
	
	//These are creating objects for motors & such
	Solenoid * gear_shift;
	
	Encoder * right_drive_encoder;
	Encoder * left_drive_encoder;
	
	Gyro * gyro;
	
	bool arcade_drive;
	RobotDrive * drive;
	Gamepad * pilot;
	Gamepad * copilot;
	DriverStationLCD * lcd;
	
	Victor * left_drive;
	Victor * right_drive;
	
	Victor * elevator_front;
	Victor * elevator_back;
	Victor * pickup_roller;

	//AxisCamera * camera;
	
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
		
		shooter = new Shooter();
		throttle = 0.7f;
		
		compressor = new DigitalOutput(COMPRESSOR_DIO_CHANNEL);
		pressure_sensor = new DigitalInput(PRESSURE_SENSOR_DIO_CHANNEL);
		
		//Names our fancy shifter
		gear_shift = new Solenoid(GEAR_SHIFT_SOLENOID_CHANNEL);
		
		//gyro = new Gyro(GYRO_CHANNEL);
		//gyro->Reset();
		//Names the gamepad
		pilot = new Gamepad(1);
		copilot = new Gamepad(2);
		
		timer = new Timer();
		
		//our variable is a pointer and GetInstance() returns an object, so we take the address
		//camera = &(AxisCamera::GetInstance());
		
		//Names the Driver Station
		lcd = DriverStationLCD::GetInstance();
	}
	
	//Stops driving in disabled
	void DisabledInit(){
		drive->ArcadeDrive(0.0f, 0.0f);
		gear_shift->Set(HIGH_GEAR);
		lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in high gear");
	}
	
	void AutonomousInit(){
		//necessary to deploy shooter
		gear_shift->Set(LOW_GEAR);
		//reset linebreak encoder
		shooter->speed->reset();
		fired_in_auton = false;
		timer->Start();
	}
	
	void TeleopInit(){
		gear_shift->Set(LOW_GEAR);
		lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in low gear");
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
	
	void AutonomousPeriodic(){
		/*
		shooter->flywheel->Set(AUTON_SHOOTER_THROTTLE);
		if (timer->Get() > 3 && !fired_in_auton){
			shooter->fire();
			fired_in_auton = true;
		}
		*/
		if (timer->Get() >= 1 && gear_shift->Get() == LOW_GEAR){
			gear_shift->Set(HIGH_GEAR);
		}
		if (timer->Get() >= 2 && timer->Get() <= 11){
			drive->ArcadeDrive(0.4f, 0.0f);
		}
		/*
		if (timer->Get() == 11){
			drive->ArcadeDrive(-0.4f, 0.0f);
		}
		if (timer->Get() >= 11){
			drive->ArcadeDrive(0.4f, 0.0f);
		}
		*/
		lcd->PrintfLine(DriverStationLCD::kUser_Line2, "time: %d", timer->Get());
		lcd->UpdateLCD();
	}
	
	
	float clamp(float val,float min,float max)
	{
		float retVal = val;
		if(val < min )
		{ 
			retVal = min;
		}
		else if( val > max )
		{
			retVal = max;
		}
		return retVal;
	}
	void TeleopPeriodic(){
		
		float limiter = 1.0;
		float top = limiter;
		float bot = -1.0*limiter;

		if(arcade_drive){
			drive->ArcadeDrive(clamp(pilot->GetLeftY(),bot,top), clamp(pilot->GetRightX(),bot,top));
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In arcade drive");
		} else {
			drive->TankDrive(clamp(pilot->GetLeftY(),bot,top), clamp(pilot->GetRightX(),bot,top));
			lcd->PrintfLine(DriverStationLCD::kUser_Line1, "In tank drive");
		}
		
		if (pilot->GetNumberedButton(SHIFT_LOW_BUTTON_1) || 
				pilot->GetNumberedButton(SHIFT_LOW_BUTTON_2)){
			gear_shift->Set(LOW_GEAR);
		} else if (pilot->GetNumberedButton(SHIFT_HIGH_BUTTON_1) || 
				pilot->GetNumberedButton(SHIFT_HIGH_BUTTON_2)){
			gear_shift->Set(HIGH_GEAR);
		}
		
		if (gear_shift->Get() == HIGH_GEAR){
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in high gear");
		}
		//Small adjustments are only allowed if we're in low gear
		if (gear_shift->Get() == LOW_GEAR) {
			lcd->PrintfLine(DriverStationLCD::kUser_Line4, "in low gear");
		}
		

		//Starts shooter when button is pressed:
		if (copilot->GetNumberedButton(SPIN_SHOOTER_BUTTON_1) || copilot->GetNumberedButton(SPIN_SHOOTER_BUTTON_2)){
			//shooter spins backward, so we need to negate shooter throttle
			shooter->flywheel->Set(throttle);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Engaged at: %d%%", (int) (throttle * 100));
		} else {
			shooter->flywheel->Set(0.0f);
			lcd->PrintfLine(DriverStationLCD::kUser_Line2, "Disengaged at: %d%%", (int) (throttle * 100));
		}
		
		if (copilot->GetNumberedButtonPressed(FIRE_SHOOTER_BUTTON)){
				lcd->PrintfLine(DriverStationLCD::kUser_Line3, "firing");
				shooter->fire();
		}
		
		if (copilot->GetNumberedButtonPressed(3)){
			lcd->PrintfLine(DriverStationLCD::kUser_Line3, "not firing");
			shooter->stop_firing();	
		}
		//Gives the copilot control of the shooter throttle
		
		if (copilot->GetNumberedButtonPressed(4) && throttle <= 0.95){
			throttle += 0.05;
		}
		if (copilot->GetNumberedButtonPressed(1) && throttle >= 0.5){
			throttle -= 0.05;
		}
		
		//compressor is backwards, so setting it to false turns it on and to true turns it off
		//pressure sensor returns true when pressure >= 120psi
		if (pilot->GetNumberedButton(10) && !pressure_sensor->Get()){
			compressor->Set(false);
		} else {
			compressor->Set(true);
		}
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "max: %d min: %d pr: %d", shooter->max->Get(), shooter->min->Get(), pressure_sensor->Get());
		//lcd->PrintfLine(DriverStationLCD::kUser_Line5, "encoder says: %f", shooter->speed->PIDGet());
		
		//camera->GetImage();
		
		shooter->update();
		
		//Updates Driver Station
		lcd->UpdateLCD();		
	}
	 
};

START_ROBOT_CLASS(UltimateAscensionRobot)
