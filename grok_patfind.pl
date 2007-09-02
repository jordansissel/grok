#!/usr/bin/perl
# Used by grok, not a standalone tool.
use re 'eval';
use Memoize;

memoize('strip_code');
memoize('delimiter_score');

$| = 1;

our $opts;
my $config = readconfig($opts->{"f"});
unless (defined($config)) {
  debug(0, "Syntax error in config?");
  exit(1);
}
setup($config);

# Remove things that are too general
my @skippable = qw(
  DATA GREEDYDATA USER USERNAME WORD NOTSPACE PID PROG YEAR
);
my $skip_re = "^(?:" . join("|",@skippable) . ')$';

our %MATCH;
my @sorted_patterns = sort { compare_by_complexity($a,$b); } keys(%MATCH);

#for my $i (@sorted_patterns) {
  #print delimiter_score(pattern2regex("%$i%")), " ", $i, "\n";
#}

while (<STDIN>) {
  chomp($_);
  analyze($_);
}

finish_and_exit();

sub strip_code {
  $_[0] =~ s/\(\?\?\{ handle_capture[^}]+\}\)//g;
  return $_[0];
}

sub delimiter_score {
  my $x = strip_code(shift);
  my $score = 0;
  my @m;
  # spaces are valued
  #print "re: $x\n";
  @m = $x =~ m/(?:\s|\\s)/g;
  $score += 1.5 * scalar(@m);
  #print "  space: " . scalar(@m) . "\n";

  # dots are good too
  @m = $x =~ m/(?:[.])/g;
  $score += 1 * scalar(@m);
  #print "  dots: " . scalar(@m) . "\n";

  # some other punctuation
  @m = $x =~ m/(?:['":_,=+-])/g;
  $score += .6 * scalar(@m);
  #print "  punct: " . scalar(@m) . "\n";

  return $score;
}

sub noncapture_length {
  my $x = strip_code(shift);
  return length($x);
}

# Comparator for sorting regexps by some kind of complexity
# - score first is checked by number of delimiters checked 
# - if equal, compare the lengths of the regexps(*)
# (*) length calculated on regexps with any (??{ }) removed..

sub compare_by_complexity {
  my ($a,$b) = @_;
  my $a_re = pattern2regex($MATCH{$a});
  my $b_re = pattern2regex($MATCH{$b});
  my $a_delim = delimiter_score($a_re);
  my $b_delim = delimiter_score($b_re);
  my $a_len = noncapture_length($a_re);
  my $b_len = noncapture_length($b_re);

  #print "$a_re\n";
  #print "-> $a_delim\n";
  #print "$a_re\nvs\n$b_re\n\n";
  return $b_delim <=> $a_delim or $b_len <=> $a_len
}

sub analyze {
  my $line = shift;

  #print "Checking '$line'\n";
  my $count = 0;

  # Counts of uses of each pattern name
  my %patcounts;

  # list of 2-tuple containing start/end ranges of already performed matches.
  my @skip_ranges;

  while (++$count <= 10) {
    pos($line) = 0;
    
    # Track how many matches were this round.
    my $round_matches = 0;
    foreach my $name (@sorted_patterns) {
      last if (pos($line) == $#line);
      next if ($name =~ m/$skip_re/);

      # Skip %FOO% and %FOO:BAR%
      #print "str: " . substr($line, pos($line)) . "\n";
      #print "skip? " . ($line =~ m/\G(%[A-z0-9:]+%)/) . "\n";
      #while ($line =~ m/\G(%[A-z0-9:]+% *)/) {
        #pos($line) = $+[1];
        ##print "Skipping past (endpos $+[1]): $&\n";
      #}

      # match %FOO~/[^A-z/% - 
      # Make sure this token is not just a plain word
      my $regex = pattern2regex("%$name~/[^A-z0-9]/%");
      #print "   -> against $name\n";
      #print "   // $regex\n";
      #print "   ... " . substr($line, pos($line)) . "\n";

  try:
      if ($line =~ m/\G.*?($regex)/) {
        my $match = $1;
        my ($start, $end) = ($-[1], $+[1]);
        for my $i (@skip_ranges) {
          my ($rstart,$rend) = @$i;
          if (($start > $rstart && $start < $rend)
              || ($end > $rstart && $end < $rend)) {
            #print "Skipping to pos $rend: match was inside " . substr($line, $rstart, $rend - $rstart) . "\n";
            pos($line) = $rend;
            goto try;
          }
        }

        $round_matches++;

        my $subname = "";
        $subname = ":" . ($patcounts{$name} + 1) if ($patcounts{$name});
        my $patstr = "%$name$subname%";

        #print "   -> against $name\n";
        #print "   -> str " . substr($line, $start) . "\n";
        substr($line, $start, $end - $start) = $patstr;
        #print " -> $line\n";
        pos($line) = $start + length($patstr);

        # Shift everything left that's to the right of our match
        for my $i (@skip_ranges) {
          my ($rstart,$rend) = @$i;
          if ($rstart > $end) {
            $i->[0] = $rstart - ($end - $start) + length($patstr);
            $i->[1] = $rend - ($end - $start) + length($patstr);
          }
        }

        push(@skip_ranges, [$start, pos($line)]);
        #print "Pos: " . pos($line) . "\n";
        $patcounts{$name}++;
        last;
      }
    }

    last if ($round_matches == 0);
  }

  print "$line\n";
}

1;
