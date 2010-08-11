
// This class is the basic object type in the system. Wm3 should be
// encapsulated within here and pLTM, only accessed by the GUI and GroundedObject.

class SVSObject {
  friend class SpatialScene; // see setGlobalTransformation()
public:
  SVSObject(GroundedObject* source, SVSObject* _parent, string _id, string _classId, bool _imagined);
  SVSObject(SpatialPtr source, SVSObject* _parent, string _id, string _classId, bool _imagined);
    
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

  // This returns an error if the object is already inhibited at a higher level
  // (but then, so should all commands referencing the object below that level)
  bool inhibit(int level);
  
  // Level is necessary here due to glitching. Object should only be actually
  // uninhibited if the level passed is the lower than the current inhibit
  // level (but this is NOT an error).
  // For example, if the agent inhibits at level 3,
  // and then again at level 2, at the next decision, support for the level-3 inhibit
  // command will go away (since the object is no longer at level 3), and
  // uninhibit will be called by that command's watcher. But the level 2
  // command is still valid, so it should remain inhibited.
  bool uninhibit(int level);
  int getInhibitionLevel();

  // getChild(int) or getChildren() may be necessary

  // this will generate the appropriate grounded object if it
  // isn't current, or return it if it is
  //
  // this object owns the dynamically allocated polyhedron
  GroundedObject* getGroundedObject(string interpretation);

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
  
  // Note that this is a Wm3 smart pointer. These are used internally in Wm3,
  // so we need them here so that system doesn't delete them. This (along with
  // pLTM) are the only places smart ptrs need to be, since that is where the
  // pointers are owned.
  SpatialPtr wm3Unit;
  string id;
  string classId;
  bool imagined;
  bool primitive;
  int inhibitedLevel;

  GroundedObjectPolyhedron3d* poly3;
  GroundedObjectPolygon2d* poly2;
  GroundedObjectBoundingBox3* bbox3;
  GroundedObjectBoundingBox2* bbox2;
  GroundedObjectPoint3* point3;
  GroundedObjectPoint2* point2;
  
  // this must be called whenever the object structurally changes
  // (a decendant is added or removed)
  void invalidateGroundedObjects(); 
  
  // Move the object so the global transformation to its centroid is this.
  // Local transformation will be calculated accordingly.
  // This is the main way of moving objects.
  // This should only be called by SpatialScene, who is a friend,
  // since the scene needs to determine the change set of everything below this
  // node and tell other modules about it
  void setGlobalTransformation(Transformation tx);


};
