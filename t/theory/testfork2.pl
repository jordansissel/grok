#!/usr/bin/perl

use strict;
use warnings;

use re 'eval';
use IO::Handle;
use Socket;
use Regexp::Common;

my $re = qr/($RE{net}{IPv4})/; # Look for a token matching this
my $subre = qr/5/;             # But the token must also match this

my $stopre = '(?=.\A)';         # A regexp guaranteed to fail.

parent();

exit(0);

sub parent {
  my $str = "$ARGV[0]";
  my $match = $str =~ m/($re)(??{ check($^N); })/;
  print "N: $^N\n";

  print "Match: $match\n";
  if ($match) {
    print "Group: $1\n";
  }
}

sub check {
  my ($word) = @_;
  print "$word / $subre\n";
  if ($word =~ m/$subre/) {
    print "sub match\n";
    return "";
  } else {
    print "no sub match / $stopre\n";
    return "$stopre";
  }
}

