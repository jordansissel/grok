#!/usr/bin/perl -l

use strict;
use warnings;
use Regexp::Common;
use Algorithm::Diff qw(LCS);

my $token_re = qr/($RE{quoted}|\b\w+\b)/;

my @lines;
my @tokens;
while (<STDIN>) {
  chomp();
  my @x = m/$token_re/g;
  #print "Token count: " . scalar(@x) . "\n";
  push(@tokens, \@x);
  push(@lines, $_);
}

my $x = 0;
for my $i (@tokens) {
  _lcs($i, $x, \@tokens);
  $x++;
}

sub jointok { return join(" ", @_); }

sub _lcs {
  my ($cur_tokens, $x, $tokensets) = @_;
  my $y = 0;
  for my $i (@$tokensets) {
    next if $x == $y; # Don't compare me against myself
    #my @seq = LCS($cur_tokens, $tokensets->[$y]);
    #print "---";
    #print "  Longest: ", join(", ", map("'$_'", @seq));
    #print jointok(@$cur_tokens);
    #print jointok(@{$tokensets->[$y]});
    my $d = Algorithm::Diff->new($cur_tokens, $tokensets->[$y]);
    while ($d->Next()) {
      next if $diff->Same();
####
    }
    $y++;
  }
}

sub _lcs_find {
  my ($a, $b) = @_;
  my @seq;
  my ($ai, $bi) = (0, 0);

  my ($curstart, $curend) = (-1, -1);
  my ($longstart, $longend) = (-1, -1);

  while ($ai < scalar(@$a) && $bi < scalar(@$b)) {
    if ($a->[$ai] eq $b->[$bi]) {
      # track start/end of equality
    }
  }
}
