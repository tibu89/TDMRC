#include "stdafx.h"
#include "particle.h"
#include "fixed_float.h"

class ElementalDust
{
	int nr_particles_int;
	particle *partiles_prt;

public:
	ElementalDust();
	~ElementalDust();

	void import(char *file_name);

	void import(void *file_contents, size_t size);

	void SaveAs(char *file_name);

	ElementalDust& operator= (ElementalDust b_flt);

	bool operator==(ElementalDust &b_flt);

	bool operator!=(ElementalDust &b_flt);

	void sortParticles();

private:
	void clear();
};