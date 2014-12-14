;; STRIPS domain of Star Puzzle
(define (domain star-puzzle)
	(:requirements :strips)
	(:predicates (on ?obj1 ?obj2)
			(clear ?obj)
			(middle ?peg)
		)

(:action movetomiddle
	:parameters (?disk ?source ?dest)
	:precondition (and 	(middle ?dest)
						(clear ?disk)
						(on ?disk ?source)
						(clear ?dest)
						(smaller ?disk ?dest)
						
				)
		
	:effect (and (on ?disk ?dest)
				(not (on ?disk ?source))
				(not (clear ?dest))
				(clear ?source)
				(middle ?disk)
		)
	)

(:action movefrommiddle
	:parameters (?disk ?source ?dest)
	:precondition (and 	(middle ?source)
						(clear ?disk)
						(on ?disk ?source)
						(clear ?dest)
						(smaller ?disk ?dest)
						
				)
		
	:effect (and (on ?disk ?dest)
				(not (on ?disk ?source))
				(not (clear ?dest))
				(clear ?source)
				(not (middle ?disk))
		)
	)

)
