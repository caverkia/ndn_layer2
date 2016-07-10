#! /usr/bin/python

import sys

def modify_node_id(infile):
	f = open(infile, 'r')
	w = open(infile + '-mapped', 'w')
	t = set()
	for line in f.readlines():
		l = line.strip().split()
		n1 = int(l[0])
		n2 = int(l[1])
		t.add(n1)
		t.add(n2)
	size = len(t)

	ofset = []
	for i in t:
		if i >= size:
			ofset.append(int(i))

	print ofset
	d = dict()
	for i in xrange(size):
		if int(i) not in t:
			curalt = ofset.pop()
			d[curalt] = i
			
	for (k,v) in d.items():
		print "%d --" %k, v
	
	f.close()
	f = open(infile, 'r')
	for line in f.readlines():
		l = line.strip()
		m = l.split()
		for x in m:
			x = int(x)
			if x in d.keys():
				w.write(str(d[x]) + '\t')
			else:
				w.write(str(x) + '\t')
		w.write('\n')
	f.close()
	w.close()
		


if __name__ == '__main__':
	infile = sys.argv[1]
	modify_node_id(infile)
	
