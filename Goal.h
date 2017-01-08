/*
 * Goal.h
 * (C) 2017 Upperbound.com. All rights reserved.
 * Goal class. 
*/

#ifndef Goal_h
#define Goal_h

#include "Arduino.h"

class Goal {

	public:
		Goal(int laserPin, int photoSensorPin, int lightThreshold);
		void activate();
		void deactivate();
		bool read(); // Returns true if light beam was broken
		bool isActivated();
		void setLightThreshold(int lightThreshold);
		int getLightThreshold();
		String label; // optional label
		
		
	private:
		int _laserPin;
		int _photoSensorPin;
		bool _isActivated;
		int _lightThreshold;
		
				
};
#endif
