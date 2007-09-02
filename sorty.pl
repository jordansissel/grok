#!/usr/bin/perl -l

sub cnt { @tmp = $_[0] =~ m/X[-+]{3}X/g; scalar(@tmp) };
sub c($$) { ($a, $b) = @_; cnt($a) <=> cnt($b) };

@a = (
 "X---X " x 10,
 "X---X " x 3,
 "X---X " x 2,
 "foo X---X " x 5,
);

for $i (sort c @a) {
  print $i;
}
