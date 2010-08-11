// This watcher should do the following:
//  -parse the generic generate command structure
//  -call the ObjectGeneratorManager function to build the new object based on
//  the object-source command structure
//  -call the ObjectGeneratorManager function to manipulate the new object
//  based on the transform-source command structure
//  - ...
//

class GenerateWatcher : public CommandWatcher {
public: 

  // constructor stores the commandWME, and calls CommandWatcher constructor
  // specifying Early phase 
  GenerateWatcher(Identifier* _commandWME, int _time);


  // All of the action occurs during the first call to updateResult().
  // Subsequent calls check if anything in the command changed, and remove the
  // image and add status error if that is the case: dynamic imagery is
  // unsupported (outside of the case where an image moves due to
  // manipulations above it in the scene tree).
  //
  // During the first call 
  void updateResult();


  ~GenerateWatcher();
};
