#ifndef VULK_IOBJECTMODIFIER_H
#define VULK_IOBJECTMODIFIER_H

#include "../world/WorldObject.h"
#include "ITimeBound.h"

class WorldObject;

class IObjectModifier : public ITimeBound {
public:
	IObjectModifier(WorldObject* obj){
		this->object = obj;
	}


//protected:
	WorldObject *object;
};

#endif //VULK_IOBJECTMODIFIER_H
