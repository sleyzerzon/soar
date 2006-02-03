#include"include/SoarGameObject.h"
#include<iostream>

SoarGameObject::SoarGameObject()
{

}

SoarGameObject::~SoarGameObject()
{
 FSM* tmp;

 while(!memory.empty())
  memory.pop();

 while(!behaviors.empty())
 {
  tmp = behaviors.front();
  behaviors.pop_front();
  delete tmp;
 }
}

void SoarGameObject::registerBehavior(FSM *b)
{
 behaviors.push_back(b);
}

void SoarGameObject::removeBehavior(std::string name)
{
 std::list<FSM*>::iterator it;
 FSM *tmp;

 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name.compare(0,name.length(),name))
  {
   tmp = (*it);
   behaviors.erase(it);
   delete tmp;
   break;
  }
}


void SoarGameObject::issueCommand(std::string cmd)
{
 std::list<FSM*>::iterator it;

 //Whether we really want this is up for analysis
 while(!memory.empty())
  memory.pop();
 
 for(it = behaviors.begin(); it != behaviors.end(); it++)
  if((*it)->name == cmd) 
  {
   memory.push((*it));
   return;
  }
  std::cout<<"No match for command"<<std::endl;
}


void SoarGameObject::update()
{
 if(!memory.empty())
  memory.top()->update();
}


void SoarGameObject::setGroup(SoarGameGroup *g)
{
 group = g;
}

SoarGameGroup *SoarGameObject::getGroup(void)
{
 return group;
}
