#ifndef MDPARTICLE_H
#define MDPARTICLE_H

#ifndef VEC_H
#include "vec.h"
#endif

class MDParticleList;

class MDParticle{

public:
        MDParticle(int);
        MDParticle(vec, vec);
        ~MDParticle();

        int dim;
        vec r, v, a;
        MDParticleList* verletList;
};

#endif
