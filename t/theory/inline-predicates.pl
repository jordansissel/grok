#!/usr/bin/perl

use strict;
use warnings;


my $data = "foo footest bar foozle foobar foobaz";

print "Match: " . $data =~ m/(foo\w+(?<=bar))/ . "\n";

print "$1\n";
