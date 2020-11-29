-module(solution).
-mode(compile).
-export([main/1]).


main(_) ->
	Lines = read_lines(),
	Severity = severity(Lines),
	io:format("~p~n", [Severity]),
	Delay = delay(Lines),
	io:format("~p~n", [Delay]),
	true = Delay == 3964778,
	ok.

caught([D, R]) ->
	Cycle = (R - 1) * 2,
	case D rem Cycle of
		0 -> D * R;
		_ -> 0
	end.

caught([D, R], T) ->
	Cycle = (R - 1) * 2,
	((D + T) rem Cycle) == 0.

severity(Xs) ->
	severity(Xs, 0).

severity([], N) -> N;
severity([X|Xs], N) ->
	severity(Xs, N + caught(X)).

good_trip([], _) -> true;
good_trip([X|Xs], T) ->
	not caught(X, T) and good_trip(Xs, T).

delay(Xs) -> delay(Xs, 0).

delay(Xs, N) ->
	case good_trip(Xs, N) of
		true -> N;
		false -> delay(Xs, N+1)
	end.

read_lines() ->
	lists:reverse(read_lines([])).

read_lines(X) ->
	case io:fread("", "~d: ~d") of
		{ok, Y} -> read_lines([Y|X]);
		_ -> X
	end.
