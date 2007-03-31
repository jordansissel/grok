#!/usr/bin/perl -l

package Foo;
use Tie::Scalar;

our @ISA = (Tie::Scalar);

sub TIESCALAR {
  my $class = shift;
  my $self = {};
  
  ($self->{"initial"}) = (@_);
  return bless $self, $class;
} 

sub FETCH { 
  my $self = shift;
  return ($self)->{"initial"};
} 

package main;

my $x;
tie $x, 'Foo', "foo bar baz";;

print $x
