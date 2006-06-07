#ifndef SortsCanvas_h
#define SortsCanvas_h
#ifdef USE_CANVAS // in Sorts.h

#include <map>

#include "GameObj.H"

#include "SDLCanvas.h"

using namespace std;

struct CanvasObjInfo {
  SDLCanvasCompound*     compound;
  SDLCanvasCircle*       mainCircle;
  SDLCanvasTrackedShape* tracker;
  SDLCanvasStableLine*   trackingLine;
  SDLColor               origColor;
  int                    flashColorCycles;

  CanvasObjInfo() : origColor(255, 255, 255) {
    compound = NULL;
    mainCircle = NULL;
    tracker = NULL;
    trackingLine = NULL;
    flashColorCycles = 0;
  }
};

class SortsCanvas {
public:
  SortsCanvas();

  void init(double ww, double wh, double scale);
  bool initted();
  void clear();
  void registerGob(GameObj* gob);
  void unregisterGob(GameObj* gob);

  void setColor(GameObj* gob, Uint8 r, Uint8 g, Uint8 b);
  void flashColor(GameObj* gob, Uint8 r, Uint8 g, Uint8 b, int cycles);
  void update();

  void trackDestination(GameObj* gob, double destx, double desty);
  void stopTracking(GameObj* gob);

  bool gobRegistered(GameObj* gob);

  SDLCanvasCircle* makeTempCircle(double cx, double cy, double r, int t) {
    return canvas.makeTempCircle(cx, cy, r, t);
  }

private:
  SDLCanvas canvas;
public:
  map<GameObj*, CanvasObjInfo> canvasObjs;
  int updateCounter;
};

#endif // def USE_CANVAS
#endif
