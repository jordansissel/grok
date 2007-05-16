#!/usr/bin/perl

use strict;
use warnings;

use IO::Handle;
use Socket;

socketpair(CHILD, PARENT, AF_UNIX, SOCK_DGRAM, PF_UNSPEC);
CHILD->autoflush(1);
PARENT->autoflush(1);

if (fork() > 0) {
  close(PARENT);
  parent();
} else {
  close(CHILD);
  child();
}

exit(0);

sub parent {
  while (1) {
    print CHILD "parent says hi\n";
    my $data = <CHILD>;
    print "parent: $data\n";
  }
}

sub child {
  while (1) {
    my $data = <PARENT>;
    print "child: $data\n";
    print PARENT "child says hi\n";
  }
}
