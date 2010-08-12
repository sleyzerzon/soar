0 (A) link to the environment (top-state only) 

0 (A) svs link (all states) 

0 (A) command interface to Perceptual LTM (top-state only) (contents are accessed via semantic memory)  

0 (A) WM interface to the Spatial Scene (other STMs in SVS should be similar) 

0 (A) All commands go under here, similar to smem/epmem. Multiple simultaneous commands are possible, however.   

0 (A) the contents of the scene. All WMEs below this are architectural.

0 (MV) links to child objects

0 
0 true if an "expand" command will add children

0 
0 True if the agent is issuing an inhibit command for this object at this state or above. The main consequence of this is that it won't render in the GUI. May be missing if false.

0 an optional string, currently only added to imagery objects. Used to associate commands and resulting structures.

0 
0 (O) relationship-specific parameters go here

0 
0 (A) result(s) for the query are placed here. All WMEs below here are architectural.

0 
0 (X) the transform is derived from an existing relationship between objects. For example, "place the fork in relation to the plate like the tree is positioned in relation to the house."

0 (X) the transform is identity (none) 

0 (X) dual projection of "on" and "at"

0 
0 if present, the object will be in a facing relationship to this 

0 
0 inherit scaling from this object's transform, along with rotation (unless a facing-relationship is present) 

0 the new object will be on the uppermost surface of this object 

0 "on" and other FOR-specific predicates are determined in the global FOR of this object 

0 as with any parameters to a transform-source, these may be LTM classes (indicated by -class) and may have interpretations

0 
0 
0 
0 
0 (X w/parent) insert the new object above this object in the scene tree. Default: new object is below the scene root  

0 (MV) recursively generate a child object (same as command.generate, but no above/below id or index) 

0 (O) the same name will appear on the resulting image object

0 (O) a structure describing the source of the object shape for this node. Presence implies this is a primitive object, absence implies it is a grouping node. 

0 (X w/child) insert the new object below this object in the scene tree (the object referred to cannot be a primitive object, it must be a grouping node). Default: new object is below the scene root  

0 (O) copy the intrinsic frame of reference from this object (relative to its parent FOR). By default, the object-source determines this as appropriate. 

0 (A) the result of this command

0 (O) copy the texture from here (useful for debugging)

0 (O) This structure describes the source of the transform of the new object. If missing, the transform is whatever the default for the object-source is (e.g., the location of a direct projection) 

0 (MV) recursively generate a child

0 (O) a structure describing the source of the object shape for this node. Presence implies this is a primitive object, absence implies it is a grouping node. 

0 (O) copy the intrinsic frame of reference from this object (relative to its parent FOR). By default, the object-source determines this as appropriate.

0 (A) each child has its own result

0 (O) copy the texture from here (useful for debugging)

0 (O) This structure describes the source of the transform of the new object. If missing, the transform is whatever the default for the object-source is (e.g., the location of a direct projection) 

0 (X) example direct predicate projection command

0 (X) retrieval command, used to copy from LTM or clone in STM

0 (X) texture of LTM object

0 (X) same texture as STM object

0 (A) the new object in the scene (missing if error) 

0 
0 (MV) Each object is linked here, so the scene can be accessed in a flat manner. 

0 a root-object is always present, so the scene can be accessed as a tree 

0 (MV) expand an expandable node in the scene, making explict objects for its parts 

0 (MV) predicate extraction commands.

0 (MV) generate a new object, or set of of objects (tree) in the scene

0 (MV) inhibit an object in the scene (don't draw it in the GUI)

0 (MV) simulation process

0 (MV) attend to the motion of a particular object, in order to specialize an LTM motion type

0 (A,MV) some environments will present a set of valid actions via a set of these IDs

0 (A)

0 (MV) issue an action to the world

0 an internal command that tells SVS to inhibit all environmental updates

0 (A)

0 (MV) specialize a general motion category to a more specific LTM motion that can be learned via track-motion commands in the scene

0 (MV) backdoor means of adding objects to LTM

0 transform.rotation

0 transform.x

0 transform.y

0 transform.z

0 (O) used to seed the class-id

0 (MV) coordinates of a point in the object

0 (A)

0 (O) structure describing the transformation (from the parent). Default: identity

0 point.x

0 point.y

0 point.z

0 rotation.axis

0 rotation.degrees

0 the class-id of a specializable LTM motion

0 (O) used to seed the class-id

0 (A)

0 (A) id of the new object

0 (A)

0 (A) class ID for the new specialization

0 (A)

0 inhibit.id

0 true if node should be expanded entirely to leaves, otherwise one level only

0 node to expand

0 reference to the motion model in pLTM to use

0 the object being tracked

0 motion-specific parameters

0 (A)

0 (O) true if the tracking should be propogated to the model in LTM, so the learned parameters will persist. Default false.

0 the STM motion. If the class is a general category, simulations must use this ID. If the class is an LTM specialization, accessing it via the STM motion (this) and LTM class is equivalent. 

0 
0 
0 for direction queries

0 
0 
0 
0 
0 (O) for direction queries, use the FOR of this object (default is reference object)

0 
0 
0 
0 id of the action (copied from one of the available-action-ids, or something environment-specific)

0 placeholder for actions that take parameters

0 
0 (X) an STM object to copy,

0 (X) an LTM object to retrieve (LTID)

0 (MV) identifier (possibly LT) of an object to hull

0 (X w/class) used for simulations based on active tracking processes (ie, STM motions) 

0 (X w/motion) used for simulations of motion in LTM 

0 
0 
0 
0 
0 
0 
