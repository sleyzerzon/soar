#!/bin/sh
#
# Generates cli_help.cpp from the command documentation on the Google
# Code wiki.  Will try to use text files cached in helptext/, so if you
# want to pull a fresh copy from Google Code, delete helptext/.
#
# Wiki html -> text conversion is performed by w3m, which you can get at
# http://w3m.sourceforge.net
#
# Joseph Xu March 2011

cachedir=helptext
html2text='w3m -dump -T text/html -r'
export LANG=ascii  # necessary to get w3m to avoid weird characters

# For some reason ?show=content is broken for anything except firefox,
# this is a work-around.
getpage() {
	curl -s "http://code.google.com/p/soar/wiki/$1?show=content" | awk '/wikicontent/ {p=1} /commentform|artifactcomment/ {p=0} p==1 {print}'
}

if [ ! -d "$cachedir" ]
then
	mkdir "$cachedir"
fi

cmds=`getpage CommandIndex | $html2text | awk '/\*/ {print $2}'`

for c in $cmds
do
	if [ ! -f "$cachedir/$c" ]
	then
		page=`echo $c | sed 's/^/cmd_/
		                     s/-/_/g'`
		getpage $page | $html2text > "$cachedir/$c"
	fi
done

echo "/* Auto-generated by gen_cli_help.sh on `date` */" > src/cli_help.cpp
cat cli_help_head.cpp >> src/cli_help.cpp

for f in $cachedir/*
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
