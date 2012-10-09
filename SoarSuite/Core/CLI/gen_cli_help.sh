#!/bin/sh
#
# Generates cli_help.cpp from the command documentation on the Google
# Code wiki.
#
# Wiki html -> text conversion is performed by w3m, which you can get at
# http://w3m.sourceforge.net
#
# Joseph Xu March 2011
# Modified  March 2012

TEXTDIR=helptext
HTML2TEXT='w3m -O ascii -dump -T text/html -r'

if ! [ -x "`which w3m`" -a -x "`which curl`" ]
then
	echo please install w3m and curl
	exit 1
fi

rm -rf "$TEXTDIR"
mkdir "$TEXTDIR"

if ! svn export -q --force http://soar.googlecode.com/svn/wiki/CommandIndex.wiki
then
	echo cannot download command index page from svn repo
	exit 1
fi

# For each command listed in CommandIndex, run its html through w3m and save the output in $TEXTDIR
awk -v "outdir=$TEXTDIR" -v "html2text=$HTML2TEXT" '
	function getpage(page, outfile,    curlcmd, p, line) {
		curlcmd = "curl -s http://code.google.com/p/soar/wiki/" page "?show=content"
		convcmd = html2text " > " outfile
		while ((curlcmd | getline line) > 0) {
			# filter out the header and comment box
			if (line ~ /wikicontent/) {
				p = 1
			} else if (line ~ /commentform|artifactcomment/) {
				p = 0
			}
			if (p == 1) {
				print line | convcmd
			}
		}
		close(curlcmd)
		close(convcmd)
	}
	
	$1 == "*" {
		gsub("\\]|\\[", "")
		page = $2
		cmd = $3
		getpage(page, outdir "/" cmd)
	}' CommandIndex.wiki

echo "/* Auto-generated by gen_cli_help.sh on `date` */" > src/cli_help.cpp
cat cli_help_head.cpp >> src/cli_help.cpp

for f in $TEXTDIR/*
do
	c=`basename $f`
	awk '
		BEGIN {
			cmdname = "'$c'"
			inhead = 1
		}
		inhead == 1 {
			if (NF > 1 || (NF == 1 && $1 != cmdname)) {
				inhead = 0
				linecount = 1
			}
		}
		inhead == 0 {
			if (lines[linecount-1] ~ /^[ \t]*$/ && $0 ~ /^[ \t]*$/) {
				# skip double blanks
				next
			}
			lines[linecount++] = $0
		}
		END {
			for (i = linecount; i >= 1; i--) {
				if (lines[i] !~ /^[ \t]*$/) {
					last = i
					break
				}
			}
			print "	docstrings[\"" cmdname "\"] = "
			for (i = 1; i <= last; i++) {
				gsub("\\\\", "\\\\", lines[i])
				gsub("\"", "\\\"", lines[i])
				gsub("&#x27;", "'"'"'", lines[i])
				print "		\"" lines[i] "\\n\""
			}
			print "	;"
		}' "$f" >> src/cli_help.cpp
done

echo "}" >> src/cli_help.cpp
