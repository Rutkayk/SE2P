/*
 * Thread.cpp
 *
 *  Created on: 02.10.2013
 *      Author: Jannik Schick (2063265)
 *              Philipp Kloth (2081738)
 *              Rutkay Kuepelikilinc (2081831)
 *              Natalia Duske (2063265)
 */

#include "Thread.h"
#include <iostream>
#include <unistd.h>
#include <inttypes.h>
#include "HWaccess.h"
#include "HALAktorik.h"
#include "Test.h"
#include "Serial.h"

#define SLEEP_TWO_SEC 2

using namespace std;

namespace thread {

//Default Konstruktor der Klasse Thread
Thread::Thread() {
	// TODO Auto-generated constructor stub

}

// Default Destruktor Klasse Thread
Thread::~Thread() {
	// TODO Auto-generated destructor stub
}

void Thread::shutdown() {
	HALAktorik* hal = HALAktorik::getInstance();
	Serial* serial = Serial::getInstance();
	cout << "Shutting down..." << endl;
	hal->engine_stop();
	serial->close_serial();

}

void Thread::execute(void *arg) {
	HALAktorik* hal = HALAktorik::getInstance();
	Serial* serial = Serial::getInstance();

	MachineState* M = MachineState::getInstance();
	if (serial->open_serial("/dev/ser1") < 0) {
		perror("open call failed");
	}

	Test test;
	//serial->start(NULL);

	//hal->engine_rigth();
	while (!isStopped()) {
//		test.componentTest(hal);
//		cout << "5" << endl;
//		sleep(1);
//		cout << "4" << endl;
//		sleep(1);
//		cout << "3" << endl;
//		sleep(1);
//
//		test.serialTest(serial);
//
//		cout << "wait 10 sec" << endl;
		sleep(10);
	}
}

}
