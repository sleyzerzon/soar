#include "WMIdentifierCounter.h"
#include "IntAttribRange.h"
#include "StringAttribSet.h"

void WMIdentifierCounter::setup() {

  // interval definitions
  IntAttribRange* myOwnerRange    = new IntAttribRange("[0,0]");
  IntAttribRange* enemyOwnerRange = new IntAttribRange("[1,1]");

  IntAttribRange* baseHealthLow  = new IntAttribRange("[1,999]");
  IntAttribRange* baseHealthMed  = new IntAttribRange("[1000,2999]");
  IntAttribRange* baseHealthHigh = new IntAttribRange("[3000,4000]");

  IntAttribRange* tankHealthLow  = new IntAttribRange("[1,49]");
  IntAttribRange* tankHealthMed  = new IntAttribRange("[50,99]");
  IntAttribRange* tankHealthHigh = new IntAttribRange("[100,150]");


  // counters for the enemy tanks, discriminating by health
  map<string, WMAttribValueSet*> enemyTankPrototype;
  enemyTankPrototype["type"] = new StringAttribSet("tank");
  enemyTankPrototype["owner"] = enemyOwnerRange;

  map<string, WMAttribValueSet*> enemyTankAlive(enemyTankPrototype);
  addPattern("eTankAlive", IdPattern("groups.group", enemyTankAlive, "num-members"));

  map<string, WMAttribValueSet*> enemyTankHealthLow(enemyTankPrototype);
  enemyTankHealthLow["health"] = tankHealthLow;
  addPattern("eTankHealthLow", IdPattern("groups.group", enemyTankHealthLow, "num-members"));

  map<string, WMAttribValueSet*> enemyTankHealthMed(enemyTankPrototype);
  enemyTankHealthMed["health"] = tankHealthMed;
  addPattern("eTankHealthMed", IdPattern("groups.group", enemyTankHealthMed, "num-members"));

  map<string, WMAttribValueSet*> enemyTankHealthHigh(enemyTankPrototype);
  enemyTankHealthHigh["health"] = tankHealthHigh;
  addPattern("eTankHealthHigh", IdPattern("groups.group", enemyTankHealthHigh, "num-members"));
  
  // counters for my tank, discriminating by health
  map<string, WMAttribValueSet*> myTankAlive(enemyTankAlive);
  myTankAlive["owner"] = myOwnerRange;
  addPattern("mTankAlive", IdPattern("groups.group", myTankAlive, "num-members"));
  
  map<string, WMAttribValueSet*> myTankHealthLow(enemyTankHealthLow);
  myTankHealthLow["owner"] = myOwnerRange;
  addPattern("mTankHealthLow", IdPattern("groups.group", myTankHealthLow, "num-members"));

  map<string, WMAttribValueSet*> myTankHealthMed(enemyTankHealthMed);
  myTankHealthMed["owner"] = myOwnerRange;
  addPattern("mTankHealthMed", IdPattern("groups.group", myTankHealthMed, "num-members"));

  map<string, WMAttribValueSet*> myTankHealthHigh(enemyTankHealthHigh);
  myTankHealthHigh["owner"] = myOwnerRange;
  addPattern("mTankHealthHigh", IdPattern("groups.group", myTankHealthHigh, "num-members"));

  // counters for enemy bases, discriminating by health
  map<string, WMAttribValueSet*> enemyBasePrototype;
  enemyBasePrototype["type"] = new StringAttribSet("controlCenter");
  enemyBasePrototype["owner"] = enemyOwnerRange;

  map<string, WMAttribValueSet*> enemyBaseAlive(enemyBasePrototype);
  addPattern("eBaseAlive", IdPattern("groups.group", enemyBaseAlive, "num-members"));

  map<string, WMAttribValueSet*> enemyBaseHealthLow(enemyBasePrototype);
  enemyBaseHealthLow["health"] = baseHealthLow;
  addPattern("eBaseHealthLow", IdPattern("groups.group", enemyBaseHealthLow, "num-members"));

  map<string, WMAttribValueSet*> enemyBaseHealthMed(enemyBasePrototype);
  enemyBaseHealthMed["health"] = baseHealthMed;
  addPattern("eBaseHealthMed", IdPattern("groups.group", enemyBaseHealthMed, "num-members"));

  map<string, WMAttribValueSet*> enemyBaseHealthHigh(enemyBasePrototype);
  enemyBaseHealthHigh["health"] = baseHealthHigh;
  addPattern("eBaseHealthHigh", IdPattern("groups.group", enemyBaseHealthHigh, "num-members"));

  // counters for my base, discriminating by health
  map<string, WMAttribValueSet*> myBaseAlive(enemyBaseAlive);
  myBaseAlive["owner"] = myOwnerRange;
  addPattern("mBaseAlive", IdPattern("groups.group", myBaseAlive, "num-members"));
  
  map<string, WMAttribValueSet*> myBaseHealthLow(enemyBaseHealthLow);
  myBaseHealthLow["owner"] = myOwnerRange;
  addPattern("mBaseHealthLow", IdPattern("groups.group", myBaseHealthLow, "num-members"));

  map<string, WMAttribValueSet*> myBaseHealthMed(enemyBaseHealthMed);
  myBaseHealthMed["owner"] = myOwnerRange;
  addPattern("mBaseHealthMed", IdPattern("groups.group", myBaseHealthMed, "num-members"));

  map<string, WMAttribValueSet*> myBaseHealthHigh(enemyBaseHealthHigh);
  myBaseHealthHigh["owner"] = myOwnerRange;
  addPattern("mBaseHealthHigh", IdPattern("groups.group", myBaseHealthHigh, "num-members"));

  // counts the number of my bases that are taking damage
  map<string, WMAttribValueSet*> myBaseAttacked;
  myBaseAttacked["owner"] = myOwnerRange;
  myBaseAttacked["taking-damage"] = new IntAttribRange("[1,1]");
  addPattern("mBaseAttacked", IdPattern("groups.group", myBaseAttacked, "num-members"));
}
