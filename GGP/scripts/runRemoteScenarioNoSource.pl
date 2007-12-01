#!/usr/bin/perl

$dir = $ARGV[0];
$machine = $ARGV[1];
$env = $ARGV[2];
$level = $ARGV[3];
$scenario = $ARGV[4];

if ($env =~ "10") {
  # level 10 has no environment
  $env = "";
  $scenario = $level;
  $level = 10;
  die unless ($#ARGV == 3);
}
else {
  die unless  ($#ARGV == 4);
}

$runScenario = "./runScenarioNoSource.pl";

%machineAliases = ();
%machineDirs = ();

$machineAliases{"b1"} = "bahamut";
$machineDirs{"b1"} = "GGP/scripts";
$machineAliases{"b"} = "bahamut";
$machineDirs{"b"} = "GGP/scripts";
$machineAliases{"b2"} = "bahamut";
$machineDirs{"b2"} = "2GGP/GGP/scripts";
$machineAliases{"b3"} = "bahamut";
$machineDirs{"b3"} = "2GGP/GGP/scripts";
$machineAliases{"w1"} = "wyrm";
$machineDirs{"w1"} = "GGP/scripts";
$machineAliases{"w"} = "wyrm";
$machineDirs{"w"} = "GGP/scripts";
$machineAliases{"w2"} = "wyrm";
$machineDirs{"w2"} = "2GGP/GGP/scripts";
$machineAliases{"s1"} = "smaug";
$machineDirs{"s1"} = "GGP/scripts";
$machineAliases{"s"} = "smaug";
$machineDirs{"s"} = "GGP/scripts";
$machineAliases{"s2"} = "smaug";
$machineDirs{"s2"} = "2GGP/GGP/scripts";
$machineAliases{"r1"} = "141.212.109.197";
$machineDirs{"r1"} = "GGP/scripts";
$machineAliases{"r"} = "141.212.109.197";
$machineDirs{"r"} = "GGP/scripts";
$machineAliases{"r2"} = "141.212.109.197";
$machineDirs{"r2"} = "2GGP/GGP/scripts";
$machineAliases{"a"} = "auk";
$machineDirs{"a"} = "GGP/scripts";
$machineAliases{"g"} = "grapes";
$machineDirs{"g"} = "GGP/scripts";
$machineAliases{"bb"} = "badboy";
$machineDirs{"bb"} = "GGP/scripts";
$machineAliases{"f"} = "flamingo";
$machineDirs{"f"} = "GGP/scripts";
$machineAliases{"m1"} = "d-109.232";
$machineDirs{"m1"} = "GGP/scripts";
$machineAliases{"m2"} = "d-109.232";
$machineDirs{"m2"} = "2GGP/GGP/scripts";
$machineAliases{"n1"} = "winter";
$machineDirs{"n1"} = "GGP/scripts";
$machineAliases{"n2"} = "winter";
$machineDirs{"n2"} = "2GGP/GGP/scripts";
$machineAliases{"y1"} = "d-109.245";
$machineDirs{"y1"} = "GGP/scripts";
$machineAliases{"y2"} = "d-109.245";
$machineDirs{"y2"} = "2GGP/GGP/scripts";

die unless (defined $machineAliases{$machine});

if ($env eq "e") {
  $longEnv = "escape";
}
elsif ($env eq "w") {
  $longEnv = "wargame";
}
elsif ($env eq "r") {
  $longEnv = "mrogue";
}
elsif ($env eq "b") {
  $longEnv = "build";
}
else {
  $longEnv = "differing";
}

$run_cmd = "ssh $machineAliases{$machine} \"cd $machineDirs{$machine}; $runScenario $env $level $scenario\"";
$copy_cmd = "scp $machineAliases{$machine}:$machineDirs{$machine}/$longEnv-$level-$scenario-* $dir/";
print $run_cmd;
system($run_cmd);
print $copy_cmd;
system($copy_cmd);
