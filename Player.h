/*
 * Player.h
 * (C) 2017 Upperbound.com. All rights reserved.
 * Player class. A data structure that holds player information for players of TCMS games
*/

#ifndef Player_h
#define Player_h

#include "Arduino.h"

class Player {

	public:
		Player();
		void resetScore();
		void clear();
		void dumpValues();
		
		int score;
		int roundsWon;
		String name;
		
};
#endif
