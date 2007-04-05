#!/usr/bin/python

import os

def removeLines(file):
	f = open(file)
	lines = f.readlines()
	f.close()

	if lines[0].find('HAVE_CONFIG_H') == -1:
		print "Warning: skipping", file
		return

	lines = lines[3:]
	
	f = open(file, 'w')
	f.writelines(lines)
	f.close()

for file in os.listdir('.'):
	if file.endswith('.cpp'):
		removeLines(file)
