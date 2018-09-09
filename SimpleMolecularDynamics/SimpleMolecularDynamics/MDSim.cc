#include <cmath>
#include <iostream>
#include "vec.h"
#include "mat.h"
#include "MDParticle.h"
#include "MDParticleList.h"
#include "MDParticleListEntry.h"
#include "AndersonThermostat.h"
#include "MDSim.h"
using namespace std;

MDParticle::MDParticle(int dim) : r(0, dim), v(0, dim), a(0, dim), verletList(new MDParticleList()) {}

MDParticle::MDParticle(vec r_, vec v_) : dim(r_.get_dim()), r(r_), v(v_), verletList(new MDParticleList()) {}

MDParticle::~MDParticle(){

}

// Deleting a list also deletes its entries
MDParticleList::~MDParticleList(){
	this->clear();
}

int MDParticleList::getLength(){
	return length;
}

MDParticleListEntry* MDParticleList::getFirst(){
	return first;
}

MDParticleListEntry* MDParticleList::getLast(){
	return last;
}

void MDParticleList::addParticle(MDParticle* particle){
	if (this->last) this->last = (this->last->next = new MDParticleListEntry(particle, this, this->last));
	else this->last = (this->first = new MDParticleListEntry(particle, this, this->last));
	this->length++;
}

void MDParticleList::clear(){
	while (this->first) delete this->first; // ParticleListEntry destructor handles everything (see below).
}

// Deleting a list entry keeps list intact.
MDParticleListEntry::~MDParticleListEntry(){
	if (this->prior) this->prior->next = this->next;
	if (this->next) this->next->prior = this->prior;
	if (!this->prior) list->first = this->next;
	if (!this->next) list->last = this->prior;
	this->list->length--;
}

MDParticleListEntry* MDParticleListEntry::getNext(){
	return next;
}

MDParticleListEntry* MDParticleListEntry::getPrior(){
	return prior;
}

MDParticle* MDParticleListEntry::getThis(){
	return thisParticle;
}

bool MDParticleListEntry::isFirst(){
	return !prior;
}

bool MDParticleListEntry::isLast(){
	return !next;
}

MDSim::MDSim(int dim_, double dt_, vec(*f_)(vec) = 0, double(*pot_)(vec) = 0, double r_inter_ = 0, double r_verlet_ = 0, int verletUpdate_ = 0) :
dim(dim_), dt(dt_), t(0), f(f_), pot(pot_), r_inter(r_inter_), r_verlet(r_verlet_), pause(false), particles(new MDParticleList()), radial(0, 201), histogramResolution(201),
verletUpdate(verletUpdate_), verletSteps(0), directional(201, 201), thermo(0), graphDataFirst(0), graphDataLast(0), periodic(false), simBox(0, dim_)
{
	this->thermo = new AndersonThermostat(this, 0, 10);
};

void MDSim::initSim(bool periodic_, vec simBox_, vec(*r0)(int) = 0, vec(*v0)(int) = 0, int histogramResolution_ = 201, double histogramLength_ = 0.5){
	// restore starting conditions
	// **************************************************
	int i, j, k;
	this->histogramResolution = histogramResolution_;
	this->histogramLength = histogramLength_;
	this->simBox = simBox_;
	MDParticleListEntry* entry;
	if (r0){
		i = 0;
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->r = r0(i++);
	}
	else{
		double n = ceil(pow((double)this->particles->getLength(), 1.0/this->dim));
		entry = this->particles->getFirst();
		if(this->dim == 2) for (i = 0; i < n; i++) for (j = 0; j < n; j++) if (entry){
			MDParticle* particle = entry->getThis();
			particle->r[1] = (double)i * this->simBox[1] / n;
			if (j & 1) particle->r[1] += 1.0 / (2.0*n);
			particle->r[2] = (double)j * this->simBox[2] / n;
			entry = entry->getNext();
		}
		if (this->dim == 3) for (i = 0; i < n; i++) for (j = 0; j < n; j++) for (k = 0; k < n; k++) if (entry){
			MDParticle* particle = entry->getThis();
			particle->r[1] = (double)i * this->simBox[1] / n;
			particle->r[2] = (double)j * this->simBox[2] / n;
			particle->r[3] = (double)k * this->simBox[3] / n;
			entry = entry->getNext();
		}
	}
	if (v0){
		i = 0;
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->v = v0(i++);
	}
	else{
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->v = vec(0, this->dim);
	}
	this->periodic = periodic_;
	this->t = 0;
	this->radial = vec(0, this->histogramResolution);
	this->directional = mat(this->histogramResolution, this->histogramResolution);
	this->ePot = (this->ePotMin = this->refreshVerletLists(true, true));
	// calculate kinetic energy and total momentum
	// vec p_ges(0, this->dim);
	this->eKin = 0;
	for (entry = this->particles->getFirst(); entry; entry = entry->getNext()){
		// p_ges += entry->getThis()->v;
		this->eKin += entry->getThis()->v * entry->getThis()->v;
	}
	this->eKin *= 0.5;
	this->resetGraphs();
	this->updateGraphs();
	this->pause = true;
}

void MDSim::velocityVerletStep(bool countRadial){
	if (!this->pause) {
		int i;
		// iterate through all particles in list
		MDParticleListEntry* entry;
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()){
			MDParticle* particle = entry->getThis();
			particle->v += 0.5*dt*particle->a; // half step v
			particle->r += dt*particle->v; // full step r with half new v
			// keep periodic boundary conditions
			if (this->periodic) for (i = 1; i <= this->dim; i++) particle->r[i] -= this->simBox[i] * floor(particle->r[i] / this->simBox[i]);
		}
		// if it's time (VerletSteps is "full") the verlet lists are updated
		// calculation of force then also takes place in list update
		if (this->verletSteps > this->verletUpdate) {
			this->ePot = this->refreshVerletLists(true, countRadial);
			this->verletSteps = 0;
		}
		else this->ePot = velocityVerletForce(); // new force with new r
		this->verletSteps++;
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->v += 0.5*dt*entry->getThis()->a; // rest step v with new force
		// calculate new kinetic energy and total momentum
		// vec p_ges(0, this->dim);
		this->eKin = 0;
		for (entry = this->particles->getFirst(); entry; entry = entry->getNext()){
			// p_ges += entry->getThis()->v;
			this->eKin += entry->getThis()->v * entry->getThis()->v;
		}
		this->eKin *= 0.5;
		this->t += this->dt;
	}
}

// has to be called after the velocityVerletStep function to extend the graphs by the last time step
void MDSim::updateGraphs(){
	if (this->pause) return;
	this->graphDataLast = (this->graphDataLast->next = new graphData_t);
	// The first list entry (graphDataFirst) contains the maximum values of the graphs (for normalization).
	// Assignment of new values and query for wether they are greater than last maximum can be combined:
	if ((this->graphDataLast->ePot = this->ePot) > this->graphDataFirst->ePot) this->graphDataFirst->ePot = this->graphDataLast->ePot;
	if ((this->graphDataLast->eKin = this->eKin) > this->graphDataFirst->eKin) this->graphDataFirst->eKin = this->graphDataLast->eKin;
	this->graphDataLast->next = 0;
	if (this->ePot < this->ePotMin) this->ePotMin = this->ePot;
}

void MDSim::resetGraphs(){
	graphData_t* graphData = this->graphDataFirst;
	if (graphData){
		graphData->ePot = (graphData->eKin = 0);
		graphData = graphData->next;
		while (graphData){
			graphData_t* temp = graphData;
			graphData = graphData->next;
			delete temp;
		}
		this->graphDataLast = this->graphDataFirst;
	}
	else{
		this->graphDataFirst = (this->graphDataLast = new graphData_t);
		this->graphDataFirst->eKin = (this->graphDataFirst->ePot = 0);
		this->graphDataFirst->next = 0;
	}
}

void MDSim::resetRadialDistribution(){
	this->radial *= 0;
}

vec MDSim::getRadialDistribution(){
	vec result = this->radial;
	result.normalize();
	return result;
}

int MDSim::getHistogramResolution(){
	return this->histogramResolution;
}

void MDSim::resetDirectionalDistribution(){
	this->directional *= 0;
}

mat MDSim::getDirectionalDistribution(){
	return this->directional / this->directional.get_max();
}

double MDSim::getDt(){
	return this->dt;
}

double MDSim::getT(){
	return this->t;
}

int MDSim::getDim(){
	return this->dim;
}

double MDSim::getEPotMin(){
	return this->ePotMin;
}

AndersonThermostat* MDSim::getThermostat(){
	return this->thermo;
}

double MDSim::refreshVerletLists(bool calc, bool countRadial){
	int i;
	double r_abs, pot_ = 0;
	vec r_, f_;
	MDParticleListEntry* entry;
	if (calc) for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->a *= 0;
	for (entry = this->particles->getFirst(); entry; entry = entry->getNext()){
		entry->getThis()->verletList->clear();
		MDParticleListEntry* verlet;
		for (verlet = entry->getNext(); verlet; verlet = verlet->getNext()) {
			r_ = entry->getThis()->r - verlet->getThis()->r;
			// nearest image convention:
			if (this->periodic) for (i = 1; i <= this->dim; i++) if (abs(r_[i]) > 0.5 * this->simBox[i]) r_[i] -= copysign(this->simBox[i], r_[i]);
			r_abs = r_.v_abs();
			if (countRadial) {
				double hr = this->histogramResolution;
				double hl = this->histogramLength;
				if (r_abs < hl) this->radial[(int)(r_abs/hl * hr) + 1]++;
				if (this->dim == 2) if ((abs(r_[1]) < hl) && (abs(r_[2]) < hl)){
					this->directional
						[(int)((r_[1] / (2.0 * hl) + 0.5)*hr) + 1]
						[(int)((r_[2] / (2.0 * hl) + 0.5)*hr) + 1]++;
					this->directional
						[(int)((r_[1] / (-2.0 * hl) + 0.5)*hr) + 1]
						[(int)((r_[2] / (-2.0 * hl) + 0.5)*hr) + 1]++;
				}
			}
			if (this->r_verlet > 0) if (r_abs > this->r_verlet) continue;
			entry->getThis()->verletList->addParticle(verlet->getThis());
			if (calc) if ((r_abs < this->r_inter) || r_inter == 0){
				f_ = this->f(r_);
				pot_ += this->pot(r_);
				entry->getThis()->a += f_; verlet->getThis()->a -= f_;
			}
		}
	}
	return pot_;
}

double MDSim::velocityVerletForce(){
	int i;
	double pot_ = 0, r_inter_2 = this->r_inter*this->r_inter;
	vec f_, r_;
	MDParticleListEntry* entry;
	for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) entry->getThis()->a *= 0; // reset force
	// address every pair of particles only once:
	for (entry = this->particles->getFirst(); entry; entry = entry->getNext()) {
		MDParticleListEntry* verlet;
		for (verlet = entry->getThis()->verletList->getFirst(); verlet; verlet = verlet->getNext()){
			r_ = entry->getThis()->r - verlet->getThis()->r;
			if (this->periodic) for (i = 1; i <= this->dim; i++) if (abs(r_[i]) > 0.5 * this->simBox[i]) r_[i] -= copysign(this->simBox[i], r_[i]);
			if (r_inter_2 > 0) if ((r_*r_) > r_inter_2) continue;
			f_ = this->f(r_);
			pot_ += this->pot(r_);
			entry->getThis()->a += f_; verlet->getThis()->a -= f_;
		}
	}
	return pot_;
}
