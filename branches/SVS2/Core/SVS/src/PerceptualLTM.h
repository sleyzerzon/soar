
class PerceptualLTM {
public:
  // read a set of model files and textures from disc during construction
  //
  // note: geometry of models must be normalized at some point prior to
  // retrieval, so the centroid is the origin of the local FOR
  PerceptualLTM(string directory);

  // retrieve one or more objects from LTM
  // note that this will return a pointer to the root of a tree (assuming the
  // object retrieved isn't primitive)
  // Caller does _not_ own this, but has to call SVSObject.clone() to get a
  // copy.
  SVSObject* retrieve(string classId);

 string declareObject(string name, vector< double > xyzList, string textureName = ""); 
};
