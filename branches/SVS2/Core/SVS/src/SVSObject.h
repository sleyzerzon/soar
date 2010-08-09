
// This class is the basic object type in the system. Wm3 should be
// encapsulated within here and pLTM, only accessed by the GUI and GroundedObject.

class SVSObject {
public:
  SVSObject(GroundedObject* source, SVSObject* _parent, string _id, string _classId, bool _imagined);
  SVSObject(Spatial* source, SVSObject* _parent, string _id, string _classId, bool _imagined);

    
  ~SVSObject();

  bool isImagined() {
    return imagined;
  }

  bool isPrimitive() {
    return primitive;
  }

  // getter for id/classid goes here

  SVSObject* getParent() {
    return parent;
  }

  // These add and remove child nodes, including handling linking within the
  // Wm3 scene graph.
  bool addChild(SVSObject* child); // return false if this is primitive 
  bool removeChild(SVSObject* child); // return false if child not present

  // getChild(int) or getChildren() may be necessary

  // this will generate the appropriate grounded object if it
  // isn't current, or return it if it is
  //
  // this object owns the dynamically allocated polyhedron
  GroundedObject* getGroundedObject(string interpretation);

  // Move the object so the global transformation to its centroid is this.
  // Local transformation will be calculated accordingly.
  // This is the main way of moving objects.
  void setGlobalTransformation(Transformation tx);

  // Set the texture of this object to be that of the source.
  // Return false if either this or the other isn't primitive.
  bool setTexture(SVSObject* source);
  
  // This clones the tree from this object to below, creating new SVSObjects
  // that the caller has ownership of. Children are implicit in the tree, but
  // also listed flatly to prevent reparsing.
  //
  // pLTM retrievals use this:
  // SVSObject* myNewSceneObject = SVSObject::clone(perceptualLTM->retrieve("something"));
  static SVSObject* clone(SVSObject* source, vector<SVSObject*>& out_children);

private:
  SVSObject* parent;
  vector< SVSObject* > children;
  Spatial* wm3Unit;
  string id;
  string classId;
  bool imagined;
  bool primitive;

  GroundedObjectPolyhedron3d* poly3;
  GroundedObjectPolygon2d* poly2;
  GroundedObjectBoundingBox3* bbox3;
  GroundedObjectBoundingBox2* bbox2;
  GroundedObjectPoint3* point3;
  GroundedObjectPoint2* point2;
  
  // this must be called whenever the object structurally changes
  // (a decendant is added or removed)
  void invalidateGroundedObjects(); 

};
