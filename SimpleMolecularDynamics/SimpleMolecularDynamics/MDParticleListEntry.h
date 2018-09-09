#ifndef MDPARTICLELISTENTRY_H
#define MDPARTICLELISTENTRY_H

class MDParticle;
class MDParticleList;

class MDParticleListEntry{

        friend class MDParticleList;

public:
        MDParticleListEntry(MDParticle* thisParticle_, MDParticleList* list_, MDParticleListEntry* prior_ = 0, MDParticleListEntry* next_ = 0) :
                thisParticle(thisParticle_), list(list_), prior(prior_), next(next_) {};
        ~MDParticleListEntry();

        MDParticleListEntry* getNext();
        MDParticleListEntry* getPrior();
        MDParticle* getThis();
        bool isLast();
        bool isFirst();

private:
        MDParticleList* list;
        MDParticle* thisParticle;
        MDParticleListEntry* next;
        MDParticleListEntry* prior;
};

#endif
