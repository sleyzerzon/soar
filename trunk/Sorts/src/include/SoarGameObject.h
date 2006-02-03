#ifndef _FSM_H_
#define _FSM_H_
#include"FSM.h"
#endif

#include<stack>

//How do I transition from one FSM to another?

class SoarGameGroup;

class SoarGameObject{
 public:
	SoarGameObject();
	~SoarGameObject();

	void registerBehavior(FSM *);
	void removeBehavior(std::string cmd);

	void issueCommand(std::string name);
	void update();

	void setGroup(SoarGameGroup *);
	SoarGameGroup *getGroup();

  // I assume this can be public?

 private:
	std::list<FSM *> behaviors;
	std::stack<FSM *> memory;

  	SoarGameGroup* group;

};
