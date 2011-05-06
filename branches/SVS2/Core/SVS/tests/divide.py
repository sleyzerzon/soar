from __future__ import print_function
import sys, os, random

ntrain = int(sys.argv[1])
ntest = int(sys.argv[2])
lines = random.sample([l.strip() for l in sys.stdin.readlines()], ntrain + ntest)
print('\n'.join(lines[:ntrain]))
print('$')
print('\n'.join(lines[ntrain:]))
