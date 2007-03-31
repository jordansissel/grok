#!/usr/bin/perl -l

my $foo = "test one two";
my $bar = "one";
sub test { "testing" }

$foo =~ s/test/(??{test($bar)})/g;
print $foo
