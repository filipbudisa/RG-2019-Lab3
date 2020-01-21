#ifndef VULK_ITIMEBOUND_H
#define VULK_ITIMEBOUND_H

class ITimeBound {
public:
	virtual void update(double time) = 0;
};

#endif //VULK_ITIMEBOUND_H
