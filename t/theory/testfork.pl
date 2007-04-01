#!/usr/bin/perl

use strict;
use warnings;

use re 'eval';
use IO::Handle;
use Socket;

my $re = qr/(\b\w+\b)/;
my $subre = qr/bar/;
my $stopre = '(?=.\A)';

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
  my $str = "foo foobar foobaz";
  local $/ = undef;
  my $match = $str =~ m/($re)(??{ check($^N); })/;

  print "Match: $match\n";
  print "Group: $1\n";
}

sub check {
  my ($word) = @_;
  print CHILD "$word\n";
  print CHILD "$subre\n";

  sleep(1);
  print STDERR "parent reading  from result\n";
  my $result = <CHILD>;
  print STDERR "parent reading  complete\n";
  print STDERR "Result: $result\n";
  return "(bar)";
}

sub child {
  $SIG{PIPE} = sub { print STDERR "child got sigpipe\n"; exit(1) };

  while (1) {
    print STDERR "Child loop start\n";
    chomp(my $word = <PARENT>);
    chomp(my $subre = <PARENT>);
    print "$word / $subre\n";
    if ($word =~ m/$subre/) {
      print STDERR "Sub re match\n";
      print PARENT "nothing\n";
    } else {
      print STDERR "Sub re no match\n";
      print PARENT "$stopre\n";
    }
    close(PARENT);
    die;
  }
}
