#!/usr/bin/perl
# Used by grok, not a standalone tool.
use re 'eval';

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
#print join("\n", @sorted_patterns);

while (<STDIN>) {
  chomp($_);
  analyze($_);
}

exit(0);

sub count_delim {
  my $x = shift;
  $x =~ s/\(\?\?\{ handle_capture[^}]+\}\)//g;
  #print "$x\n";
  my @m = $x =~ m/(\s|\\s|[.r-])/g;
  return scalar(@m);
}

sub noncapture_length {
  my $x = shift;
  $x =~ s/\(\?\?\{ handle_capture[^}]+\}\)//g;
  return length($x);
}

sub compare_by_complexity {
  my ($a,$b) = @_;
  my $a_re = pattern2regex($MATCH{$a});
  my $b_re = pattern2regex($MATCH{$b});
  my $a_delim = count_delim($a_re);
  my $b_delim = count_delim($b_re);
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

  while (++$count <= 10) {
    pos($line) = 0;
    foreach my $name (@sorted_patterns) {
      last if (pos($line) == $#line);
      next if ($name =~ m/$skip_re/);

      # Skip %FOO%
      while ($line =~ m/\G%/) {
        $line =~ m/(%[A-Z]+%)/;
        pos($line) = $+[1];
      }

      my $regex = pattern2regex("%$name~/[^A-z]/%");
      #print "   -> against $name\n";
      #print "   // $regex\n";
      #print "   ... " . substr($line, pos($line)) . "\n";

      if ($line =~ m/\G.*?($regex)/) {
        my $match = $1;
        my ($start, $end) = ($-[1], $+[1]);

        # Don't bother injecting a pattern if we aren't a nonword.

        my $patstr = "%$name%";
        substr($line, $start, $end - $start) = $patstr;
        pos($line) = $start + length($patstr) + 1;
        #print " -> $line\n";
        #print "Pos: " . pos($line) . "\n";
        last;
      }
    }
  }

  print "$line\n";
}

1;
