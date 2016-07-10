#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
#include <unordered_set>

//#define CSIZE 3
#define NO_DUPLICATE_ALLOWED 0  // if allow an N-switch to forward a packing passing blocked link to STP tree again
//#define CONT_DL_DELAY 20   // the download delay to fetch a content from outside content source
//#define DATA_PENDING_TIMER 30

using namespace std;



// global varibles
uint CONT_DL_DELAY = 11;
uint DATA_PENDING_TIMEOUT = 8;
//uint PIT_PENDING_TIMEOUT = 30;
uint SIMUL_END_TIME = 70;
uint SIMUL_END_PKT_NUM = 150;
uint REQ_BYTES = 100;
uint DATA_BYTES = 4096;
uint CSIZE = 3;

uint served_data_num = 0;
uint dropped_req_num = 0;
uint dropped_data_num = 0;
uint data_dl_delay = 0;
uint total_req_num = 0;
uint total_data_num = 0;
uint cs_hits = 0;
uint cs_reqs = 0;
uint source_served_req_num = 0;
uint resend_req_num = 0;
//uint dropped_req_hops = 0;
//uint dropped_data_hops = 0;


typedef uint32_t uint;
/*
typedef struct _edge{
	uint nid; //the other vertex's ID
	uint type; //wh: 0 for stp link, 1 for non-stp link
}edge;
*/
/*
typedef struct _cache{
	vector<uint> CList;
	vector<uint> CPendingList;
}cache;
*/
typedef struct _pend_req{
	uint cont;
	uint wait_time;
}pend_req;


typedef struct _pend_data{
	uint cont;
	uint dstMac;
	/*
	bool operator < (const struct _pend_data &T) const
	{
		if(dstMac != T.dstMac)
			return (dstMac < T.dstMac);
		return (cont < T.cont);
	}
*/
	bool operator ==(const struct _pend_data &T) const
	{
		return (cont == T.cont && dstMac == T.dstMac);
	}
}pend_data;




typedef struct _pit_port{
	uint port;
	uint dstMac;
	/*
	bool operator < (const struct _pit_port &T) const
	{
		if(dstMac != T.dstMac)
			return (dstMac < T.dstMac);
		return (port < T.port);
	}
*/
	bool operator ==(const struct _pit_port &T) const
	{
		if(port == T.port && dstMac == T.dstMac)
			return true;
		else
			return false;
	}
}pit_port;



struct hash_pit_port{
	float operator() (const pit_port &T) const {
		return T.port * 2.3 + T.dstMac * 5.4;
	}
};

struct hash_pend_data{
	float operator() (const pend_data &T) const{
		return T.dstMac * 3.7 + T.cont * 10.9;
	}
};


typedef struct _pit_entry{
	uint cont;
//	uint64_t nonce;
	unordered_set<uint64_t> nonces;
	unordered_set<pit_port, hash_pit_port> inportList;

	uint timer;
	bool pass_blocked_link;

	bool operator ==(const struct _pit_entry &T)
	{
		return (cont == T.cont);
	}
}pit_entry;


typedef struct _fib_entry{
	uint cont;
	uint nextHop;
	uint dstMac;
}fib_entry;


typedef struct _interest{
	uint cont;
	uint lastHop;
	uint srcMac;
	uint dstMac;
	uint input_time;
	bool MUST_FWD;  // if == true, the node MUST forward this interest no matter whether it is recorded in PIT
//	uint hops;
	bool pass_blocked_link;
	uint64_t nonce;
	bool operator ==(const struct _interest &T)
	{
		return (cont == T.cont);
	}
}interest;


typedef struct _data{
	uint cont;
	uint lastHop;
	uint srcMac;
	uint dstMac;
	uint input_time;
//	uint hops;
	bool been_cached;  // indicate if the data has been cached in the previous path so that the rest router will not cache it again
	bool operator ==(const struct _interest &T)
	{
		return (cont == T.cont);
	}
}data;
/*
void update_cs(node * n)
{
	for(auto k : n->CPendingList)
	{
		auto it = find(n->CList.begin(), n->CList.end(), *k);
		if(it == n->CList.end())
		{
			n->CList.insert(n->CList.begin(), *k);
		//	n->CPendingList.erase(it);
			if(n->CList.size() > CSIZE)
				n->CList.pop_back();
		}
		else
		{
			n->CList.erase(it);
			n->CList.insert(n->CList.begin(), *k);
		}
	}
	n->CPendingList.clear();
}

void update_pit(node * n)
{
	for(auto k : n->pit_deleteList)
	{
		pit_entry tmp;
		tmp.cont = k;
		auto it = n->pit.find(tmp);
		if(it != n->pit.end())
		{
			it->second.inportList.clear();
			n->pit.erase(it);
		}
	}
	n->pit_deleteList.clear();
	for(auto k : n->pit_addList)
	{
		uint cid = k.cont;
		auto it = 
		n->pit[cid] = k;
		k.inportList.clear();
	}
	n->pit_addList.clear();

}
*/


typedef struct _req_input_entry{
	uint cont;
	uint wait_time;
	bool MUST_FWD;
}req_input_entry;


typedef struct _node{
	uint id;
	uint type;   //wh: 0 for E-switch, 1 for N-switch
//	bool is_active;
	bool is_outside_content_source;
	unordered_map<uint, uint> edge; //first: for the connected vertex's ID,k, second for the link type: 1 for stp and 0 for non-stp
//	vector<uint> req_input;
	vector<req_input_entry> req_input;
	vector<uint> SList;  // the source list for an access node
	unordered_map<uint, unordered_set<pend_data, hash_pend_data>> SPendingList;  // the source pending list for the gateway node
	vector<uint> CList;   // the cache list for an N-switch
//	vector<uint> CPendingList;
	unordered_map<uint, pit_entry> pit;
//	vector<pit_entry> pit_addList;
//	vector<uint> pit_deleteList;
	// content-nextHop/dstMac mapping
	unordered_map<uint, fib_entry> fib;
//	unordered_map<uint, fib_entry> fib_addList;
	// dstMac-nextHop mapping
	unordered_map<uint, uint> eth_fib;
//	unordered_map<uint, uint> eth_fib_addList;
	
	vector<interest> req_input_queue;
	vector<data> data_input_queue;
	// indicate if the node is an access node that can receive interests from users
	bool is_host;
	vector<pend_req> req_pendingList;

	uint pkt_no;
	bool operator ==(const struct _node &T)
	{
		return (id == T.id);
	}
}node;



/*return: 0 for PIT miss, should forward based on FIB
1 for PIT hit, should drop the interest
note: for a duplicate interest coming from an Ethernet while a same interest coming from blocked link has been recorded in PIT now, in this case, the new coming duplicate interest should still be forwarded
*/
int pit_lookup_interest(node *d, interest & req, uint lastHop, bool & is_req_drop)
{
	pit_entry tmp;
	tmp.cont = req.cont;
	tmp.pass_blocked_link = req.pass_blocked_link;
	tmp.timer = 0;


	pit_port tmp_pitport;
	tmp_pitport.port = lastHop;
	tmp_pitport.dstMac = req.srcMac;
	auto it = d->pit.find(tmp.cont);
	if(it != d->pit.end())
	{
		if(it->second.nonces.find(req.nonce) != it->second.nonces.end())
//		if(it->second.nonce == req.nonce)
		{
		/*
			if(NO_DUPLICATE_ALLOWED && it->second.pass_blocked_link == true && req.pass_blocked_link == false)
			{
				it->second.pass_blocked_link = false;
				it->second.inportList.insert(tmp_pitport);
				return 0;
			}
			*/
			if(req.MUST_FWD == false)
			{
				is_req_drop = true;
				return 1;
			}
			else
				return 0;
		}
		else
		{
			it->second.nonces.insert(req.nonce);
			it->second.inportList.insert(tmp_pitport);
			if(req.MUST_FWD == false)
				return 1;
			else
				return 0;
		}
	}
	tmp.nonces.clear();
//	tmp.nonce = req.nonce;
	tmp.nonces.insert(req.nonce);
	tmp.inportList.clear();
	tmp.inportList.insert(tmp_pitport);
	d->pit[tmp.cont] = tmp;
	return 0;
}
	

// return the data hit pit outports and delete the pit entry
unordered_set<pit_port, hash_pit_port> pit_lookup_data(node *d, data & chunk, uint lastHop)
{
	unordered_set<pit_port, hash_pit_port> outports;
	outports.clear();

	pit_entry tmp;
	tmp.cont = chunk.cont;

	auto it = d->pit.find(tmp.cont);
	if(it != d->pit.end())
	{
		outports = it->second.inportList;
		it->second.inportList.clear();
		it->second.nonces.clear();
		d->pit.erase(it);
		return outports;
	}
	return outports;
}

/*
void update_fib(node *n)
{
	n->fib.insert(n->fib_addList.begin(), n->fib_addList.end());
	n->addList.clear();
}


void update_eth_fib(node *n)
{
	assert(n->type == 0);
	n->eth_fib.insert(n->eth_fib_addList.begin(), n->eth_fib_addList.end());
	n->eth_fib_addList.clear();
}
*/


int eth_fib_lookup_interest(node * d, interest & req)
{
	if(req.dstMac == 9999) // broadcast Mac addr
		return -1;
	auto it = d->eth_fib.find(req.dstMac);
	if(it != d->eth_fib.end())
		return it->second;
	else
		return -1;
}

int eth_fib_lookup_data(node * d, data & chunk)
{
	if(chunk.dstMac == 9999) // broadcast Mac addr
		return -1;
	auto it = d->eth_fib.find(chunk.dstMac);
	if(it != d->eth_fib.end())
		return it->second;
	else
		return -1;
}


uint eth_broadcast_interest_on_stp(vector<node *> &lan, node * d, interest req, uint lastHop)
{	
	uint sent_num = 0;
	for(auto k : d->edge)
	{
		if(k.second == 1 && k.first != lastHop)
		{	
			sent_num ++;
			lan[k.first]->req_input_queue.push_back(req);
		}
	}
	return sent_num;
}
	
uint eth_broadcast_data_on_stp(vector<node *> & lan, node * d, data  chunk, uint lastHop)
{
	uint sent_num = 0;
	for(auto k : d->edge)
	{
		if(k.second == 1 && k.first != lastHop)
		{
			sent_num ++;
			lan[k.first]->data_input_queue.push_back(chunk);
		}
	}
	return sent_num;
}


bool cs_lookup(node * d, interest &req, data &chunk)
{
	auto it = find(d->CList.begin(), d->CList.end(), req.cont);
	if(it == d->CList.end())
		return false;
	else
	{
		d->CList.erase(it);
		d->CList.insert(d->CList.begin(), req.cont);
		chunk.cont = req.cont;
		chunk.lastHop = d->id;
		chunk.srcMac = d->id;
		chunk.dstMac = req.srcMac;
		chunk.been_cached = true;
		return true;
	}
}

bool source_lookup(node *d, interest & req, data &chunk)
{
	auto it = find(d->SList.begin(), d->SList.end(), req.cont);
	if(it == d->SList.end())
		return false;
	else
	{
		chunk.cont = req.cont;
		chunk.lastHop = d->id;
		chunk.srcMac = d->id;
		chunk.dstMac = req.srcMac;
		chunk.been_cached = false;
		return true;
	}
}
	


int fib_lookup(node *d, interest &req)
{
	auto it = d->fib.find(req.cont);
	if(it != d->fib.end())
	{
		req.dstMac = it->second.dstMac;
		return it->second.nextHop;
	}
	return -1;
}
	

uint ndn_broadcast_interest(vector<node*> &lan, node * d, interest req, uint lastHop)
{
//	req->dstMac = 9999;
	req.srcMac = d->id;
	req.lastHop = d->id;
	uint sent_num = 0;
	if(!NO_DUPLICATE_ALLOWED)
	{
		for(auto k : d->edge)
		{
			if(k.second == 1 && k.first != lastHop)
			{
				lan[k.first]->req_input_queue.push_back(req);
				sent_num ++;
				continue;
			}
			if(lan[k.first]->type == 1 && k.first != lastHop && k.second == 0)
			{
				interest tmp = req;
				assert(tmp.dstMac == req.dstMac);
				tmp.pass_blocked_link = true;
				lan[k.first]->req_input_queue.push_back(tmp);
				sent_num ++;
			}
		}
	}
	else
	{
		for(auto k : d->edge)
		{
			// if the connecting sw is N-switch
			if(lan[k.first]->type == 1 && k.first != lastHop)
			{
				if(k.second == 1)
				{
					sent_num ++;
					lan[k.first]->req_input_queue.push_back(req);
				}
				else
				{
					sent_num ++;

					interest tmp = req;
					assert(tmp.dstMac == req.dstMac);
					tmp.pass_blocked_link = true;
					lan[k.first]->req_input_queue.push_back(tmp);
				}
			}
			else if(lan[k.first]->type == 0 && k.first != lastHop && k.second == 1 && req.pass_blocked_link == false)
			{
				sent_num ++;
				lan[k.first]->req_input_queue.push_back(req);
			}
		}
	}
	return sent_num;
}

bool add_data_to_cs(node * d, data &chunk)
{
	if(chunk.been_cached == true)
		return false;

	auto it = find(d->CList.begin(), d->CList.end(), chunk.cont);
	if(it == d->CList.end())
	{
		d->CList.insert(d->CList.begin(), chunk.cont);
		chunk.been_cached = true;
		if(d->CList.size() > CSIZE)
			d->CList.pop_back();
		return true;
	}
	else
	{
	//	chunk->lastHop = d->id;
	//	chunk->srcMac = d->id;
		chunk.been_cached = true;
		return false;
	}
}


uint64_t compute_nonce(uint nid, uint pkt_no)
{
	uint64_t nonce = nid;
	uint64_t no64 = pkt_no;
	nonce = (nonce << 32) | no64;
	return nonce;
}


void fib_add_entry(node *d, data &chunk)
{
	fib_entry tmp;
	tmp.cont = chunk.cont;
	tmp.nextHop = chunk.lastHop;
	tmp.dstMac = chunk.srcMac;
	d->fib[chunk.cont] = tmp;
}





void run_simulate(vector<node *> &lan)
{
	uint time = 0;
	while(1)
	{
		// used to distinguish the input queue
		uint input_time = time % 2;
		uint last_input_time = (time + 1) % 2;
		bool no_pending_req = true;
//		for(auto d: lan)
//		{
//			if(d->type == 1)
//			{
//				/*cache CS entry made in last slot*/
//				update_cs(d);
//				/*put PIT entry made in last slot*/
//				update_pit(d);
//				/*put FIB entry made in last slot*/
//				update_fib(d);
//			}
//			else
//				update_eth_fib(d);
//		}
		for(auto d: lan)
		{
			// for the content source connecting the gateway
			if(d->is_outside_content_source == true && d->is_host == true)
			{
				// check if there is arrived data in this time slot
				auto iter = d->SPendingList.find(time);
				if(iter != d->SPendingList.end())
				{
					data tmp;
					tmp.lastHop = d->id;
					tmp.srcMac = d->id;
					tmp.input_time = input_time;
					tmp.been_cached = false;
					for(auto it : iter->second)
					{
						source_served_req_num ++;
						total_data_num ++;

						tmp.cont = it.cont;
						tmp.dstMac = it.dstMac;
					//	assert(tmp.dstMac == 0);
						// the outside content server only connects to lan gateway node-0
						lan[0]->data_input_queue.push_back(tmp);
					}
					iter->second.clear();
					d->SPendingList.erase(iter);
				}

				// when get a request going out of the lan
				auto itr = d->req_input_queue.begin();
				pend_data tmp_pd;
				unordered_set<pend_data, hash_pend_data> tmp_pend_set;
				tmp_pend_set.clear();
				while(itr != d->req_input_queue.end())
				{
					if(itr->input_time != last_input_time)
					{
						if(itr != d->req_input_queue.begin())
							itr = d->req_input_queue.erase(d->req_input_queue.begin(), itr);
						break;
					}

					interest tmp_req;
					tmp_req.cont = itr->cont;
					data tmp_chunk;
					tmp_chunk.cont = itr->cont;
					if(find(d->SList.begin(), d->SList.end(), itr->cont) == d->SList.end())
					{
						dropped_req_num ++;
						itr ++;
						continue;
					}

					tmp_pd.cont = itr->cont;
					tmp_pd.dstMac = itr->srcMac;
					tmp_pend_set.insert(tmp_pd);
					itr ++;
				}
				if(itr == d->req_input_queue.end())
					d->req_input_queue.clear();
				if(tmp_pend_set.empty() == false)
				{
					uint arr_time = time + CONT_DL_DELAY;
					d->SPendingList[arr_time] = tmp_pend_set;
					tmp_pend_set.clear();
				}
			} // end of the outside content source


			// for an access node inside the LAN
			if(d->is_host == true && d->is_outside_content_source == false)
			{	
				// when get a data chunk, check if it is the previously requested content
				auto dit = d->data_input_queue.begin();
				while(dit != d->data_input_queue.end())
				{
					if(dit->input_time != last_input_time)
					{
						if(dit != d->data_input_queue.begin())
							dit = d->data_input_queue.erase(d->data_input_queue.begin(), dit);
						break;
					}
					// check layer-2, if the dst mac addr is current host
					if(dit->dstMac != d->id && dit->dstMac != 9999)
					{
						dropped_data_num ++;

						dit ++;
						continue;
					}

					data tmp;
					tmp.cont = dit->cont;
					tmp.input_time = input_time;
					tmp.lastHop = dit->lastHop;
					tmp.srcMac = dit->srcMac;
				//	uint lastHop = dit->lastHop;
					// update host's FIB entry
					interest req;
					req.cont = tmp.cont;
					if(fib_lookup(d, req) == -1)
						fib_add_entry(d, tmp);
					auto iter = d->req_pendingList.begin();
					// check if the coming data can serve any pending requests sent by this host
					
					bool pending_hit = false;
					while(iter != d->req_pendingList.end())
					{
						if(iter->cont == tmp.cont)
						{
							pending_hit = true;
							// do some stats
							served_data_num ++;
							data_dl_delay += iter->wait_time;
							iter = d->req_pendingList.erase(iter);
							continue;
						}
						iter ++;
					}
					if(pending_hit == false)
						dropped_data_num ++;
					dit ++;
				}
				if(dit == d->data_input_queue.end())
					d->data_input_queue.clear();
				
				// when get a interest, the access node can serve as the content source
				auto it = d->req_input_queue.begin();
				while(it != d->req_input_queue.end())
				{
					if(it->input_time != last_input_time)
					{
						if(it != d->req_input_queue.begin())
							it = d->req_input_queue.erase(d->req_input_queue.begin(), it);
						break;
					}
					interest tmp;
					tmp.cont = it->cont;
					tmp.lastHop = it->lastHop;
					tmp.srcMac = it->srcMac;
					tmp.dstMac = it->dstMac;
					tmp.MUST_FWD = it->MUST_FWD;

					uint lastHop = it->lastHop;
					data chunk;
					chunk.input_time = input_time;
					bool source_hit = source_lookup(d, tmp, chunk);
					if(source_hit == true)
					{
						source_served_req_num ++;
						total_data_num ++;

						lan[lastHop]->data_input_queue.push_back(chunk);
					}
					else
						dropped_req_num ++;
					it ++;
				}
				if(it == d->req_input_queue.end())
					d->req_input_queue.clear();

				// to send out an interest packet
				uint cont;
				uint delay;
				if(d->req_input.empty() != true)
				{
					cont = d->req_input[0].cont;
					delay = d->req_input[0].wait_time;
					// check the coming interest is not in the node's source list
					assert(find(d->SList.begin(), d->SList.end(), cont) == d->SList.end());

					interest req;
					req.cont = cont;
					req.lastHop = d->id;
					req.srcMac = d->id;
					req.input_time = input_time;
					req.pass_blocked_link = false;
					req.nonce = compute_nonce(d->id, d->pkt_no);
					req.MUST_FWD = d->req_input[0].MUST_FWD;

					d->req_input.erase(d->req_input.begin());
					int nextHop = fib_lookup(d, req);

					total_req_num ++;
					if(nextHop != -1)
					{
						if(d->edge[nextHop] == 0)
							req.pass_blocked_link = true;
						lan[nextHop]->req_input_queue.push_back(req);
					}
					else
					{
						req.dstMac = 9999; // define 9999 as the broadcast Mac addr
						total_req_num += (ndn_broadcast_interest(lan, d, req, req.lastHop) - 1);
					}
					//if req.MUST_FWD == true, it means the current outgoing interest is a resent one, do not add it in pending list
					if(req.MUST_FWD == false)
					{
						pend_req tmp_pr;
						tmp_pr.cont = cont;
						tmp_pr.wait_time = delay;
						assert(delay == 0);
						d->req_pendingList.push_back(tmp_pr);
						d->pkt_no ++;
					}
				}
				else
				{
					d->req_input.clear();
				}

				if(d->req_pendingList.empty() == false)
					no_pending_req = false;
				
				// update the pending list, increase the waiting time
				auto itk = d->req_pendingList.begin();
				while(itk != d->req_pendingList.end())
				{
					// if the pending request time out, resend it
				//	if(itk->wait_time > DATA_PENDING_TIMEOUT)
					if(itk->wait_time % DATA_PENDING_TIMEOUT == (DATA_PENDING_TIMEOUT - 1) )
					{
						req_input_entry tmp_rie;
						tmp_rie.cont = itk->cont;
						tmp_rie.wait_time = itk->wait_time + 1;
						tmp_rie.MUST_FWD = true;
						d->req_input.insert(d->req_input.begin(), tmp_rie);
						// do not delete the exising pending entry if the data does not arrive!
				//		itk = d->req_pendingList.erase(itk);
						resend_req_num ++;
				//		continue;
					}
					itk->wait_time ++;
					itk ++;
				}
			} // if the node is an access node
				

			// for an E-switch
			if(d->type == 0 && d->is_host == false)
			{
				// for data packet
				auto dit = d->data_input_queue.begin();
				while(dit != d->data_input_queue.end())
				{
					if(dit->input_time != last_input_time)
					{
						if(dit != d->data_input_queue.begin())
							dit = d->data_input_queue.erase(d->data_input_queue.begin(), dit);
						break;
					}
					data tmp;
					tmp.cont = dit->cont;
					tmp.lastHop = d->id;
					tmp.srcMac = dit->srcMac;
					tmp.dstMac = dit->dstMac;
					tmp.input_time = input_time;
					tmp.been_cached = dit->been_cached;

					uint lastHop = dit->lastHop;
					//if the src mac addr is not in the E-switch's fib, do mac learning
					if(d->eth_fib.find(tmp.srcMac) == d->eth_fib.end())
					{	
						assert(tmp.srcMac != 9999);
						d->eth_fib[tmp.srcMac] = lastHop;
					}
					int nextHop = eth_fib_lookup_data(d, tmp);
					
					if(nextHop != -1)  // if fib hit
						lan[nextHop]->data_input_queue.push_back(tmp);
					else //broadcast among E-switch's spanning tree
					{
						uint copy_num = eth_broadcast_data_on_stp(lan, d, tmp, lastHop) - 1;
						total_data_num += copy_num;
					}
					dit ++;
				}
				if(dit == d->data_input_queue.end())
					d->data_input_queue.clear();
				// for interest packet
				auto it = d->req_input_queue.begin();
				while(it != d->req_input_queue.end())
				{
					if(it->input_time != last_input_time)
					{
						if(it != d->req_input_queue.begin())
							it = d->req_input_queue.erase(d->req_input_queue.begin(), it);
						break;
					}
					interest tmp;
					tmp.cont = it->cont;
					tmp.lastHop = d->id;
					tmp.srcMac = it->srcMac;
					tmp.dstMac = it->dstMac;
					tmp.input_time = input_time;
					tmp.pass_blocked_link = it->pass_blocked_link;
					tmp.nonce = it->nonce;
					tmp.MUST_FWD = it->MUST_FWD;

					uint lastHop = it->lastHop;
					//if the src mac addr is not in the E-switch's fib, do mac learning
					if(d->eth_fib.find(tmp.srcMac) == d->eth_fib.end())
					{
						assert(tmp.srcMac != 9999);
						d->eth_fib[tmp.srcMac] = lastHop;
					}
					int nextHop = eth_fib_lookup_interest(d, tmp);
					if(nextHop != -1)  // if fib hit
						lan[nextHop]->req_input_queue.push_back(tmp);
					else //broadcast among E-switch's spanning tree
					{
						uint copy_num = eth_broadcast_interest_on_stp(lan, d, tmp, lastHop) - 1;
						total_req_num += copy_num;
					}
					it ++;
				}
				if(it == d->req_input_queue.end())
					d->req_input_queue.clear();
			}  // end of an E-switch
			// for an N-switch
			if(d->type == 1 && d->is_host == false)
			{
				// for data packet
				auto dit = d->data_input_queue.begin();
				while(dit != d->data_input_queue.end())
				{
					if(dit->input_time != last_input_time)
					{
						if(dit != d->data_input_queue.begin())
							dit = d->data_input_queue.erase(d->data_input_queue.begin(), dit);
						break;
					}
					
					data tmp;
					tmp.cont = dit->cont;
					tmp.lastHop = d->id;
					tmp.srcMac = dit->srcMac;
					tmp.dstMac = dit->dstMac;
					tmp.input_time = input_time;
					tmp.been_cached = dit->been_cached;

					uint lastHop = dit->lastHop;
					unordered_set<pit_port, hash_pit_port> outports = pit_lookup_data(d, tmp, lastHop);
					if(outports.empty() == true)
					{
						dropped_data_num ++;
						dit ++;
						continue;
					}
					add_data_to_cs(d, tmp);
					// update the fib entry according to the coming data chunk
					interest tmp_req;
					tmp_req.cont = tmp.cont;
					if(fib_lookup(d, tmp_req) == -1 )
					{
						data tmp_chunk;
						tmp_chunk.cont = tmp.cont;
						tmp_chunk.lastHop = lastHop;
						tmp_chunk.srcMac = dit->srcMac;
						fib_add_entry(d, tmp_chunk);
					}
					// forward the data chunk to outports
					tmp.srcMac = d->id;
					for(auto iter = outports.begin(); iter != outports.end(); iter ++)
					{
						tmp.dstMac = iter->dstMac;
						lan[iter->port]->data_input_queue.push_back(tmp);
					}
					total_data_num += (outports.size() - 1);
					dit ++;
				}
				if(dit == d->data_input_queue.end())
					d->data_input_queue.clear();
				
				// for interest packet
				auto it = d->req_input_queue.begin();
				while(it != d->req_input_queue.end())
				{
					if(it->input_time != last_input_time)
					{
						if(it != d->req_input_queue.begin())
							it = d->req_input_queue.erase(d->req_input_queue.begin(), it);
						break;
					}
					interest tmp;
					tmp.cont = it->cont;
					tmp.lastHop = d->id;
					tmp.srcMac = it->srcMac;
					tmp.dstMac = it->dstMac;
					tmp.input_time = input_time;
					tmp.pass_blocked_link = it->pass_blocked_link;
					tmp.nonce = it->nonce;
					tmp.MUST_FWD = it->MUST_FWD;

					uint lastHop = it->lastHop;
					data chunk;
					chunk.input_time = input_time;
					bool cs_hit = cs_lookup(d, tmp, chunk);
					cs_reqs ++;
					if(cs_hit == true)
					{
						cs_hits ++;
						lan[lastHop]->data_input_queue.push_back(chunk);
						it ++;
						continue;
					}
					bool is_req_drop = false;
					int pit_hit = pit_lookup_interest(d, tmp, lastHop, is_req_drop);
					if(pit_hit == 1)
					{
						// check if the request is dropped due to duplicate nonce
						if(is_req_drop == true)
							dropped_req_num ++;
						it ++;
						continue;
					}
					tmp.srcMac = d->id;
					int nextHop = fib_lookup(d, tmp);

					total_req_num ++;
					if(nextHop != -1)
					{
						if(d->edge[nextHop] == 0)
							tmp.pass_blocked_link = true;
						lan[nextHop]->req_input_queue.push_back(tmp);
					}
					else
					{
						uint copy_num = ndn_broadcast_interest(lan, d, tmp, lastHop) - 1;
						total_req_num += copy_num;
					}
					it ++;
				}
				if(it == d->req_input_queue.end())
					d->req_input_queue.clear();

				// increase pit entry's timer
			/*
				auto itp = d->pit.begin();
				while(itp != d->pit.end())
				{
					if(itp->second.timer >= PIT_PENDING_TIMEOUT)
					{
						itp = d->pit.erase(itp);
						continue;
					}
					itp->second.timer ++;
					itp ++;
				}
				*/
			} // end of an N-switch
		} // end of checking every node in the lan
		if(no_pending_req == true)
		{
			cout << "Time = " << time << endl;
			break;
		}
		time ++;
	//	if(time == SIMUL_END_TIME)
	//		break;
	} // while iterate in every time slot

	cout << "cs hit ratio = " << cs_hits*1.0/cs_reqs << endl;
	cout << "# of served data =" << served_data_num << endl;
	cout << "# of dropped reqs = " << dropped_req_num << endl;
	cout << "# of dropped data = " << dropped_data_num << endl;
	cout << "# of total reqs = " << total_req_num << endl;
	cout << "# of total data chunks = " << total_data_num << endl;
	cout << "# of source-served reqs = " << source_served_req_num << endl;
	cout << "# of resent reqs = " << resend_req_num << endl;
	cout << "data download delay = " << data_dl_delay << endl;
	
	cout << "avg dl delay = " << data_dl_delay*1.0/served_data_num << endl;
	
}
				
					


// read connecting information from topo file
// topofilename: the whole network topology
// stp_topofilename: the stp connecting topo
void add_links(string &topofilename, 
					string &stp_topofilename, 
					string &host_node_id_filename,
					string &cont_source_filename,
					vector<node *> &lan)
{
	ifstream infile;
	infile.open(stp_topofilename.c_str());
	if(!infile)
	{
		perror("stp topo file error!");
		exit(0);
	}
	uint node_num = lan.size();
	for(auto k : lan)
	{
		for(auto j = 0; j < node_num; j ++)
		{
			uint t = 0;
			infile >> t;
			if(t == 1)
				k->edge.insert(make_pair<uint, uint>(j, 1));
		}
	}
	infile.clear();
	infile.close();

	infile.open(topofilename.c_str());
	if(!infile)
	{
		perror("the whole topo file error!");
		exit(0);
	}

	uint non_stp_num = 0;
	for(auto k : lan)
	{
		for(auto j = 0; j < node_num; j ++)
		{
			uint t = 0;
			infile >> t;
			auto it = k->edge.find(j);
			// if we find a connected link which is not on stp, we label it as a blocked link
			if(t == 1 && it == k->edge.end())
			{
				k->edge.insert(make_pair<uint, uint>(j, 0));
				cout << k->id << '\t' << j << endl;
				non_stp_num ++;
			}
			else if(t == 1 && it != k->edge.end() )
			{
				assert(it->second == 1);
			}
		}
	}
	non_stp_num /= 2;
	cout << "#non stp links = " << non_stp_num << endl;
	infile.clear();
	infile.close();


	// input request stream in every access host
	infile.open(host_node_id_filename.c_str());
	if(!infile)
	{
		perror("host file error!");
		exit(0);
	}
	uint outside_source_id = 0;
	infile >> outside_source_id;
   // set the host connecting gateway as the outside content source
	lan[outside_source_id]->is_outside_content_source = true;
	lan[outside_source_id]->is_host = true;

	int tmpi, nid;
//	uint nid;
	infile >> nid;
	lan[nid]->is_host = true;
	while(infile >> tmpi)
	{
		if(nid == -1)
		{
			nid = tmpi;
			lan[nid]->is_host = true;
			continue;
		}
		if(tmpi != -1)
		{
			req_input_entry tmp_rie;
			tmp_rie.cont = tmpi;
			tmp_rie.wait_time = 0;
			tmp_rie.MUST_FWD = false;
			lan[nid]->req_input.push_back(tmp_rie);
		}
		else
			nid = -1;
	}
	infile.clear();
	infile.close();


	// input content source for every access host
	infile.open(cont_source_filename.c_str());
	if(!infile)
	{
		perror("content source file error!");
		exit(0);
	}
	infile >> nid;
	while(infile >> tmpi)
	{
		if(nid == -1)
		{
			nid = tmpi;
			continue;
		}
		if(tmpi != -1)
			lan[nid]->SList.push_back(tmpi);
		else
			nid = -1;
	}
	infile.clear();
	infile.close();
}
	
		
/*
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
*/

