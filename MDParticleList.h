#ifndef MDPARTICLELIST_H
#define MDPARTICLELIST_H

#ifndef MDPARTICLE_H
#include "MDParticle.h"
#endif

#ifndef MDPARTICLELISTENTRY_H
#include "MDParticleListEntry.h"
#endif

class MDParticleList{

        friend class MDParticleListEntry;

public:
        MDParticleList() :first(0), last(0), length(0){};
        ~MDParticleList();

        int getLength();
        MDParticleListEntry* getFirst();
        MDParticleListEntry* getLast();
        void addParticle(MDParticle*);
        void clear();

private:
        int length;
        MDParticleListEntry* last;
        MDParticleListEntry* first;
};

#endif
