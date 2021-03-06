/*
 * Blinki.cpp
 *
 *  Created on: 27.11.2013
 *      Author: Jannik
 */

#include "Blinki.h"
#include "HWaccess.h"
#include <stdio.h>
#include "HALAktorik.h"


hal::Blinki::Blinki() {

}

hal::Blinki::~Blinki() {
}


void hal::Blinki::execute(void* arg) {

	cout << "Blinki startet" << endl;
	while (!isStopped()) {
		switch ((int) arg) {
		case REDFAST:
			flashRedFast();
			break;
		case REDSLOW:
			flashRedSlow();
			break;
		case YELLOW:
			flashYellow();
			break;
		case GREEN:
			flashGreenIni();
			break;
		}
	}
}

void hal::Blinki::shutdown() {
	HALAktorik::getInstance()->yellowLigths(OFF);
	HALAktorik::getInstance()->redLigths(OFF);
	cout << "shutdown Blinki..." << endl;

}

void hal::Blinki::flashYellow() {
	HALAktorik::getInstance()->yellowLigths(ON);
	sleep(1);
	HALAktorik::getInstance()->yellowLigths(OFF);
	sleep(1);
}

void hal::Blinki::flashRedSlow() {
	HALAktorik::getInstance()->redLigths(ON);
	sleep(2);
	HALAktorik::getInstance()->redLigths(OFF);
	sleep(2);
}

void hal::Blinki::flashRedFast() {
	HALAktorik::getInstance()->redLigths(ON);
	sleep(1);
	HALAktorik::getInstance()->redLigths(OFF);
	sleep(1);
}

void hal::Blinki::flashGreenIni(){
	HALAktorik::getInstance()->greenLigths(ON);
	sleep(1);
	HALAktorik::getInstance()->greenLigths(OFF);
	sleep(1);
}

