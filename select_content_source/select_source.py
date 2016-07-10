#! /usr/bin/python

import sys
import random

def select_source(infile, cont_num, inside_source_num):
	f = open(infile,'r')
	num = int(cont_num)
	in_num = int(inside_source_num)
	source = []
	for lines in f.readlines():
		line = lines.strip()
		source.append(int(line))
	f.close()
	size = len(source)

	all_cont = []
	for k in xrange(num):
		all_cont.append(int(k))
	selected_cont = []
	for k in xrange(in_num):
		tail = len(all_cont) - 1
		index = random.randint(0, tail)
		selected_cont.append(all_cont[index])
		del all_cont[index]
	d = dict()
	for k in source:
		tmp = []
		index = int(k)
		d[index] = tmp
	for k in selected_cont:
		pos = random.randint(0, size - 1)
		index = int(source[pos])
		d[index].append(k)
#	print d

	w = open('source_cont_mapping', 'w')
	for (k, v) in d.iteritems():
		w.write(str(k) + '\t')
		for m in v:
			w.write(str(m) + '\t')
		w.write('\n')
	w.close()
		

if __name__ == '__main__':
	infile = sys.argv[1]
	cont_num = sys.argv[2]
	inside_source_num = sys.argv[3]
	select_source(infile, cont_num, inside_source_num)
	
