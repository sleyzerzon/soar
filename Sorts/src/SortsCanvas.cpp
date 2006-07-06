#include "SortsCanvas.h"
#include "Sorts.h"

#define REDRAW_PERIOD 1

#define CLASS_TOKEN "CANVAS"
#define DEBUG_OUTPUT false 
#include "OutputDefinitionsUnique.h"

SortsCanvas::SortsCanvas() { 
  updateCounter = 0;
}

void SortsCanvas::init(double ww, double wh, double scale) {
  int status = canvas.init(ww, wh, scale);
  assert(status == 0);
  statusObj.compound = canvas.makeCompound(10,10);
  statusObj.mainShape = canvas.makeCircle(10,5,1);
  statusObj.mainShape->setLabel("initted");
  statusObj.mainShape->setShapeColor(198,226,255);
  statusObj.origColor = statusObj.mainShape->getShapeColor();
  statusObj.compound->addShape(statusObj.mainShape);
  soarStatObj.compound = canvas.makeCompound(10,10);
  soarStatObj.mainShape = canvas.makeCircle(10,25,1);
  soarStatObj.mainShape->setLabel("");
  soarStatObj.mainShape->setShapeColor(198,226,255);
  soarStatObj.origColor = soarStatObj.mainShape->getShapeColor();
  soarStatObj.compound->addShape(soarStatObj.mainShape);
  commandStatObj.compound = canvas.makeCompound(10,10);
  commandStatObj.mainShape = canvas.makeCircle(10,45,1);
  commandStatObj.mainShape->setLabel("");
  commandStatObj.mainShape->setShapeColor(198,226,255);
  commandStatObj.origColor = commandStatObj.mainShape->getShapeColor();
  commandStatObj.compound->addShape(commandStatObj.mainShape);
}

bool SortsCanvas::initted() {
  return canvas.initted();
}

void SortsCanvas::clear() {
  canvasObjs.clear();
  canvas.clear();
}

void SortsCanvas::registerSGO(SoarGameObject* sgo) {
  assert(canvasObjs.find(sgo) == canvasObjs.end());
  CanvasObjInfo newObj;
  newObj.compound = canvas.makeCompound(sgo->getX(), sgo->getY());
  if (sgo->isRectangle()) {
    newObj.mainShape = canvas.makeRectangle(sgo->getX(), sgo->getY(), 
                         sgo->getWidth(), sgo->getHeight(), 0);
  }
  else {
    newObj.mainShape = canvas.makeCircle(sgo->getX(), sgo->getY(), 
                                          sgo->getRadius());
  }
  if (sgo->isWorld()) {
    newObj.mainShape->setShapeColor(0,0,255);
  }
  else if (sgo->isFriendly()) {
    newObj.mainShape->setShapeColor(0,255,255);
  }
  else {
    newObj.mainShape->setShapeColor(255,0,255);
  }

//  newObj.mainShape->setLabel(sgo->getName().c_str());
  //stringstream ss;
  //PerceptualGroup* p = sgo->getPerceptualGroup();
  //if (p != NULL) {
  //  ss << sgo->getPerceptualGroup()->getDistToFocus();
  //}
  //newObj.mainShape->setLabel(ss.str());
  newObj.origColor = newObj.mainShape->getShapeColor();
  newObj.compound->addShape(newObj.mainShape);
  canvasObjs[sgo] = newObj;
}

void SortsCanvas::unregisterSGO(SoarGameObject* sgo) {
  assert(canvasObjs.find(sgo) != canvasObjs.end());
  CanvasObjInfo& obj = canvasObjs[sgo];
  list<SDLCanvasShape*> elements;
  obj.compound->getElements(elements);

  for(list<SDLCanvasShape*>::iterator
      i  = elements.begin();
      i != elements.end();
      ++i)
  {
    canvas.remove(*i);
  }
  canvas.remove(obj.compound);

  if (obj.trackingLine != NULL) {
    canvas.remove(obj.trackingLine);
  }
  canvasObjs.erase(sgo);
}

void SortsCanvas::registerGroup(PerceptualGroup* group) {
  assert(canvasGroups.find(group) == canvasGroups.end());
  int x, y;
  Rectangle r = group->getBoundingBox();

  int width = r.getWidth();
  int height = r.getHeight();
  r.getCenterPoint(x,y);
  
  stringstream ss;
  string label;
  ss << "id " << group->getSoarID();
  label = ss.str();
  
  CanvasGroupInfo newObj;
  newObj.compound = canvas.makeCompound(x,y);
  newObj.mainRectangle = canvas.makeRectangle(x, y, width, height, 0);
  newObj.mainRectangle->setLabel(label);
  if (group->isFriendly()) {
    newObj.mainRectangle->setShapeColor(0,255,0);
  }
  else {
    newObj.mainRectangle->setShapeColor(255,0,0);
  }
  // TODO: case for world objs
  newObj.compound->addShape(newObj.mainRectangle);
  canvasGroups[group] = newObj;
}

void SortsCanvas::unregisterGroup(PerceptualGroup* group) {
  assert(canvasGroups.find(group) != canvasGroups.end());
  CanvasGroupInfo& obj = canvasGroups[group];
  list<SDLCanvasShape*> elements;
  obj.compound->getElements(elements);

  for(list<SDLCanvasShape*>::iterator
      i  = elements.begin();
      i != elements.end();
      ++i)
  {
    canvas.remove(*i);
  }
  canvas.remove(obj.compound);

  canvasGroups.erase(group);
}

void SortsCanvas::resetSGO(SoarGameObject* sgo) {
  assert(canvasObjs.find(sgo) != canvasObjs.end());
  CanvasObjInfo& obj = canvasObjs[sgo];
  stopTracking(obj);
  //obj.origColor.set(255,255,255);
}
  


void SortsCanvas::setColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b) {
  assert(canvasObjs.find(sgo) != canvasObjs.end());
  canvasObjs[sgo].mainShape->setShapeColor(r, g, b);
  canvasObjs[sgo].origColor.set(r, g, b);
}

void SortsCanvas::flashColor(SoarGameObject* sgo, Uint8 r, Uint8 g, Uint8 b, int cycles) {
  assert(canvasObjs.find(sgo) != canvasObjs.end());
  CanvasObjInfo& canvasObj = canvasObjs.find(sgo)->second;
  canvasObj.flashColorCycles = cycles;
  canvasObj.mainShape->setShapeColor(r, g, b);
}

void SortsCanvas::update() {
  if (updateCounter < REDRAW_PERIOD) {
    ++updateCounter;
    return;
  }
  updateCounter = 0;

  for(map<SoarGameObject*,CanvasObjInfo>::iterator
      i  = canvasObjs.begin();
      i != canvasObjs.end();
      ++i)
  {
    SoarGameObject* sgo = i->first;
    // don't bother updating immobile things
    // if they die, unregisterSGO will get it

    // isMobile is set at constructor time, so watch out if things that are
    // immobile when first seen later become mobile
    if (sgo->isMobile()) {
      CanvasObjInfo& obj = i->second;
      obj.compound->moveTo(sgo->getX(), sgo->getY());
      dbg << "updating sgo " << sgo << ", type " << sgo->getName() 
          << " owned by " << sgo->getOwner() 
          << " located at " << sgo->getLocation() << endl;
      if (obj.flashColorCycles > 0) {
        --obj.flashColorCycles;
        if (obj.flashColorCycles == 0) {
          obj.mainShape->setShapeColor(obj.origColor);
        }
      }
    }
  }

  canvas.redraw();
}

void SortsCanvas::trackDestination(SoarGameObject* sgo,double destx,double desty) {
  assert(canvasObjs.find(sgo) != canvasObjs.end());
  CanvasObjInfo& obj = canvasObjs[sgo];
  stopTracking(obj);

  if (sgo->getX() < 0 || sgo->getY() < 0) {
    msg << "ERROR: sgo out of canvas!\n";
  }
  if (destx < 0 || desty < 0) {
    msg << "ERROR: dest out of canvas!\n";
  }
  SDLCanvasTrackedShape* t = canvas.makeTracker(sgo->getX(), sgo->getY(), destx, desty, true);
  obj.compound->addShape(t);
  obj.tracker = t;
  obj.trackingLine = t->getTrackingLine();
}

void SortsCanvas::stopTracking(CanvasObjInfo& obj) {
  if (obj.tracker != NULL) {
    obj.compound->removeShape(obj.tracker);
    canvas.remove(obj.tracker);
    canvas.remove(obj.trackingLine);
    obj.tracker = NULL;
    obj.trackingLine = NULL;
  }
}

bool SortsCanvas::sgoRegistered(SoarGameObject* sgo) {
  return (canvasObjs.find(sgo) != canvasObjs.end());
}

void SortsCanvas::drawLine(double x1, double y1, double x2, double y2) {
  canvas.makeLine(x1, y1, x2, y2);
}

void SortsCanvas::setStatus(string status) {
  statusObj.mainShape->setLabel(status);
  canvas.redraw();
}
void SortsCanvas::clearStatus() {
  statusObj.mainShape->setLabel("");
  canvas.redraw();
}
void SortsCanvas::setSoarStatus(string status) {
  soarStatObj.mainShape->setLabel(status);
  canvas.redraw();
}
void SortsCanvas::setCommandStatus(string status) {
  commandStatObj.mainShape->setLabel(status);
  canvas.redraw();
}
