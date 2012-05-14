
(define (problem hanoi4)
  (:domain hanoi)
  (:objects peg1 peg2 peg3 d1 d2 d3 d4)
  (:init 
   (smaller peg1 d1) (smaller peg1 d2) (smaller peg1 d3) (smaller peg1 d4)
   (smaller peg2 d1) (smaller peg2 d2) (smaller peg2 d3) (smaller peg2 d4)
   (smaller peg3 d1) (smaller peg3 d2) (smaller peg3 d3) (smaller peg3 d4)
   (smaller d2 d1) (smaller d3 d1) (smaller d3 d2) (smaller d4 d1)
   (smaller d4 d2) (smaller d4 d3)
   (clear peg2) (clear peg3) (clear d1)
   (on d4 peg1) (on d3 d4) (on d2 d3) (on d1 d2))
  (:goal (and (on d4 peg3) (on d3 d4) (on d2 d3) (on d1 d2)))
  )
