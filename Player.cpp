/*
 * Player.h
 * (C) 2017 Upperbound.com. All rights reserved.
 * Implementation for Player class.
*/

#include "Player.h"
#include "Arduino.h"

Player::Player() {
	clear();
}

void Player::clear() {

	score=0;
	name="";
	roundsWon=0;
}

void Player::resetScore() {
	score=0;
}
// Dumps values to serial

void Player::dumpValues() {
	Serial.print("   name: ");
	Serial.println(name);
	Serial.print("   score: ");
	Serial.println(score);
	Serial.print("   roundsWon: ");
	Serial.println(roundsWon);

}

