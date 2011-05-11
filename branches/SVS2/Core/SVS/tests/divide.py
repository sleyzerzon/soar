from __future__ import print_function
import sys, os, random

ntrain = int(sys.argv[1])
ntest = int(sys.argv[2])
lines = [l.strip() for l in sys.stdin.readlines()]
random.shuffle(lines)
print('\n'.join(lines[:ntrain]))
print('$')
print('\n'.join(lines[ntrain:ntrain+ntest]))
