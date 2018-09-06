#ifndef ANDERSONTHERMOSTAT_H
#define ANDERSONTHERMOSTAT_H

class MDParticleList;
class MDSim;

class AndersonThermostat{

public:
	AndersonThermostat(MDSim*, double, double);
	~AndersonThermostat();

	void setT(double);
	double getT();
	void setNue(double);
	double getNue();
    void execute();

private:
	double temp, nue, p;
	MDSim* sim;

};

#endif
