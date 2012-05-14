(define (problem p8)
  (:domain blocks_world)
  (:objects b1 b2 b3 b4 b5 b6 b7 b8)
  (:init
    (on b2 b5)
    (on b3 b6)
    (on b4 b7)
    (on b5 b4)
    (on b6 b8)
    (on b7 b1)
    (on b8 b2)
    (on-table b1)
    (clear b3))
  (:goal (and
    (on b1 b3)
    (on b3 b7)
    (on b4 b1)
    (on b6 b2)
    (on b8 b5))))
