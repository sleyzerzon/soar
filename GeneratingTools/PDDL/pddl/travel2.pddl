(define (problem bulldozer2)
  (:domain travel)
  (:objects a b c d e f g h i j k l m n
	    jack bulldozer)
  (:init (at jack a) (at bulldozer e)
	 (vehicle bulldozer)
	 (mobile jack)
	 (person jack)
	 (road a b) (road b a)
	 (road a c) (road c a)
	 (road c d) (road d c)
	 (road d e) (road e d)
	 (road e j) (road j e)
	 (road d f) (road f d)
	 (road f j) (road j f)
	 (road f k) (road k f)
	 (road j h) (road h j)
	 (road h k) (road k h)
	 (bridge k l) (bridge l k)
	 (bridge k n) (bridge n k)
	 (road l m) (road m l)
	 (road m n) (road n m)
	 (road m g) (road g m)
	 (road n g) (road g n))
  (:goal (and (at bulldozer g)))
  )
