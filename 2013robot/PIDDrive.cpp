#include "WPILib.h"
#include "PIDDrive.h"

PIDDrive::PIDDrive(Victor * left, Victor * right, float p, float i, float d) : 
											RobotDrive(left, right), 
											PIDController (p, i, d, this, this){
	
}
void PIDDrive::PIDWrite(float output){
	speed = output;
}

double PIDDrive::PIDGet(){
	return speed;
}

void PIDDrive::go(float rotate_value){
	ArcadeDrive(speed, rotate_value);
}
