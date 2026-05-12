/*
 * RxClass.h
 *
 *  Created on: Apr 18, 2025
 *      Author: Phoenix Cardwell
 *      UCONN Formula SAE 2025
 */

#ifndef INC_RXCLASS_H_
#define INC_RXCLASS_H_

#include <main.h>
#ifdef __cplusplus
#include <cstdint>
class Rx_TypeDef;

typedef enum{
	TIME,
	FLOAT,
	INT
} DATA_TYPES;
class Rx_TypeDef{
public:
	Rx_TypeDef() {}
	Rx_TypeDef(uint16_t ID, DATA_TYPES type = INT, const char* label = "") : _ID(ID),
																  _type(type),
																  _label(label) { _val = 0; _float = 0;}
	~Rx_TypeDef() {}
	uint32_t getID() { return _ID; }
	uint32_t getValue() const { return _val; }
	float getFloat() const {return _float; }
	const char* getLabel() const { return _label; }
	DATA_TYPES getType() const { return _type; }

	void _Update(int val) { _val = val; }
	void _UpdateFloat(float flt) { _float = flt; }
private:
	int _val;
	float _float;
	uint32_t _ID;
	DATA_TYPES _type;
	const char* _label;
};

#endif
#endif /* INC_RXCLASS_H_ */
