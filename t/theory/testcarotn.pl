#!/usr/bin/perl

use strict;
use re 'eval';

my ($foo,$bar);

$foo = "testing";
$foo =~ m/(test)(?{ $bar = $^N })/;

print "\$^N works\n" if ($bar eq 'test');
print "\$^N DOES NOT WORK \n" if ($bar ne 'test');
