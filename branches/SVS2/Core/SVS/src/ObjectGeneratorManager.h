
// This singleton class handles connecting generate command watchers to the
// functions that actually do the geometry processing

// This helps insulate qualitative/SVSObject representations in the system from
// geometric representations.

// Commands will be passed to this with parameters
// some parameters will refer to objects, the code here is responsible for
// grounding these before passing to the generator functions.
//
// This is done as follows:
// for all identifier parameters ending in (something)-object:
//  lookup the SVSObject* in the spatial scene, fail if not found
//  look for another parameter (something)-object-interpretation
//    if found, get the GroundedObject of that interpretation, and remove the
//      interpretation parameter
//    otherwise, get the poly3d GroundedObject
//  convert the parameter to be a GroundedObject parameter
//
// for all identifier parameters ending in (something)-object-class:
//  lookup the SVSObject* in pLTM, fail if not found
//  look for another parameter (something)-object-interpretation
//    if found, get the GroundedObject of that interpretation, and remove the
//      interpretation parameter
//    otherwise, get the poly3d GroundedObject
//  rename the parameter name to (something)-object (remove "class")
//  convert the parameter to be a GroundedObject parameter
//
//  Retrieve commands are handled specially, since they need to clone tree
//  structure rather than make a new object based solely on geometric info. For
//  those, the SVSObject* is looked up in the spatial scene or pLTM and
//  SVSObject::clone is called on it to get a new set of nodes. The
//  manipulateObject function is called in the root of the result as if
//  the object came from generateObject.

class ObjectGeneratorManager {
public:
  // constructor goes here (initialize the map of types to generator functions)

  // Return a new SVSObject of the generated object, which the caller will now
  // own (note that it isn't in the spatial scene yet, the generate watcher
  // does that).
  //
  // This is intended for direct predicate projection.
  SVSObject* generateObject(string type, 
      vector< Parameter > parameters);
  
  // Return an object from STM or LTM (clone the scene tree structure)
  SVSObject* retrieveObject(string id, bool isLTM);
  
  // Manipulate the primaryObject such that it follows the
  // qualitative specification.
  //
  // This is intended to be used for indirect predicate projection. The
  // manipulation function will return a new GroundedObject*. manipulateObject() 
  // must then change the transformation on the primaryObject (which is, at
  // this point, rooted at the scene origin) by calling makeTransformation() on
  // the new GroundedObject, applying that tx to the primaryObject, and deleting
  // the GroundedObject.
  //
  // This means that any shape changes to the GroundedObject will be discarded,
  // and also that everything will work appropriately if the primaryObject is a
  // grouping node (a complete object with substructure will be manipulated, even though the
  // manipulation function deals only with the convex hull of all its parts).
  bool manipulateObject(string type,
      SVSObject* primaryObject,
      string primaryObjectInterpretation,
      vector< Parameter > parameters);
private:
  // table of types to generators and manipulators goes here

};

// example object generator header
// these all must have the same prototype, there will be a switch statement
// inside ObjectGeneratorManager that will associate type strings to function
// ptrs

GroundedObject* generateConvexHull(vector< Parameter > parameters); 
                                  // should contain one or more GroundedObject parameters 

// example manipulation header
GroundedObject* applyAnalogicalTransform(GroundedObject* primaryObject,
                                         vector< Parameter > parameters); 
                                        // should contain GroundedObject
                                        // parameters:
                                        // source-primary-object
                                        // source-reference-object
                                        // source-frame-object
                                        // reference-object
                                        // reference-frame-object

