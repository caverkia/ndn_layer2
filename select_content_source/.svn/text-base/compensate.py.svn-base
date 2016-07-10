#! /usr/bin/python

import sys
def compensate(infile, tot_cont):
	f = open(infile, 'r')
	w = open(infile + '-with0', 'w')

	tot_cont = int(tot_cont)
	new_set = set()
	ll = set()
	for lines in f.readlines():
		l = lines.strip().split()
		del l[0]
		print l
		for x in l:
			ll.add(int(x))
	w.write('0' + '\t')
	for d in xrange(tot_cont):
		if d not in ll:
			w.write(str(d) + '\t')
	w.write('\n')
	f.close()
	f = open(infile, 'r')
	for line in f.readlines():
		l = line.strip()
		w.write(l + '\n')

if __name__ == '__main__':
	infile = sys.argv[1]
	tot_cont = sys.argv[2]
	compensate(infile, tot_cont)
	
		
