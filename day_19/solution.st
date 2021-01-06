PackageLoader fileInPackage: 'Complex'.

grid := Dictionary new.

row := 0.
[ stdin atEnd ] whileFalse: [ | line |
	line := stdin nextLine.
	(line =~ '\S') ifMatched: [
		grid at: row put: line.
	].
	row := row + 1.
].

start := (grid at: 1) indexOf: $| startingAt: 0.

dir := 1 i.
steps := 0.

space := Character space.

[ grid isEmpty ] whileFalse: [ | char line index |
	steps := steps + 1.
	(grid includesKey: (start imaginary)) ifTrue: [
		index := start real.
		line := grid at: (start imaginary).
		char := line at: index.
		line at: index put: space.
		(line =~ '\S') ifNotMatched: [
			grid removeKey: (start imaginary).
		].
		char = $+
		ifTrue: [
			index := start + ( dir * 1 i ).
			line := grid at: (index imaginary) ifAbsent: [ '' ].
			(line at: (index real) ifAbsent: [ space ]) = space
			ifTrue: [ dir := dir * -1i. ]
			ifFalse: [ dir := dir * 1i. ].
		]
		ifFalse: [
			char isUppercase ifTrue: [ char display. ].
		].
	].
	start := start + dir.
].

Character nl display.
steps printNl.
