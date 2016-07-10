#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <assert.h>



using namespace std;
typedef uint32_t uint;

typedef struct _edge{
	uint nid; //the other vertex's ID
	uint type; //wh: 0 for stp link, 1 for non-stp link
}edge;

typedef struct _node{
	uint id;
	uint type;   //wh: 0 for E-switch, 1 for N-switch
	unordered_map<uint, uint> edge; //first: for the connected vertex's ID,k, second for the link type: 1 for stp and 0 for non-stp
	bool operator ==(const struct _node &T)
	{
		return (id == T.id);
	}
}node;

void compute_leaf_node(string &topofilename, vector<node *> &lan)
{
	ifstream infile;
	infile.open(topofilename.c_str());
	if(!infile)
	{
		perror("topo file error!");
		exit(0);
	}

	string leafFileName = topofilename + "-leafnode";
	ofstream outfile;
	outfile.open(leafFileName.c_str());
	if(!outfile)
	{
		perror("leaf file error!");
		exit(0);
	}
	uint node_num = lan.size();
	for(auto k : lan)
	{
		uint link_num = 0;
		for(auto j = 0; j < node_num; j ++)
		{
			uint t = 0;
			infile >> t;
			if(t == 1)
			{
				link_num ++;
				k->edge.insert(make_pair<uint, uint>(j, 0));
			}
		}
		if(link_num == 1)
			outfile << k->id << endl;
	}

	outfile.clear();
	outfile.close();
	infile.clear();
	infile.close();
}
	
		

unordered_map<uint, node *> & build_stp(string &topofilename)
{
	unordered_map<uint, node *> lan;
	lan.clear();
	ifstream infile(topofilename.c_str());
	if(!infile)
	{
		perror("read topo file name error!\n");
		exit(0);
	}
	uint n1, n2;
	while(infile >> n1 >> n2)
	{
		if(lan.find(n1) == lan.end())
		{
			node * t = (node *)malloc(sizeof(node) * sizeof(char));
			t->id = n1;
			t->type = 0;
			lan.insert(std::make_pair<uint, node*>((uint)n1, (struct _node *)t));
			cout << n1 << '\t' << t->id << endl;
		}
		if(lan.find(n2) == lan.end())
		{
			node * t = (node *)malloc(sizeof(node) * sizeof(char));
			t->id = n2;
			t->type = 0;
			lan.insert(std::make_pair<uint, node*>((uint)n2, (struct _node *)t));
			cout << n2 << '\t' << t->id << endl;
		}
	}
	infile.clear();
	infile.close();
	return lan;
}


