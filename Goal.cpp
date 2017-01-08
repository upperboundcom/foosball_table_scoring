/*
 * Goal.cpp
 * (C) 2017 Upperbound.com. All rights reserved.
 * Implementation for Goal class.
*/

#include "Goal.h"
#include "Arduino.h"

/* 
	Constructor
*/
Goal::Goal(int laserPin, int photoSensorPin, int lightThreshold) {

	_laserPin = laserPin;
	_photoSensorPin = photoSensorPin;
	_lightThreshold = lightThreshold;
	_isActivated = false;
	pinMode(_laserPin, OUTPUT);
}

/* 
	Sets light threshold, which determines photo resistor sensitivity.
*/
void Goal::setLightThreshold(int lightThreshold) {

	_lightThreshold = lightThreshold;
}
	

/* 
	Activates the goal by turning on the laser.
*/
void Goal::activate() {

	digitalWrite(_laserPin, HIGH);
	_isActivated = true;
}

/* 
	Deactivates the goal by turning off the laser.
*/
void Goal::deactivate() {

	digitalWrite(_laserPin, LOW);
	_isActivated = false;
}
/*
	Returns true if the goal is activated; false otherwise
*/
bool Goal::isActivated() {
	return _isActivated;
}
	
/* 
	Returns true if the laser light level was broken, indicating a goal.
*/
bool Goal::read() {

	int val = analogRead(_photoSensorPin);

	if (val <= _lightThreshold) {
		return true;
	}
	else {
		return false;
	}
	
}

/*
	Returns the current light threshold
*/
int Goal::getLightThreshold() {
	return _lightThreshold;
}

