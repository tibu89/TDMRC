#include "fixed_float.h"


fixed_float::fixed_float(unsigned char integer_uchar, unsigned char fractional_uchar)
{
	float flt = integer_uchar*256.0f + fractional_uchar;
	bits = (unsigned short)flt;
}

fixed_float& fixed_float::operator=(float b)
{
	//TO DO:verify
	bits = ((unsigned short)(b*256.0f));

	return *this;
}

float fixed_float::operator* (float b_flt)
{
	return getFloat() * b_flt;
}

float fixed_float::operator/ (float b_flt)
{
	return getFloat() / b_flt;
}

bool fixed_float::operator<(const fixed_float &b_fflt)
{
	return bits < b_fflt.bits;
}

bool fixed_float::operator>(const fixed_float &b_fflt)
{
	return bits > b_fflt.bits;
}

bool fixed_float::operator<=(const fixed_float &b_fflt)
{
	return bits <= b_fflt.bits;
}

bool fixed_float::operator>=(const fixed_float &b_fflt)
{
	return bits >= b_fflt.bits;
}

bool fixed_float::operator!=(const fixed_float &b_fflt)
{
	return bits != b_fflt.bits;
}

bool fixed_float::operator==(const fixed_float &b_fflt)
{
	return bits == b_fflt.bits;
}

float fixed_float::getFloat()
{
	float ret;
	ret = ((float)bits)/256.0f;
	return ret;
}