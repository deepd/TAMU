;; STRIPS 4 disks problem for the star puzzle 
(define (problem pb4)
	(:domain star-puzzle)
	(:objects D1 D2 D3 D4 D5
		A B C O
		)
	(:length (:parallel 32))

(:init
		(middle O)
		(on D5 A) (on D4 D5) (on D3 D4) (on D2 D3) (on D1 D2)
		(clear D1) (clear B) (clear C) (clear O)
		(smaller D1 D2) (smaller D1 D3) (smaller D1 D4) (smaller D1 D5) (smaller D1 A) (smaller D1 B) (smaller D1 C) (smaller D1 O)
		(smaller D2 D3) (smaller D2 D4) (smaller D2 D5) (smaller D2 A) (smaller D2 B) (smaller D2 C) (smaller D2 O)
		(smaller D3 D4) (smaller D3 D5) (smaller D3 A) (smaller D3 B) (smaller D3 C) (smaller D3 O)
		(smaller D4 D5) (smaller D4 A) (smaller D4 B) (smaller D4 C) (smaller D4 O)
		(smaller D5 A) (smaller D5 B) (smaller D5 C) (smaller D5 O)

	)
(:goal
		(and 	(on D5 C)
				(on D4 D5)
				(on D3 D4)
				(on D2 D3)
				(on D1 D2)
		)
	)
)