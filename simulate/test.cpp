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
	string stp_topofilename = argv[3];
	string host_id_filename = argv[4];
	string source_filename = argv[5];

	vector<node *> lan;
	lan.clear();
	for(auto i = 0; i < node_num; i ++)
	{
		node * t = new node;
		t->id = i;
		t->type = 0;  // all nodes are initialized as E-switch
		t->is_outside_content_source = false;
		t->edge.clear();
		t->SList.clear();
		t->SPendingList.clear();
		t->CList.clear();
		t->pit.clear();
		t->eth_fib.clear();
		t->req_input_queue.clear();
		t->data_input_queue.clear();
		t->is_host = false;
		t->req_pendingList.clear();
		t->pkt_no = 0;
		lan.push_back(t);
	}

	add_links(topofilename, stp_topofilename, host_id_filename, source_filename, lan);
	
	lan[0]->type = 1;
	lan[2]->type = 1;
	lan[4]->type = 1;
	
	run_simulate(lan);

	for(auto d: lan)
	{
		d->edge.clear();
		delete d;
	}
	lan.clear();
	return 1;
}

