-module(solution).
-mode(compile).
-export([main/1]).

main(_) ->
	Lines = read_lines(),
	Sorted = lists:sort(fun({_, A}, {_, B}) -> A < B end, Lines),
	Severity = severity(Sorted),
	io:format("~p~n", [Severity]),
	true = Severity =:= 1876,
	Delay = delay(Sorted),
	io:format("~p~n", [Delay]),
	true = Delay =:= 3964778,
	ok.

caught({D, C}) ->
	case D rem C of
		0 -> D * (C div 2 + 1);
		_ -> 0
	end.

caught({D, C}, T) ->
	((D + T) rem C) =:= 0.

severity(Xs) ->
	severity(Xs, 0).

severity([], N) -> N;
severity([X|Xs], N) ->
	severity(Xs, N + caught(X)).

good_trip([], _) -> true;
good_trip([X|Xs], T) ->
	not caught(X, T) and good_trip(Xs, T).

delay([{D,2}|Xs]) ->
	N = bool_num(caught({D,2}, 0)),
	delay(Xs, N, 2);
delay(Xs) ->
	delay(Xs, 0, 1).

delay(Xs, N, Incr) ->
	case good_trip(Xs, N) of
		true -> N;
		false -> delay(Xs, N+Incr, Incr)
	end.

bool_num(true) -> 1;
bool_num(false) -> 0.

read_lines() ->
	read_lines([]).

read_lines(X) ->
	case io:fread("", "~d: ~d") of
		{ok, [D, R]} -> read_lines([{D,(R-1)*2}|X]);
		_ -> X
	end.
