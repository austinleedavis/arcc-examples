-module (pi).
-export ([pi/0]).

pi() -> 4 * pi(0,1,1).

pi(T,M,D) ->
	A = 1 / D,
	if A > 0.00001 -> pi(T+(M*A), M*-1, D+2);
		true -> T 
	end.
