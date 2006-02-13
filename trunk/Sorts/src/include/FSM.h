#ifndef FSM_H
#define FSM_H

#include<map>
#include<string>
#include<list>
#include<vector>

#include "general.h"
#include "GameObj.H"
#include "SoarAction.h"



class FSM{
 public:
	FSM();
	virtual ~FSM();

	virtual bool update()=0;

	virtual void setGameObject(GameObj *g){gob = g;}
	virtual GameObj *getGameObject(){return gob;}
	virtual void setParams(std::vector<sint4>);

 //private:
	SoarActionType name;
	GameObj *gob;
	Vector<sint4> params;
};

#endif
