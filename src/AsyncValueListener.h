/*
 * AsyncValueListener.h
 *
 *  Created on: Aug 13, 2020
 *      Author: AlexCamilo
 */

#ifndef LIBRARIES_RBE1001LIB_SRC_ASYNCVALUELISTENER_H_
#define LIBRARIES_RBE1001LIB_SRC_ASYNCVALUELISTENER_H_

#include <Arduino.h>

class AsyncValueListener {
public:
	virtual void valueChanged(String name, float value)=0;
};

#endif /* LIBRARIES_RBE1001LIB_SRC_ASYNCVALUELISTENER_H_ */
