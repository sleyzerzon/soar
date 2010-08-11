
// This singleton class handles connecting extract command watchers to the
// functions that actually do the geometry processing

// This helps insulate qualitative/SVSObject representations in the system from
// geometric representations.

// Commands will be passed to this with parameters
// some parameters will refer to objects, the code here is responsible for
// grounding these before passing to the extractor functions.
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
// ERROR: extractions cannot access LTM


class ExtractionManager {
public:
  Result extract(string type, vector< Parameter > parameters);
private:
  // table of types to extracters goes here
};

// example extraction header

Result rccDR(vector< Parameter > parameters);
             // should contain two GroundedObject parameters

