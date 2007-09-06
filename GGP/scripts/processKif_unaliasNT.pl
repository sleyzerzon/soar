#!/usr/bin/perl
# change the format of a kif file to something easier to parse in perl

foreach $line (`cat $ARGV[0]`) {
  if ($line =~ /\(or (.*)\)\s*$/) {
    $line = $1;
#    print $line;
  }
  while ($line =~ /^(\s*\(.*?\))\s*\(/) {
    $part = $1;
    $line =~ s/^\s*\(.*?\)\s*\(/\(/;
    handleLine($part);
  }

  handleLine($line);
}
sub handleLine() {
  my $line = shift;
  $line=~ s/\s*#.*//;
  $line=~ s/\s*;.*//;
  $line =~ s/^\s*//;
  $line =~ s/\s*$//;
  if ($line =~ /^$/) { next; }
  $line = "$line\n";
  $line =~ s/ \)/\)/g;
  $line =~ s/ /:/g;

  #$line = lc $line;
  
  if ($line =~ /^\(<=:/) {
    $line =~ s/^\(<=://;
    print "BEGIN ";
  }


  if ($line =~ /^\(/) {
    $line =~ s/^\(//;
    $line =~ s/\)$//;
  }
  if ($line =~ /true:\(/) {
    $line =~ s/true:\(/TRUE:/;
    $line =~ s/\)$//;
  }
  if ($line =~ /not:\(/) {
    $line =~ s/not:\(/NOT /;
    $line =~ s/\)$//;
  }
  $line =~ s/not:/NOT /;
 

  if ($line =~/:\(/) {
    $line =~ s/:\(/:/;
    $line =~ s/\)//;
  }

  $line =~ s/\)$/\nEND/g;

  $line =~ s/goal:[^:]*:(\d+)/NEXTgoal$1/;
  $line =~ s/ legal:[^:]*:/ L/;
  $line =~ s/^legal:[^:]*:/L/;
  $line =~ s/ next:/ NEXT:/;
  $line =~ s/^next:/NEXT:/;
  $line =~ s/ does:[^:]*:/ AXN/;
  $line =~ s/^does:[^:]*:/AXN/;
  $line =~ s/\?/V/g;
  print $line;
}
