<?php
$moves = explode(",", readline());
$programs = [];
for ($j = 0; $j < 16; $j++) {
	$programs[$j] = "abcdefghijklmnop"[$j];
}

function spin($arr, $n) {
	$len = count($arr);
	$new = [];
	$k = 0;
	for ($j = $len -  $n; $j < $len; $j++) {
		$new[$k++] = $arr[$j];
	}
	for ($j = 0; $j < $len -  $n; $j++) {
		$new[$k++] = $arr[$j];
	}
	return $new;
}

function exchange($arr, $n) {
	$temp = $arr[$n[0]];
	$arr[$n[0]] = $arr[$n[1]];
	$arr[$n[1]] = $temp;
	return $arr;
}

function partner($arr, $n) {
	$a = array_search($n[0], $arr);
	$b = array_search($n[1], $arr);
	return exchange($arr, [$a, $b]);
}

function dance($m, $p) {
	foreach ($m as $move) {
		$rest = substr($move, 1);
		switch ($move[0]) {
		case "s":
			$p = spin($p, $rest);
			break;
		case "x":
			$p = exchange($p, explode("/", $rest));
			break;
		case "p":
			$p = partner($p, explode("/", $rest));
			break;
		}
	}
	return $p;
}

$programs = dance($moves, $programs);
$s = implode("", $programs);
echo $s, "\n";

function find_cycle($m, $p) {
	$n = 0;
	$seen = [implode("", $p)];
	while (TRUE) {
		$n++;
		$p = dance($m, $p);
		$s = implode("", $p);
		if (in_array($s, $seen))
			return $n;
		$seen[] = $s;
	}
}

$billion = 1000000000;
$rem = $billion % find_cycle($moves, $programs);
while (--$rem > 0) {
	$programs = dance($moves, $programs);
}
$s = implode("", $programs);
echo $s, "\n";
?>
