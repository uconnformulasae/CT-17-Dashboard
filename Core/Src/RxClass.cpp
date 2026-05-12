/*
 * RxClass.cpp
 *
 *  Created on: Apr 18, 2025
 *      Author: Phoenix Cardwell
 *      UCONN Formula SAE 2025
 */

#include <main.h>
#include <RxClass.h>

#if 0
void Rx_oilPressure :: _Update(int val)
{
	int eopA = hcan->RxData[0];
	int eopB = hcan->RxData[1];
	_val = (((eopA * 256) + eopB) * 0.0135037738); // oil pressure in psi
}

void Rx_ECT::_Update(int val)
{
	int tempA = hcan->RxData[2];
	int tempB = hcan->RxData[3];
	_val = ((tempA * 256) + tempB) / 10; // temp in C
}

void Rx_RPM :: _Update(int val)
{
	int rpmA = hcan->RxData[0];
	int rpmB = hcan->RxData[1];
	ENGINE_RPM = ((rpmA * 256) + rpmB);
}
#endif
