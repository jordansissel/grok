#!/usr/bin/perl

use strict;
use warnings;

use re 'eval';
use IO::Handle;
use Socket;
use Regexp::Common;

my $re = qr/($RE{net}{IPv4})/; # Look for a token matching this
my $subre = qr/5$/;             # But the token must also match this

my $stopre = '(?=.\A)';         # A regexp guaranteed to fail.

socketpair(CHILD, PARENT, AF_UNIX, SOCK_STREAM, PF_UNSPEC);
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
  my $str = "$ARGV[0]";
  my $match = $str =~ m/($re)(??{ check($^N); })/;

  print "Match: $match\n";
  if ($match) {
    print "Group: $1\n";
  }
}

sub check {
  my ($word) = @_;
  print CHILD "$word\n";
  print CHILD "$subre\n";

  #print STDERR "parent reading  from result\n";
  chomp(my $result = <CHILD>);
  #print STDERR "parent reading  complete\n";
  return "$result";
}

sub child {
  $SIG{PIPE} = sub { print STDERR "child got sigpipe\n"; exit(1) };

  while (1) {
    #print STDERR "Child loop start\n";
    my $word = <PARENT>;
    my $subre = <PARENT>;
    last if (! ($word && $subre) );
    $word = chomp($word);
    $subre = chomp($subre);
    print "$word / $subre\n";
    if ($word =~ m/$subre/) {
      #print STDERR "Sub re match\n";
      print PARENT "\n";
    } else {
      #print STDERR "Sub re no match\n";
      print PARENT "$stopre\n";
    }
  }
}
