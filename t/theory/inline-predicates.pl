#!/usr/bin/perl -l

use strict;
use warnings;

use re 'eval';
use Regexp::Common;
use Data::Dumper;

my %MATCH = (
  USERNAME => qr/[a-zA-Z0-9_-]+/,
  USER => "%USERNAME%",
  INT => qr/$RE{num}{int}/,
  NUMBER => qr/$RE{num}{real}/,
  WORD => qr/\w+/,
  NOTSPACE => qr/\S+/,
  DATA => qr/.*?/,
  GREEDYDATA => qr/.*/,
  QUOTEDSTRING => $RE{quoted},

  # Networking
  MAC => qr/(?:%CISCOMAC%|%WINDOWSMAC%|%COMMONMAC%)/,
  CISCOMAC => qr/(?:(?:[A-Fa-f0-9]{4}\.){2}[A-Fa-f0-9]{4})/,
  WINDOWSMAC => qr/(?:(?:[A-Fa-f0-9]{2}-){5}[A-Fa-f0-9]{2})/,
  COMMONMAC => qr/(?:(?:[A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2})/,
  IP => $RE{net}{IPv4},
  HOSTNAME => qr/$RE{net}{domain}{-nospace}/,
  HOST => "%HOSTNAME%",
  IPORHOST => "(?:%IP%|%HOSTNAME%)",
  
  # Months: January, Feb, 3, 03, 12, December
  MONTH => qr/\b(?:Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?|0?[0-9]|1[0-2])\b/,
  MONTHDAY => qr/(?:(?:[0-2]?[0-9])|3[01])/,
  
  # Days: Monday, Tue, Thu, 0 (Sunday?), 6 (Saturday?)
  DAY => qr/(?:Mon(?:day)?|Tue(?:sday)?|Wed(?:nesday)?|Thu(?:rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?|[0-6])/,
  
  # Years?
  YEAR => qr/%INT%/,
  # Time: HH:MM:SS
  TIME => qr/\d{2}:\d{2}(?::\d{2})?/,
  
  # Syslog Dates: Month Day HH:MM:SS 
  SYSLOGDATE => "%MONTH% +%MONTHDAY% %TIME%",
  PROG => "%WORD%",
  PID => "%INT%",
  SYSLOGPROG => qr/%PROG%(?:\[%PID%\])?/,
  HTTPDATE => qr,%MONTHDAY%/%MONTH%/%YEAR%:%TIME% %INT:ZONE%,,
  
  # Shortcuts
  QS => "%QUOTEDSTRING%",
  
  # Log formats 
  SYSLOGBASE => "%SYSLOGDATE% %HOSTNAME% %SYSLOGPROG%:",
  APACHELOG => "%IPORHOST% %USER:IDENT% %USER:AUTH% \\[%HTTPDATE%\\] %QS:URL% %NUMBER:RESPONSE% %NUMBER:BYTES% %QS:REFERRER% %QS:AGENT%",
);

sub pattern2regex {
  my $pattern = shift;
  my $regex = $pattern;
  my $orig = $pattern;
  my $predicate_regex = qr@
      (
       (?:(?:[<>]=?|==)[^%]+)
       |
       (?:~$RE{delimited}{-delim=>'/'})
      )(?{ $predicate = $^N; })
     @x;

  my $count;
  do {
    my $rounds = 0;
    $count = 0;
    # Replace %KEY:SUBNAME% with a regex and store the capture as KEY:SUBNAME
    # The :SUBNAME specification is optional,
    # ie; %IP% is valid, as is %IP:FOO%
    for my $key (keys(%MATCH)) {
      my $predicate = "";
      my $re = qr,%
                   (
                    (?:$key)                # Pattern name
                    (?::\w+)?             # Pattern subname
                   )
                   (?:$predicate_regex)? # Post-match predicate
                  %
                 ,x;
      if ($regex =~ m/$re/) {
        print "prematch\n";
        $count += 
          $regex =~ 
            s@$re@($MATCH{$key})(??{foo("$predicate")})(?{ \$values{'$1'} = \$^N })@g;
        print "postmatch\n";
      }

      #s@$re@($MATCH{$key})$predicate(?{ \$values{'$1'} = \$^N; \$predicates{'$1'} = '$predicate' })@g;
    }
    $rounds++;
    if ($rounds > 20) {
      debug(0, "Deep recursion translating '$orig'");
      last;
    }
  } while ($count > 0);

  return $regex;
}

sub foo {
  my $predicate = shift;

  if ($predicate =~ m@^~@) {
    $predicate = substr($predicate, 2, -1);
    print STDERR "MATCH: $^N";
  }
  return ""
}

package Foo;

sub TIESCALAR {
  my $class = shift;
  my $self = shift;
}
sub FETCH { }

package main;
my $re = pattern2regex($ARGV[0]);
print $re;
my (%values, %predicates);

<STDIN> =~ m/$re/;
print Dumper(\%values);



