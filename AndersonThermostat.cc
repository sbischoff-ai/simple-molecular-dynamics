#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "vec.h"
#include "MDSim.h"
#include "AndersonThermostat.h"
#include <cmath>
#include <cstdint>
#include <iostream>
//using namespace std;

const double TWOPI = 2 * M_PI;

class MDParticleList;
class MDParticleListEntry;

double mersenneTwister(){
#define N     624
#define M     397
#define HI    0x80000000
#define LO    0x7fffffff
#define MAX   0xFFFFFFFF
	static const uint32_t seed = 5489UL;
	static const uint32_t A[2] = { 0, 0x9908b0df };
	static uint32_t y[N];
	static int i = N + 1;

	if (i > N) {
		/* Initialisiere y mit Pseudozufallszahlen */
		y[0] = seed;

		for (i = 1; i < N; ++i) {
			y[i] = (1812433253UL * (y[i - 1] ^ (y[i - 1] >> 30)) + i);
		}
	}

	if (i == N) {
		/* Berechne neuen Zustandsvektor */
		uint32_t h;

		for (i = 0; i < N - M; ++i) {
			h = (y[i] & HI) | (y[i + 1] & LO);
			y[i] = y[i + M] ^ (h >> 1) ^ A[h & 1];
		}
		for (; i < N - 1; ++i) {
			h = (y[i] & HI) | (y[i + 1] & LO);
			y[i] = y[i + (M - N)] ^ (h >> 1) ^ A[h & 1];
		}

		h = (y[N - 1] & HI) | (y[0] & LO);
		y[N - 1] = y[M - 1] ^ (h >> 1) ^ A[h & 1];
		i = 0;
	}

	uint32_t e = y[i++];
	/* Tempering */
	e ^= (e >> 11);
	e ^= (e << 7) & 0x9d2c5680;
	e ^= (e << 15) & 0xefc60000;
	e ^= (e >> 18);

	return (double)e / MAX;
#undef N
#undef M
#undef HI
#undef LO
}


AndersonThermostat::AndersonThermostat(MDSim* sim_, double temp_, double nue_) : sim(sim_), temp(temp_), nue((-1)*nue_), p(1 - exp((-1)*nue_*sim_->getDt())) {};

void AndersonThermostat::setT(double temp_){
	this->temp = (-2.0)*temp_;
}

double AndersonThermostat::getT(){
	return (-0.5)*this->temp;
}

void AndersonThermostat::setNue(double nue_){
	this->nue = (-1.0)*nue_;
	this->p = 1 - exp(this->nue*this->sim->getDt());
}

double AndersonThermostat::getNue(){
	return (-1.0)*this->nue;
}

void AndersonThermostat::execute(){

	if (this->sim->pause) return;

	MDParticleListEntry* entry;
	for (entry = this->sim->particles->getFirst(); entry; entry = entry->getNext()) if (mersenneTwister() < this->p) {
		double u1 = log(1.0 - mersenneTwister());
		double u2 = TWOPI * mersenneTwister();
		double s = sin(u2), c = cos(u2);
		if (this->sim->getDim() == 2){
			entry->getThis()->v[1] = sqrt(this->temp*u1)*c;
			entry->getThis()->v[2] = sqrt(this->temp*u1)*s;
		}
		if (this->sim->getDim() == 3){
			double u3 = M_PI * mersenneTwister();
			double s_ = sin(u3), c_ = cos(u3);
			entry->getThis()->v[1] = sqrt(this->temp*u1)*s_*c;
			entry->getThis()->v[2] = sqrt(this->temp*u1)*s_*s;
			entry->getThis()->v[3] = sqrt(this->temp*u1)*c_;
		}
	}
}
