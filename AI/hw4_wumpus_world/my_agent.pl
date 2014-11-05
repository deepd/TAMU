%my_agent.pl

%   this procedure requires the external definition of two procedures:
%
%     init_agent: called after new world is initialized.  should perform
%                 any needed agent initialization.
%
%     run_agent(percept,action): given the current percept, this procedure
%                 should return an appropriate action, which is then
%                 executed.
%
% This is what should be fleshed out

:- dynamic ([
		agent_loc/1, 
		wumpus_loc/1, 
		safe/1, 
		pit_loc/1, 
		agent_orient/1, 
		gold_loc/1, 
		time/1,
		visited_cells/1,
		arrow/1,
		wumpus_heart/1,
		worldsize/1,
		isGoldMine/1
	]).

init_agent:-
	format('\n=====================================================\n'),
	format('This is init_agent:\n\tIt gets called once, use it for your initialization\n\n'),
	format('=====================================================\n\n'),
	retractall(agent_loc([_,_])),
	retractall(agent_orient(_)),
	retractall(time(_)),
	retractall(arrow(_)),
	retractall(wumpus_heart(_)),
	retractall(safe([_,_])),
	retractall(action(_,_)),
	retractall(worldsize(_)),
	retractall(isGoldMine(_)),
	retractall(visited_cells(_)),
	retractall(isWumpus(_,_)),
	retractall(isPit(_,_)),
	retractall(foundWS(_)),
	assert(time(0)),
	assert(agent_loc([1,1])),
	assert(visited_cells([1,1])),
	assert(agent_orient(0)),
	assert(arrow(1)),
	assert(wumpus_heart(1)),
	assert(safe([1,1])),
	assert(worldsize(40)),
	assert(isGoldMine(0)),
	assert(foundWS(0)).



%   Action is one of:
%     goforward: move one square along current orientation if possible
%     turnleft:  turn left 90 degrees
%     turnright: turn right 90 degrees
%     grab:      pickup gold if in square
%     shoot:     shoot an arrow along orientation, killing wumpus if
%                in that direction
%     climb:     if in square 1,1, leaves the cave and adds 1000 points
%                for each piece of gold
%
%   Percept = [Stench,Breeze,Glitter,Bump,Scream]



	
turnLeftAction:-
	format("in turnLeftAction~n"),
	(	agent_orient(0)		->	retractall(agent_orient(_)), assert(agent_orient(90));
  		agent_orient(90)	->	retractall(agent_orient(_)), assert(agent_orient(180));
  		agent_orient(180)	->	retractall(agent_orient(_)), assert(agent_orient(270));
  		agent_orient(270)	->	retractall(agent_orient(_)), assert(agent_orient(0))
  	).

turnRightAction:-
	format("in turnRightAction~n"),
	(	agent_orient(0)		->	retractall(agent_orient(_)), assert(agent_orient(270));
  		agent_orient(90)	->	retractall(agent_orient(_)), assert(agent_orient(0));
  		agent_orient(180)	->	retractall(agent_orient(_)), assert(agent_orient(90));
  		agent_orient(270)	->	retractall(agent_orient(_)), assert(agent_orient(180))
  	).

goForwardAction:-
	format("in goForwardAction~n"),
	agent_loc([X,Y]),
	worldsize(WS),
  	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	(	agent_orient(0)		->	(X =\= WS 	->	retractall(agent_loc([_,_])), assert(agent_loc([X1,Y])), makeitSafe([X1,Y]), assert(visited_cells([X1,Y])); 
  												format("doing nothing becuase it's gonaa be a bump~n"));
	  	agent_orient(90)	->	(Y =\= WS 	->	retractall(agent_loc([_,_])), assert(agent_loc([X,Y1])), makeitSafe([X,Y1]), assert(visited_cells([X,Y1])); 
	  											format("doing nothing becuase it's gonaa be a bump~n"));
	  	agent_orient(180)	->	(X =\= 1 	->	retractall(agent_loc([_,_])), assert(agent_loc([X0,Y])), makeitSafe([X0,Y]), assert(visited_cells([X0,Y])); 
	  											format("doing nothing becuase it's gonaa be a bump~n"));
	  	agent_orient(270)	->	(Y =\= 1 	->	retractall(agent_loc([_,_])), assert(agent_loc([X,Y0])), makeitSafe([X,Y0]), assert(visited_cells([X,Y0])); 
	  											format("doing nothing becuase it's gonaa be a bump~n"))
  	).

grabAction:-
	format("in grabAction~n"),
	agent_loc([X,Y]),
	assert(gold_loc([X,Y])),
	retractall(isGoldMine(_)),
	assert(isGoldMine(1)).

shootAction:-
	format("in shootAction~n"),
	retractall(arrow(_)),
	assert(arrow(0)),
	format('Shoot action').

climbAction:-
	format('Climb action').

updateKB([Stench,Breeze,Glitter,_,Scream]):-
	%listing(agent_loc),
	add_wumpus_KB(Stench),
	add_pit_KB(Breeze),
	add_gold_KB(Glitter),
	add_wumpus_health_KB(Scream).
	%listing(agent_loc).

add_wumpus_health_KB(yes):-
	retractall(wumpus_heart(_)),
	format("You shot the wumpus !!!~n"),
	assert(wumpus_heart(0)).

add_wumpus_health_KB(no):-
	time(T),
	OldTime is T - 1,
	(action(OldTime, shoot)	->	format("You shot and didn't kill the wumpus~n")
		; format('')
	).

add_wumpus_KB(no):-
	agent_loc([X,Y]),
	assume_wumpus(no,[X,Y]),
    Z1 is Y+1, assume_wumpus(no,[X,Z1]),
    Z2 is Y-1, assume_wumpus(no,[X,Z2]),
    Z3 is X+1, assume_wumpus(no,[Z3,Y]),
    Z4 is X-1, assume_wumpus(no,[Z4,Y]).

add_wumpus_KB(yes):-
	agent_loc([X,Y]),
	assume_wumpus(no,[X,Y]),
    Z1 is Y+1, assume_wumpus(yes,[X,Z1]),
    Z2 is Y-1, assume_wumpus(yes,[X,Z2]),
    Z3 is X+1, assume_wumpus(yes,[Z3,Y]),
    Z4 is X-1, assume_wumpus(yes,[Z4,Y]).

add_pit_KB(no):-
	agent_loc([X,Y]),
	assume_pit(no,[X,Y]),
    Z1 is Y+1, assume_pit(no,[X,Z1]),
    Z2 is Y-1, assume_pit(no,[X,Z2]),
    Z3 is X+1, assume_pit(no,[Z3,Y]),
    Z4 is X-1, assume_pit(no,[Z4,Y]).

add_pit_KB(yes):-
	agent_loc([X,Y]),
	assume_pit(no,[X,Y]),
    Z1 is Y+1, assume_pit(yes,[X,Z1]),
    Z2 is Y-1, assume_pit(yes,[X,Z2]),
    Z3 is X+1, assume_pit(yes,[Z3,Y]),
    Z4 is X-1, assume_pit(yes,[Z4,Y]).

add_gold_KB(no):-
	agent_loc([X,Y]),
	assume_gold(no, [X,Y]).

add_gold_KB(yes):-
	agent_loc([X,Y]),
	assume_gold(yes,[X,Y]).

assume_wumpus(no, L):-
	retractall( isWumpus(_, L) ),
	retractall( isWumpus(_,[0,_]) ),
	retractall(	isWumpus(_,[_,0]) ),
    assert( isWumpus(no, L) ),
    format("KB learn ~p - no Wumpus there!~n", [L]).

assume_wumpus(yes, L) :-
	retractall( isWumpus(_,[0,_]) ),
	retractall(	isWumpus(_,[_,0]) ),
	(	isWumpus(no,L)		->	donothing;
		retractall( isWumpus(_, L) ),
    	assert( isWumpus(yes, L) )
    ),
    format("KB learn ~p - possibly the Wumpus is there!~n", [L]).

assume_pit(no, L) :-
    retractall( isPit(_, L) ),
    retractall( isPit(_,[0,_]) ),
	retractall(	isPit(_,[_,0]) ),
    assert( isPit(no, L) ),
    format("KB learn ~p - there's no Pit there!~n", [L]).

assume_pit(yes, L) :-
    retractall( isPit(_,[0,_]) ),
	retractall(	isPit(_,[_,0]) ),
	(	isPit(no,L)		->	donothing;
		retractall( isPit(_, L) ),
    	assert( isPit(yes, L) )
    ),
    format("KB learn ~p - its possibly a Pit!~n", [L]).

assume_gold(no, L) :-
    retractall( isGold(_, L) ),
    assert( isGold(no, L) ),
    format("KB learn ~p - there's no gold here!~n", [L]).

assume_gold(yes, L) :-
    retractall( isGold(_, L) ),
    assert( isGold(yes, L) ),
    format("KB learn ~p - GOT THE GOLD!!!~n", [L]).

update_time:-
	time(T),
	Newtime is T+1,
	retractall(time(_)),
	assert(time(Newtime)).

%run_agent(Percept,Action):-	

run_agent(Percept, Action):- 
	format("Percept recieved : ~p~n",[Percept]),
	listing(agent_orient),
	listing(agent_loc),
	time(T),
	format("time done~n"),
	updateKB(Percept),
	format("updateKB done~n"),
	update_safe_cells_KB,
	format("update_safe_cells_KB done~n"),
	listing(safe),
	(	isGoldMine(1), format("Going in to gotGoldFindMePathHome~n")	->	gotGoldFindMePathHome(Percept,Action);	
		isGoldMine(0), format("Going in to findAction~n")	->	findAction(Percept,Action);
		donothing
	),
	format("findAction done~n"),
	assert(action(T,Action)),
	format("assert done~n"),
	update_time,
	format("update_time done~n"),
	display_world,
	(	Action == turnleft 	->	turnLeftAction	;
		Action == turnright ->	turnRightAction	;
		Action == goforward	->	goForwardAction	;
		Action == grab 		->	grabAction		;
		Action == climb 	->	climbAction		;
		Action == shoot 	->	shootAction		;
		donothing	
	).
	
gotGoldFindMePathHome([_,_,yes,_,_],Action):-
	format("in gotGoldFindMePathHome with grab~n"),
	Perc = [x,x,yes,x,x],
	findAction(Perc,Action).
  		
gotGoldFindMePathHome([_,_,_,yes,_],Action):-
	format("in gotGoldFindMePathHome with bump~n"),
	Perc = [x,x,x,yes,x],
	findAction(Perc,Action).
	
gotGoldFindMePathHome(_, Action):-
	agent_loc([X,Y]),
	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	worldsize(WS),
  	(	X =:= 1 , Y =:= 1 	->	(	not(visited_cells([1,2])), safe([1,2])	->	(	agent_orient(0)		->	Action = turnleft;
  																					agent_orient(90)	->	Action = goforward;
  																					agent_orient(180)	->	Action = turnright;
  																					agent_orient(270)	->	Action = turnleft	
  																				);
  									not(visited_cells([2,1])), safe([2,1])	->	(	agent_orient(0)		->	Action = goforward;
  																					agent_orient(90)	->	Action = turnright;
  																					agent_orient(180)	->	Action = turnleft;
  																					agent_orient(270)	->	Action = turnleft	
  																				);
  									Action = climb	
  								);
  		agent_orient(0)		->	(	not(visited_cells([X1,Y])), safe([X1,Y]), X =\= WS	->	Action = goforward;
  									safe([X1,Y]), X1 =\= WS	->	Action = goforward;
  									not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnleft;
  									safe([X,Y1])	->	Action = turnleft;
  									not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnright;
  									safe([X,Y0])	->	Action = turnright;
  									Action = turnleft
  								);
  		agent_orient(90)	->	(	not(visited_cells([X,Y1])), safe([X,Y1]), Y =\= WS	->	Action = goforward;
  									safe([X,Y1]), Y1 =\= WS 	->	Action = goforward;
  									not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = turnleft;
									safe([X0,Y])	->	Action = turnleft;
									not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnright;
  									safe([X1,Y])	->	Action = turnright;
  									Action = turnleft
  								);
  		agent_orient(180)	->	(	not(visited_cells([X0,Y])), safe([X0,Y]), X0 =\= 0	->	Action = goforward;
  									safe([X0,Y]), X0 =\= 0	->	Action = goforward;
  									not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnleft;
  									safe([X,Y0])	->	Action = turnleft;
  									not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnright;
  									safe([X,Y1])	->	Action = turnright;
  									Action = turnleft
  								);
  		agent_orient(270)	->	(	not(visited_cells([X,Y0])), safe([X,Y0]), Y0 =\= 0	->	Action = goforward;
  									safe([X,Y0]), Y0 =\= 0	->	Action = goforward;
  									not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = turnright;
  									safe([X0,Y])	->	Action = turnright;
  									not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnleft;
  									safe([X1,Y])	->	Action = turnleft;
  									Action = turnleft
  								);
  		donothing
  	).


makeitSafe(L):-
	retractall(safe(L)),
	assert(safe(L)).

donothing:-
	format(".").

update_safe_cells_KB:-
	format("Entered update_safe_cells_KB~n"),
	worldsize(WS),
	EX is WS + 1,
	agent_loc([X,Y]),
	%format("first done~n"),
	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	(	format("1st clause~n"), isPit(no,[X,Y1]), isWumpus(no,[X,Y1])	->	makeitSafe([X,Y1]); donothing)	,
  	(	format("2nd clause~n"), isPit(no,[X,Y0]), isWumpus(no,[X,Y0])	->	makeitSafe([X,Y0]); donothing)	,
  	(	format("3rd clause~n"), isPit(no,[X1,Y]), isWumpus(no,[X1,Y])	->	makeitSafe([X1,Y]); donothing)	,
  	(	format("4th clause~n"), isPit(no,[X0,Y]), isWumpus(no,[X0,Y])	->	makeitSafe([X0,Y]); donothing)	,
  	retractall(safe([0,_])),
	retractall(safe([_,0])),
	retractall(safe([_,EX])),
	retractall(safe([EX,_])),
  	format("nothing to do in update_safe_cells_KB~n"),
  	format("Leaving update_safe_cells_KB~n").


countWumpusLocation(N,E):-
	format("In countWumpusLocation~n"),
	findall(C,isWumpus(yes,C),Bag),
	length(Bag,N),
	[E|_] = Bag.

findAction([_,_,yes,_,_],Action):-
	format("Yo Yo Yo, Got the GOLD!!!~n"),
	Action = grab.


findAction([_,_,_,yes,_],Action):-
	format("In bump findAction~n"),
	agent_loc([X,Y]),
	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	(	agent_orient(0), format("in bump 2nd if clause~n") 			->	(	safe([X,Y1])	->	Action = turnleft, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(X0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X0,Y])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	);
																		safe([X,Y0])	->	Action = turnright, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(X0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X0,Y])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	);
																		Action = turnleft, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(X0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X0,Y])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	)
																	);
  																	donothing
  	),
	(	agent_orient(90), format("in bump 3rd if clause~n") 			->	(	safe([X1,Y])	->	Action = turnright, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(Y0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X,Y0])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	);
																		safe([X0,Y])	->	Action = turnleft, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(Y0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X,Y0])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	);
																		Action = turnleft, (	foundWS(0)	->	retractall(worldsize(_)), 
  																								assert(worldsize(Y0)), retractall(agent_loc([X,Y])),
  																								assert(agent_loc([X,Y0])), retractall(foundWS(0)), assert(foundWS(1));
  																								donothing	)
																		);
  																	donothing
	),
	(	agent_orient(180), format("in bump 4th if clause~n") 		->	(	safe([X,Y1])	->	Action = turnright;
																		safe([X,Y0])	->	Action = turnleft;
																		Action = turnleft
																	);
  																	donothing
	),
	(	agent_orient(270), format("in bump 5th if clause~n") 		->	(	safe([X1,Y])	->	Action = turnleft;
																		safe([X0,Y])	->	Action = turnright;
																		Action = turnleft
																	);
  																	donothing		
	).

findAction([no,no,no,no,no],Action):-
	format("In no no findAction~n"),
	agent_loc([X,Y]),
	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	Perc = [no,no,no,no,no],
	(	agent_orient(0)		->	(	not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = goforward;
									not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnleft;
									not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnright;
									retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
								);
		agent_orient(90)	->	(	not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = goforward;
									not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnright;
									not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = turnleft;
									retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
								);
		agent_orient(180)	->	(	not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = goforward;
									not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnright;
									not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnleft;
									retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
								);
		agent_orient(270)	->	(	not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = goforward;
									not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnleft;
									not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnright;
									retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
								)
	),
	makeitSafe([X,Y]),
	makeitSafe([X,Y1]),
	makeitSafe([X,Y0]),
	makeitSafe([X0,Y]),
	makeitSafe([X1,Y]).


findAction([yes,_,_,_,_],Action):-
	format("In stink~n"),
	agent_loc([X,Y]),
	X1 is X + 1,
  	Y1 is Y + 1,
  	X0 is X - 1,
  	Y0 is Y - 1,
  	Perc = [yes,x,x,x,x],
  	(	agent_orient(0), format("in stink 2nd if clause~n") 			->	(	not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnleft;
																				not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnright;
																				not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = goforward;
																				retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)

																	);
  																	donothing
  	),
	(	agent_orient(90), format("in stink 3rd if clause~n") 			->	(	not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnright;
																				not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = turnleft;
																				not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = goforward;
																		retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
																	);
  																	donothing
	),
	(	agent_orient(180), format("in stink 4th if clause~n") 		->	(		not(visited_cells([X,Y1])), safe([X,Y1])	->	Action = turnright;
																				not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = turnleft;
																				not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = goforward;
																		retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
																	);
  																	donothing
	),
	(	agent_orient(270), format("in stink 5th if clause~n") 		->	(		not(visited_cells([X1,Y])), safe([X1,Y])	->	Action = turnleft;
																				not(visited_cells([X0,Y])), safe([X0,Y])	->	Action = turnright;
																				not(visited_cells([X,Y0])), safe([X,Y0])	->	Action = goforward;
																		retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
																	);
  																	donothing
	),
	countWumpusLocation(N,E),
	(	N =:= 1 	->	[WX,WY] = E, (	agent_orient(0)	->	(	WY =:= Y, WX > X 	-> 	Action = shoot;
																			donothing
												)
						);
						[WX,WY] = E, (	agent_orient(90)	->	(	WX =:= X, WY > Y 	-> 	Action = shoot;
																			donothing
												)
						);
						[WX,WY] = E, (	agent_orient(180)	->	(	WY =:= Y, WX < X 	-> 	Action = shoot;
																			donothing
												)
						);
						[WX,WY] = E, (	agent_orient(270)	->	(	WX =:= X, WY < Y	-> 	Action = shoot;
																			donothing
												)
						);
						donothing
	),
	(	X =:= 1, Y =:= 1 	->	format("in last clause~n"), gotGoldFindMePathHome(Perc,Action), format("after last clause~n");
								donothing
	).
	
findAction([_,_,_,_,yes],_):-
	format("Yo you killed the Wumpus~n"),
	donothing.

findAction([_,yes,_,_,_],Action):-
	time(T),
	agent_loc([X,Y]),
	X1 is X + 1,
	X0 is X - 1,
	Y1 is Y + 1,
	Y0 is Y - 1,
  	T1 is T - 1,
  	T2 is T - 2,
  	T3 is T - 3,
  	T4 is T - 4,
  	Perc = [x,yes,x,x,x],
  	format("In breeze findAction~n"),
	(	format("0th breeze clause~n"), action(T1, turnleft), action(T2, turnleft), action(T3, turnleft), action(T4, turnleft)	->	retractall(isGoldMine(0)),
																																	assert(isGoldMine(1)),
																																	gotGoldFindMePathHome(Perc,Action);
		format("1st breeze clause~n"), action(T1, turnleft), action(T2, turnleft)	->	(	agent_orient(0), not(isPit(yes,[X1,Y]))	->	 Action = goforward;
																							agent_orient(90), not(isPit(yes,[X,Y1]))	->	 Action = goforward;
																							agent_orient(180), not(isPit(yes,[X0,Y]))	->	 Action = goforward;
																							agent_orient(270), not(isPit(yes,[X,Y0]))	->	 Action = goforward;
																							retractall(isGoldMine(0)), assert(isGoldMine(1)), gotGoldFindMePathHome(Perc,Action)
																						);
		format("2nd breeze clause~n"), action(T1, turnleft)	->	Action = turnleft;
		Action = turnleft
	).

bumpAction:-
	agent_loc([X,Y]),
	X1 is X + 1,
	X0 is X - 1,
	Y1 is Y + 1,
	Y0 is Y - 1,
	(	agent_orient(0)		->	retractall(worldsize(_)), assert(worldsize(X0)),
								retractall(agent_loc(_)), assert(agent_loc([X0,Y]));
		agent_orient(90)	->	retractall(worldsize(_)), assert(worldsize(Y0)),
								retractall(agent_loc(_)), assert(agent_loc([X,Y0]));
		agent_orient(180)	->	retractall(worldsize(_)), assert(worldsize(X1)),
								retractall(agent_loc(_)), assert(agent_loc([X1,Y]));
		agent_orient(270)	->	retractall(worldsize(_)), assert(worldsize(Y1)),
								retractall(agent_loc(_)), assert(agent_loc([X,Y1]))
	).

