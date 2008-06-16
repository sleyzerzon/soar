#!/usr/bin/perl
# Jonathan Voigt
# 2006

use strict;

my $cliSource = "../../../Core/CLI/src/cli_Commands.cpp";
my $commandNames = "../command-names";
my $htmlProcess = "./html-process.pl";
my $latexProcess = "./latex-post-process.pl";

# Generate command-names from source
open COMMANDS, "<$cliSource" or die "Could not open $cliSource: $!";
my @commandsSource = <COMMANDS>;
close COMMANDS;

open NAMES, ">$commandNames" or die "Could not open $commandNames: $!";

print "Commands found: ";
my @commands;

foreach (@commandsSource) {
	if (/char\s+const\s*\*\s*Commands::kCLI\w+\s*=\s*"([\w-]+)"/) {
		push(@commands, $1);
		print $1 . " ";
		print NAMES $1 . "\n";
	}
}
print "\n";

close NAMES;

open POSTPROCESSFILE, "$htmlProcess" or die "Could not find required file $htmlProcess: $!";
close POSTPROCESSFILE;

foreach my $command (@commands) {
	chomp $command;
	print "Processing $command\n";

	my $commandWiki = "../$command.wiki.html";
	my $commandHtml = "../$command.html";
	my $commandFile = "../$command";
	my $commandTex  = "../$command.tex";

	`wget -q -O $commandWiki http://winter.eecs.umich.edu/soarwiki/Documentation/CLI/$command`;

	`$htmlProcess $commandWiki > $commandHtml`;
	unlink $commandWiki or die "Could not remove $commandWiki: $!";

	`elinks -dump -no-numbering -no-references $commandHtml > $commandFile`;
	
#	`gnuhtml2latex --nopar $commandHtml`;
#	`$latexProcess < $commandTex > $commandTex.new`;
#	rename ("$commandTex.new", "$commandTex") or die "Could not rename $commandTex.new: $!";
}
  #`pdflatex --interaction=nonstopmode @names[$i].tex`;
