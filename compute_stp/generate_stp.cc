#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

#define VERTEXNUM 10

void createGraph(int (*edge)[VERTEXNUM], int start, int end, int value);
void displayGraph(int (*edge)[VERTEXNUM]);
void prim(int (*edge)[VERTEXNUM], int (*tree)[VERTEXNUM], int startVertex, int* vertexStatusArr, bool & connect);
void saveGraph(int (*edge)[VERTEXNUM], string &filename);

void addLinks(int (*edge)[VERTEXNUM], string & filename)
{
	ifstream infile;
	infile.open(filename.c_str());
	if(!infile)
	{
		perror("file open error!");
		exit(0);
	}
	int n1, n2;

//	int len;
	while(infile >> n1 >> n2 )
		createGraph(edge, n1, n2, 1);
	infile.clear();
	infile.close();
}

int main(int argc, char **argv)
{
	int (*edge)[VERTEXNUM] = (int (*)[VERTEXNUM])malloc(sizeof(int)*VERTEXNUM*VERTEXNUM);
	int (*tree)[VERTEXNUM] = (int (*)[VERTEXNUM])malloc(sizeof(int)*VERTEXNUM*VERTEXNUM);
	int i,j;
	for(i=0;i<VERTEXNUM;i++)
	{
		for(j=0;j<VERTEXNUM;j++)
		{
			edge[i][j] = 0;
		}
	}
	int* vertexStatusArr = (int*)malloc(sizeof(int)*VERTEXNUM);
	for(i=0;i<VERTEXNUM;i++)
	{
		vertexStatusArr[i] = 0;
	}
   printf("after init:\n");
   displayGraph(edge);
	
	/*
	createGraph(edge,0,1,6);
	createGraph(edge,0,3,5);
	createGraph(edge,0,2,1);
	createGraph(edge,1,2,5);
	createGraph(edge,1,4,3);
	createGraph(edge,2,4,6);
	createGraph(edge,2,3,5);
	createGraph(edge,2,5,4);
	createGraph(edge,3,5,2);
	createGraph(edge,4,5,6);
	*/
	string filename = argv[1];
	addLinks(edge, filename);
	printf("after create:\n");
	displayGraph(edge);
	string originalGraph = filename + "-originalGraph";
	saveGraph(edge, originalGraph);
//	int (*tree)[VERTEXNUM] = NULL;
	
	bool connect = false;
	prim(edge, tree, 0, vertexStatusArr, connect);
	if(connect == true)
	{
		printf("after generate tree:\n");
		displayGraph(tree);
		string stpGraph = filename + "-stp";
		saveGraph(tree, stpGraph);
	}
	free(edge);
	free(tree);
	return 0;
}
// add link
void createGraph(int (*edge)[VERTEXNUM], int start, int end, int value)
{
	edge[start][end] = value;
	edge[end][start] = value;
}
void displayGraph(int (*edge)[VERTEXNUM])
{
	int i,j;
	for(i=0;i<VERTEXNUM;i++)
	{
		for(j=0;j<VERTEXNUM;j++)
		{
			printf("%d ",edge[i][j]);
		}
		printf("\n");
	}
}

void saveGraph(int (*edge)[VERTEXNUM], string &filename)
{
	ofstream outfile;
	outfile.open(filename.c_str());
	if(!outfile)
	{
		perror("open file error!" );
		return;
	}
	int i,j;
	for(i=0;i<VERTEXNUM;i++)
	{
		for(j=0;j<VERTEXNUM;j++)
		{
			if(j != VERTEXNUM - 1)
				outfile << edge[i][j] << ' ';
			else
				outfile << edge[i][j] << endl;
		}
	}
	outfile.clear();
	outfile.close();
}


void prim(int (*edge)[VERTEXNUM], int (*tree)[VERTEXNUM], int startVertex, int* vertexStatusArr, bool & connect)
{
//	tree = (int (*)[VERTEXNUM])malloc(sizeof(int)*VERTEXNUM*VERTEXNUM);
	int i,j;
	for(i=0;i<VERTEXNUM;i++)
	{
		for(j=0;j<VERTEXNUM;j++)
		{
			tree[i][j] = 0;
		}
	}
	//record if the vertex is visited
	vertexStatusArr[0] = 1;
	int least, start, end, vNum = 1;
	//if there still exists unvisited vertexes
	while(vNum < VERTEXNUM)
	{
		least = 9999;
		connect = false;
		for(i=0;i<VERTEXNUM;i++)
		{
			//select a visited vertex
			if(vertexStatusArr[i] == 1)
			{
				for(j=0;j<VERTEXNUM;j++)
				{
					//select a unvisited vertex
					if(vertexStatusArr[j] == 0)
					{
						if(edge[i][j] != 0 && connect == false)
							connect = true;
						//select an edge with min value
						if(edge[i][j] != 0 && edge[i][j] < least)
						{
							least = edge[i][j];
							start = i;
							end = j;
						}
					}
				}
			}
		}
		vNum++;
		if(connect == false)
		{
			cout << "This is not a connected graph!!" << endl;
			return;
		}
		//set the vertex as visited
		vertexStatusArr[end] = 1;
		//add the edge to the tree
//		if(least == 9999)
//			cout << start << '\t' << end << endl;
		
		createGraph(tree,start,end,least);
	}
}
