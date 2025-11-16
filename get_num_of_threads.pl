#!/usr/bin/perl

use strict;
use warnings;

chomp(my $hostname = `hostname`);
while (<>) { /threads=(\d+)/ and print($1), last if /$hostname/ }


