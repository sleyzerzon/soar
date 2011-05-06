from __future__ import print_function
import sys, os, math, random
from subprocess import *
sys.path.append('.')
from lwr import LWR
import numpy as np

def dist(x1, x2):
	d = x1 - x2
	return math.sqrt(np.dot(d, d))

def toarray(line):
	return np.array([float(v) for v in line.split()])
	
def regress(nnbrs, text):
	p = Popen(['./lwrtest', str(nnbrs)], stdin=PIPE, stdout=PIPE)
	out = p.communicate(text)[0]
	return toarray(out)

m = None
nnbrs = int(sys.argv[1])
diffs = []
training = True
for line in sys.stdin:
	if line.startswith('$'):
		training = False
		continue
		
	x, y = map(toarray, line.split(';'))
	if training:
		if not m:
			m = LWR(len(x), len(y), nnbrs)
		m.update(x, y)
	else:
		p = m.predict(x)
		diffs.append(dist(p, y))
	
diffa = np.array(diffs)
print('Mean: ', np.mean(diffa))
print('STD:  ', np.std(diffa))
