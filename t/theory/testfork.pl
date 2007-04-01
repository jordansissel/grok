#!/usr/bin/perl

use strict;
use warnings;

use re 'eval';
use IO::Handle;

my $re = qr/(\b\w+\b)/;
my $subre = qr/bar/;
my $stopre = '(?=.\A)';

pipe(RPCREAD, RPCWRITE);
pipe(RESULTREAD, RESULTWRITE);

RPCWRITE->autoflush(1);
RESULTWRITE->autoflush(1);
RPCREAD->autoflush(1);
RESULTREAD->autoflush(1);

if (fork() > 0) {
  parent()
} else {
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
  print RPCWRITE "$word\n";
  print RPCWRITE "$subre\n";
  print STDERR "parent reading  from result\n";
  my $result; # = <RESULTREAD>;
  print STDERR "parent reading  complete\n";
  print STDERR "Result: $result\n";
  return "(bar)";
}

sub child {
  RPCWRITE->autoflush(1);
  RESULTWRITE->autoflush(1);
  RPCREAD->autoflush(1);
  RESULTREAD->autoflush(1);

  $SIG{PIPE} = sub { print STDERR "child got sigpipe\n"; exit(1) };
  while (1) {
    print STDERR "Child loop start\n";
    chomp(my $word = <RPCREAD>);
    chomp(my $subre = <RPCREAD>);
    print "$word / $subre\n";
    if ($word =~ m/$subre/) {
      print STDERR "Sub re match\n";
      print RESULTWRITE "\n";
    } else {
      print STDERR "Sub re no match\n";
      print RESULTWRITE "$stopre\n";
    }
    die "Child suicide";
  }
}
