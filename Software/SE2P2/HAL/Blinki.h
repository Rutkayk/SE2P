/*
 * Blinki.h
 *
 *  Created on: 27.11.2013
 *      Author: Jannik
 */

#ifndef BLINKI_H_
#define BLINKI_H_

#include <stdint.h>
#include "../HAW/HAWThread.h"
#include "../Mutex/Mutex.h"
#include "../HAW/HWaccess.h"
#include "MachineState.h"
#include "Addresses.h"
#include "Dispatcher.h"
#include <unistd.h>
#include <stdint.h>
#include <time.h>

namespace hal{

class Blinki: public thread::HAWThread  {

private:
public:
	Blinki();
	~Blinki();
	void flashYellow();
	void flashRedSlow();
	void flashRedFast();
	void flashGreenIni();

protected:
	virtual void execute(void* arg);
	virtual void shutdown();


};
}
#endif /* BLINKI_H_ */
