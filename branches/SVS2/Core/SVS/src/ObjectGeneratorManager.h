
// This singleton class handles connecting generate command watchers to the
// functions that actually do the geometry processing

// This helps insulate qualitative/SVSObject representations in the system from
// geometric representations.

class ObjectGeneratorManager {
public:
  // constructor goes here (initialize the map of types to generator functions)

  // This function does the following:
  // convert the SVSObject parameters to GroundedObjects
  //
  // lookup the function based on the type of the command
  // 
  // call the generator function
  //
  // return a new SVSObject of the generated object, which the caller will now
  // own (note that it isn't in the spatial scene yet)
  SVSObject generate(string type, 
      vector< SVSObject* > objects, 
      vector< ObjectInterpretation > interpretations, 
      vector< Parameter > parameters);


}

// example spatial generator header

GroundedObject* generateConvexHull(vector< GroundedObject* > objects, 
                                   vector< Parameter > parameters);


