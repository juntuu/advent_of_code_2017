#!/usr/bin/perl

use strict;
use warnings;

use List::Util 'sum';

my $start;
my $steps;
my $st;
my $val;
my %tape;
my %state;

while (<>) {
	s/[\n.:]+$//;
	my @fields = split /\s+/;

	if (/Begin in state/) {
		$start = $fields[-1];
	} elsif (/Perform a diagnostic/) {
		$steps = $fields[-2];
	} elsif (/In state/) {
		$st = $fields[-1];
	} elsif (/If the current value is/) {
		$val = $fields[-1];
	} elsif (/Write the value/) {
		$state{$st, $val, 'write'} = $fields[-1];
	} elsif (/Move one slot to the/) {
		$state{$st, $val, 'move'} = /left/ ? -1 : 1;
	} elsif (/Continue with state/) {
		$state{$st, $val, 'continue'} = $fields[-1];
	}
}

my $at = 0;
while ($steps--) {
	$val = $tape{$at} // 0;
	$tape{$at} = $state{$start, $val, 'write'};
	$at += $state{$start, $val, 'move'};
	$start = $state{$start, $val, 'continue'};
}

print sum(values %tape), "\n";
