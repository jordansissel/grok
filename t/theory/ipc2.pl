#!/usr/bin/perl

use strict;
use warnings;

use IO::Handle;

pipe(READ, WRITE);
WRITE->autoflush(1);

if (fork() > 0) {
  parent();
} else {
  child();
}

exit(0);

sub parent {
  while (1) {
    print WRITE "parent says hi\n";
    my $data = <READ>;
    print "parent: $data\n";
  }
}

sub child {
  while (1) {
    my $data = <READ>;
    print "child: $data\n";
    print WRITE "child says hi\n";
  }
}
