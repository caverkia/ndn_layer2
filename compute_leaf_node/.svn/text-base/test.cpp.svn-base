#include <stdio.h>
#include <string>
#include <funcs.h>
#include <vector>
#include <cstring>

using namespace std;

int main(int argc, char *argv[])
{
	string node_num_str = argv[1];
	uint node_num = stoi(node_num_str);
	string topofilename = argv[2];

	vector<node *> lan;
	lan.clear();
	for(auto i = 0; i < node_num; i ++)
	{
		node * t = new node;
		t->id = i;
		t->type = 0;  // all nodes are initialized as E-switch
		t->edge.clear();
		lan.push_back(t);
	}
	
	compute_leaf_node(topofilename, lan);

	for(auto d: lan)
	{
		d->edge.clear();
		free(d);
	}
	lan.clear();
	return 1;
}

