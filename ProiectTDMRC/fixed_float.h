#ifndef __FIXED_FLOAT_H__
#define __FIXED_FLOAT_H__

class fixed_float
{
public:
	//first 8 bits = integer, last 8 bits = fractionary
	unsigned short bits; 

	fixed_float(unsigned char integer_uchar = 0, unsigned char fractional_uchar = 0);

	fixed_float& operator= (float b_flt);

	float operator* (float b_flt);

	float operator/ (float b_flt);

	bool operator< (const fixed_float &b_flt);

	bool operator> (const fixed_float &b_flt);

	bool operator<= (const fixed_float &b_flt);

	bool operator>= (const fixed_float &b_flt);

	bool operator!= (const fixed_float &b_flt);

	bool operator== (const fixed_float &b_flt);

	float getFloat();

private:
	float getSubunitry();
};

#endif