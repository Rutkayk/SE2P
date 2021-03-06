/*
 * HALSensorik.cpp
 *
 *  Created on: 24.10.2013
 *      Author: Jannik Schick (2063265)
 *              Philipp Kloth (2081738)
 *              Rutkay Kuepelikilinc (2081831)
 *              Natalia Duske (2063265)
 */

#include "HALSensorik.h"
#include <stdio.h>

static int isr_coid;
hal::HALSensorik* hal::HALSensorik::instance = NULL;
Mutex* hal::HALSensorik::HAL_Smutex = new Mutex();
/**
 * boolen for Port Status
 */


const struct sigevent* ISR(void* arg, int id) {
	struct sigevent *event = (struct sigevent *) arg;
	uint16_t val;
	uint8_t state;
	state = in8(DIO_BASE + OFFS_INT_STATUS) & (BIT_1 | BIT_3);//read
	out8(DIO_BASE + OFFS_INT_STATUS, 0);//reset

	/**
	 * switch case check where the bits are changend
	 * BIT_1 for Port B and BIT_3 for Port C
	 */
	switch (state) {
	case (BIT_1 | BIT_3):
		val = (in8(DIO_BASE + DIO_OFFS_B) << 8) | (in8(DIO_BASE + DIO_OFFS_C));
		SIGEV_PULSE_INIT(event,isr_coid,SIGEV_PULSE_PRIO_INHERIT, PB_STATUS | PC_STATUS, val);
		break;
	case BIT_1:
		val = in8(DIO_BASE + DIO_OFFS_B);
		SIGEV_PULSE_INIT(event,isr_coid,SIGEV_PULSE_PRIO_INHERIT, PB_STATUS, val);
		break;
	case BIT_3:
		val = in8(DIO_BASE + DIO_OFFS_C);
		SIGEV_PULSE_INIT(event,isr_coid,SIGEV_PULSE_PRIO_INHERIT, PC_STATUS, val);
		break;
	default:
		event = NULL;
	}
	return event;
}

hal::HALSensorik::HALSensorik() {
	initInterrupts();
	//create channel for dispatcher
	signalChid = ChannelCreate(0);
	if (signalChid == -1) {
		perror("Dispatcher: ChannelCreate signalChid failed");
		exit(EXIT_FAILURE);
	}
	//creats connection between PC Channel and Prozess
	signalCoid = ConnectAttach(0, 0, signalChid, _NTO_SIDE_CHANNEL, 0);
	if (signalCoid == -1) {
		perror("Dispatcher: ConnectAttach signalCoid failed");
		exit(EXIT_FAILURE);
	}

}

hal::HALSensorik* hal::HALSensorik::getInstance() {
	HAL_Smutex->lock();
	if (instance == NULL) {
		// Zugriffsrechte fuer den Zugriff auf die HW holen
		if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
			perror("ThreadCtl access failed\n");
			return NULL;
		}
		instance = new HALSensorik();
	}
	HAL_Smutex->unlock();

	return instance;
}

hal::HALSensorik::~HALSensorik() {
	delete instance;
	instance = NULL;
}

void hal::HALSensorik::initInterrupts() {
	// create channel to recive pulse message from the ISR
	isrChid = ChannelCreate(0);
	if (isrChid == -1) {
		perror("SensorikIntro: ChannelCreate isrChid failed");
		exit(EXIT_FAILURE);
	}
	isr_coid = ConnectAttach(0, 0, isrChid, _NTO_SIDE_CHANNEL, 0);
	if (isr_coid == -1) {
		perror("SensorikIntro: ConnectAttach isr_coid failed");
		exit(EXIT_FAILURE);
	}

	// attach ISRF
	SIGEV_PULSE_INIT(&event, isr_coid, SIGEV_PULSE_PRIO_INHERIT,0,0);
	interruptId = InterruptAttach(11, ISR, &event, sizeof(event), 0);
	if (interruptId == -1) {
		perror("SensorikIntro: ConnectAttach failed");
		exit(EXIT_FAILURE);
	}

	// configure interrupts

	//reset irq status register
	out8(DIO_BASE + OFFS_INT_STATUS, 0);
	// disable interrupts for all ports (Bit 0-5)
	uint8_t int_ctrl = in8(DIO_BASE + OFFS_INT_CTRL);
	out8(DIO_BASE + OFFS_INT_CTRL, int_ctrl | 0b00111111);
	// enable interrupt for portB und portC (Bit 1 und 2) (S. 18)
	int_ctrl = in8(DIO_BASE + OFFS_INT_CTRL);
	out8(DIO_BASE + OFFS_INT_CTRL, int_ctrl & ~(PB_CTRL | PC_CTRL));

	// read out port B and C values
	portBstatus = in8(DIO_BASE + DIO_OFFS_B);
	portCstatus = in8(DIO_BASE + DIO_OFFS_B);

}
/**
 * Detach the Connection and Interrupt
 * Destroy the Channel
 */
void hal::HALSensorik::stop() {
	HAWThread::stop(); // super.stop();

	if (ConnectDetach(isr_coid)) {
		perror("SensorCtrl: ConnectDetatch isr_coid failed");
	}
	if (ChannelDestroy(isrChid) == -1) {
		perror("SensorCtrl: ChannelDestroy isrChid failed");
	}
	//in Simulation: bleibt hier haengen
	if (InterruptDetach(interruptId) == -1) {
		perror("SensorCtrl: InterruptDetach failed");
	}

	if (-1 == ConnectDetach(signalCoid)) {
		perror("SensorCtrl: ConnectDetach ConnectDetach failed");
	}
	if (-1 == ChannelDestroy(signalChid)) {
		perror("SensorCtrl: ChannelDestroy signalChid failed");
	}

}

void hal::HALSensorik::shutdown() {
	cout << "shutdown HALSensorik..." << endl;
}

void hal::HALSensorik::execute(void *arg) {
	struct _pulse pulse;
	while (!isStopped()) {
		if (-1 == MsgReceivePulse(isrChid, &pulse, sizeof(pulse), NULL)) {
			if (isStopped()) {
				break; // channel destroyed, Thread ending
			}
			perror("SensorCtrl: MsgReceivePulse");
			exit(EXIT_FAILURE);
		}

		MsgSendPulse(signalCoid, SIGEV_PULSE_PRIO_INHERIT, pulse.code, pulse.value.sival_int);


	}
}

/**
 * Write 0x10 on Register 0x02
 * waits till  Bit 7 gets tall and read value from 0x02
 * @return the tall
 *
 */
int hal::HALSensorik::getHeight() {
	int heigth = -1;
	int i;
	out8(AIO_PORT_A, AIO_GET_VAL);
	for (i = 0; 1 < 50; i++) {
		if (in8(AIO_BASE) & 1 << 7) {
			heigth = in16(AIO_PORT_A);
			break;
		}
	}
	return heigth;

}
int hal::HALSensorik::getSignalChid(){
	return signalChid;
}

int hal::HALSensorik::getSignalCoid(){
	return signalCoid;
}
