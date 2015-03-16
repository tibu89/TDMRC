#include "DetectLeaks.h"
#include "Direct_Access_Image.h"
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

	void import(KImage *import, float multiplicator);

	void import(char *file_name);

	void SaveAs(char *file_name);

	void SaveAs(KImage *import, float multiplicator);

	ElementalDust& operator= (ElementalDust b_flt);

private:
	void sortParticles();

	void clear();
};