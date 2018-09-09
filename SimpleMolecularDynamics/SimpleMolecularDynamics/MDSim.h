#ifndef MDSIM_H
#define MDSIM_H

#ifndef VEC_H
#include "vec.h"
#endif

#ifndef MAT_H
#include "mat.h"
#endif

#ifndef MDPARTICLELIST_H
#include "MDParticleList.h"
#endif

#include "AndersonThermostat.h"

typedef struct graphData_t{
	double eKin, ePot;
	graphData_t* next;
} graphData_t;

class MDSim{

public:
	MDSim(int, double, vec(vec), double(vec), double, double, int);
	~MDSim() {};

	void initSim(bool, vec, vec(int), vec(int), int, double);
	void velocityVerletStep(bool);
	void updateGraphs();
	void resetGraphs();
	void resetRadialDistribution();
	vec getRadialDistribution();
	int getHistogramResolution();
	void resetDirectionalDistribution();
	mat getDirectionalDistribution();
	double getDt();
	double getT();
	int getDim();
	double getEPotMin();
	AndersonThermostat* getThermostat();

    MDParticleList* particles;
	bool pause;
	graphData_t *graphDataFirst, *graphDataLast; // graphDataFirst contains max values and leads to values for t=0

private:
	double refreshVerletLists(bool, bool);
	double velocityVerletForce();

	AndersonThermostat* thermo;
	double (*pot)(vec), dt, t, r_inter, r_verlet, eKin, ePot, ePotMin, histogramLength;
	vec (*f)(vec), simBox, radial;
    mat directional;
	int verletSteps, verletUpdate, dim, histogramResolution;
	bool periodic;
};

#endif
