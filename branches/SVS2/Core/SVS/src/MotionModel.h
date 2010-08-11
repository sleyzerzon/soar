
// Motion models are patterns of movement attached to objects in the scene
// Theoretically, they should be recognized, but can currently be
// deliberately attached
//
// Motion models have types, in LTM they exist as categories
// A motion model instantiated in STM (through a track-motion command)
// learns parameters based on how the object it is attached to moves
// (e.g., learns that allocentric translation for object X means 2units/dec.
// north)
//
// During tracking, the agent can specify whether the tracking process should
// propogate changes to LTM (if the model supports it), or only locally track
//
// A motion model can spawn a simulator, which is the forward-projection
// version of itself. They can attach to different objects, so they are
// different things.
//
// Motion models can be present that ignore all tracking (e.g., old SRS-type
// motions).
// 
// A simulation can be invoked based on a model in LTM or STM. 
//
// In addition, through a store-motion command, the agent can add new LTM
// motion models. These copy an existing LTM model (and its learned state).
// This may not make sense for non-learning models, but is syntactically valid.
//
// From the POV of the motion model, this distinction is unimportant, however.

class Simulation {
public:
  // this process updates the spatial scene (via the appropriate interface) in
  // arbitrary ways
  virtual bool updateSimulation(vector< Parameter > parameters, double time) = 0;
private:
  // arbitrary internal state is allowed here (but remember not to keep
  // SVSObject*'s)
};

class MotionModel {
public:
  MotionModel(bool _isTemporary) {
    temporary = _isTemporary;
  }
  ~MotionModel(); 

  // this is the interface new motions must implement:
  
  // startSession() tells the model to start tracking the provided object using
  // the given (model-specific) parameters
  // The current time (in terms of an environmental event counter) is provided.
  // Parameters referring to objects will already have been processed to
  // GroundedObjects.
  virtual void startSession(vector< Parameter > parameters,
                            double time) = 0;
  
  // Learn is called every time a new datapoint (may) available. The model is
  // responsible for taking into account the time difference since the
  // startSession() call or the last learn() call and handling it
  // appropriately. This difference may be zero,
  // if the agent is ignoring the environment.
  virtual void learn(vector< Parameter> parameters, double time) = 0;
  
  // Some models may require at least one datapoint before doing anything. This
  // queries the model to determine if a simulation is yet possible.
  virtual bool isReady() = 0;

  // Spawn a SpatialSimulator object (owned by the caller) instantiating the
  // motion
  virtual SpatialSimulator* createNewSimulation(vector< Parameter > parameters) = 0;


  // below here is handled by the base class, subclasses need not be concerned
  
  // temporary is true if the motion is in STM only, and state will be forgotten once
  // tracking ceases
  bool isSTMOnly() {
    return STMOnly;
  }

  // a given motion model can only handle one learning session at once,
  // MotionModelManager uses this to ensure only one is started
  bool isTracking() {
    return tracking;
  }


  void setId(string _id) {
    id = _id;
  }
  string getId() {
    return id;
  }
protected:
  bool STMOnly;
  string id;

};


// MotionControllers are a subset of motion models, that spawn
// ControllerInstances, which are a derived class of SpatialSimulator
//
// TODO: determine what a MotionSimulation should return, and how to get it to
// the environment

class ControllerInstance : public MotionSimulation {
  virtual vector< double > updateControl(vector< Parameter > parameters, double time) = 0;
};


class MotionController : public MotionModel {
  public:
    virtual ControllerInstance* createNewControllerInstance(vector< Parameter > parameters) = 0;
};

// Example complex learning application: learn how the cursor moves relative to the screen as a
// function of how the mouse moves relative to the table
//
// The tracking process would take the cursor as its object, and some
// parameters:
// target-motion-reference-object screen
// source-motion-object mouse
// source-motion-reference-object table
//
// These all show up in the learning process as GroundedObject parameters.
// The GroundedObjects are current at every learning step.
// 
// To learn the motion, at each update, the model would get the transformation
// from the desk to the mouse in the FOR of the desk, using relateInFrame in GeometryHelpers.h
//
// The model would also get the transformation from the screen to the pointer
// in the FOR of the screen using the same function.
//
// The learning process would determine how the quantities are varying relative
// to one another, somehow.
//
// Once enough data is present, the learning process would set ready=true.
// At this point, a simulation could be spawned. The agent would set up a
// simulation command specifying to imagine the motion of the pointer.
//
// step motion of the mouse
// respond with motion of the pointer
