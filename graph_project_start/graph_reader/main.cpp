#include <iostream>
#include "graph.h"
#include <queue>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <omp.h>
using namespace std;
using namespace std::chrono;



//Algorithm 1: Single-threaded BFS frontier queue
char* BFSTraverse(graph<long, long, int, long, long, char>* ginst,int source)
{
  int j,frontier =0;
  char* statusArray = new char[ginst->vert_count];
  for (int i = 0; i < ginst->vert_count; i++)
    statusArray[i] = -1; //-1 means unvisited;
  

  int* currFrontierQueue = new int[ginst->vert_count];
  int* nextFrontierQueue = new int[ginst->vert_count];

  int currFrontierSize = 0;
  int nextFrontierSize = 0;

  currFrontierQueue[currFrontierSize] = source;
  currFrontierSize++;
  
  statusArray[source] = 0;
  int myFrontierIndex = 0;
  
  int currLevel = 1;
  
  while (true)
    {
      while (myFrontierIndex < currFrontierSize) {
	frontier = currFrontierQueue[myFrontierIndex];
	long int beg = ginst->beg_pos[frontier];
	long int end = ginst->beg_pos[frontier + 1];
	
	for (j = beg; j < end; j++) {
	  if (statusArray[ginst->csr[j]] == -1) {
	    statusArray[ginst->csr[j]] = currLevel;
	    nextFrontierQueue[nextFrontierSize] = ginst->csr[j];
	    nextFrontierSize++;
	    
	  }
	}
	myFrontierIndex++;
      }
      
      if (nextFrontierSize == 0) {
	return statusArray;
	
      } 

      //Swap current and next frontier queue;
      currFrontierSize = nextFrontierSize;
      myFrontierIndex = 0;
      nextFrontierSize = 0;
      
      int* temp = currFrontierQueue;
      currFrontierQueue = nextFrontierQueue;
		nextFrontierQueue = temp;
		currLevel++;
    }
  
}



//Algorithm 2: Single-threaded BFS status array
char* BFSTraverse2(graph<long, long, int, long, long, char>* ginst,int source)
{
  int ptr;
  int j;
  char* statusArray = new char[ginst->vert_count];
  for (int i = 0; i < ginst->vert_count; i++)
    statusArray[i] = -1; //-1 means unvisited;
  statusArray[source] = 0;
  int myFrontierCount = 0;
  int currLevel = 0;
while (true)
  {
    ptr = 0;
    while (ptr < ginst->vert_count) {
      if (statusArray[ptr] == currLevel) {
	int beg = ginst->beg_pos[ptr];
	int end = ginst->beg_pos[ptr + 1];
	for (j = beg; j < end; j++) {
	  if (statusArray[ginst->csr[j]] == -1) {
	    statusArray[ginst->csr[j]] = currLevel+1;
	  }
	}
      }
      else if (statusArray[ptr] != currLevel) {
	myFrontierCount++;
      }
      ptr++;
    }
    currLevel++;
    if (myFrontierCount == ginst->vert_count) {
      return statusArray;
    }
    myFrontierCount = 0;
  }
}

//Algorithm 2.5: Multithreaded vector reduction
/*int vect_add (int *a, int num, int thread_count)
{
  int res = 0;
#pragma omp num_threads (thread_count)
  {
    int work_per_thread = num/thread_count;
    int my_thread_id = omp_get_thread_num();
    int my_beg = my_thread_id * work_per_thread;
    int my_end = my_beg + work_per_thread;
    int my_res = 0;
    //In case num cannot be evenly divided by thread_count
    if (my_thread_id == thread_count - 1)
      {
	my_end = num;
      }
    while (my_beg < my_end)
      {
	my_res += a[my_beg];
	my_beg ++;
      }
    __sync_fetch_and_add(&res, my_res)
      }
  return res;
}
*/

 //Algorithm 3: Multi-threaded BFS status array
char* MultiBFSTraverse2(graph<long, long, int, long, long, char>* ginst,int source, int thread_count)
{
  char* statusArray = new char[ginst->vert_count];
  int j;  
  for (j = 0; j < ginst->vert_count; j++) {
    statusArray[j] = -1; //-1 means unvisited
  }
  statusArray[source] = 0;
  int myFrontierCount = 0;
  int currLevel = 0;

  while (true) {
    
#pragma omp parallel num_threads (thread_count)
    {
      int num = ginst->vert_count;
      int work_per_thread =  num / thread_count; 
      int my_thread_id = omp_get_thread_num();    
      int my_beg = my_thread_id * work_per_thread; 
      int my_end = my_beg + work_per_thread;       
      int my_myFrontierCount = 0;
      
      //In case vert_count can't be evenly divided by thread count
      if (my_thread_id == thread_count - 1) {
	my_end = ginst->vert_count;
      }
      
      while (my_beg < my_end) {
	if (statusArray[my_beg] == currLevel) {
	  int beg = ginst->beg_pos[my_beg];
	  int end = ginst->beg_pos[my_beg + 1];
	  
	  
	  for (j = beg; j < end; j++) {
	    if (statusArray[ginst->csr[j]] == -1) {
	      statusArray[ginst->csr[j]] = currLevel + 1;
	    }
	  }
	}
	
	else if (statusArray[my_beg] != currLevel) {
	  my_myFrontierCount++;
	}
	my_beg++;
      }
      __sync_fetch_and_add(&myFrontierCount, my_myFrontierCount);
    }
    currLevel++;
    
    if (myFrontierCount == ginst->vert_count) {
      return statusArray;
    }
    myFrontierCount = 0;
	}
}

//Algorithm 4: Multi-threaded BFS frontier queue
char* MultiBFSTraverse(graph<long, long, int, long, long, char>* ginst, int source, int thread_count) {
	//int j,frontier=0;
	int frontier = 0;
	char* statusArray = new char[ginst->vert_count];

	for (int i = 0; i < ginst->vert_count; i++)
		statusArray[i] = -1;//-1 means unvisited;

	int* currFrontierQueue = new int[ginst->vert_count];
	int* nextFrontierQueue = new int[ginst->vert_count];
	int currFrontierSize = 0;
	int nextFrontierSize = 0;
	currFrontierQueue[currFrontierSize] = source;
	currFrontierSize++;
	statusArray[source] = 0;
	int myFrontierIndex = 0;
	int currLevel = 1;
	int count = 0;
	while (true)
	  {
#pragma omp parallel num_threads(thread_count) 
	    {
	      int work_per_thread = currFrontierSize / thread_count;
	      int my_thread_id = omp_get_thread_num();
	      int my_beg = my_thread_id * work_per_thread;
	      int my_end = my_beg + work_per_thread;
	      
	      
	      if (my_thread_id == thread_count - 1) {
		my_end = currFrontierSize;
	      }
	      
	      int my_frontier = 0;
	      
	      while (my_beg < my_end) {
		my_frontier = currFrontierQueue[my_beg];
		long int beg = ginst->beg_pos[my_frontier];
		long int end = ginst->beg_pos[my_frontier + 1];
		
		for (int j = beg; j < end; j++) {
		  if (statusArray[ginst->csr[j]] == -1) {
		    statusArray[ginst->csr[j]] = currLevel;
		    count++;
		    nextFrontierQueue[__sync_fetch_and_add(&nextFrontierSize, 1)] = ginst->csr[j];
		  }
		}
		my_beg++;
	      }
	    }
	    
	    if (nextFrontierSize == 0) {
	      return statusArray;
	    }
	    
	    //Swap current and next frontier queue;
	    currFrontierSize = nextFrontierSize;
	    myFrontierIndex = 0;
	    nextFrontierSize = 0;
		
	    int* temp = currFrontierQueue;
	    currFrontierQueue = nextFrontierQueue;
	    nextFrontierQueue = temp;
	    currLevel++;
	  }
}




int main(int args, char **argv)
{
	std::cout<<"Input: ./exe beg csr weight\n";
	if(args!=4){std::cout<<"Wrong input\n"; return -1;}
	
	
	const char *beg_file=argv[1];
	const char *csr_file=argv[2];
	const char *weight_file=argv[3];
	
	//template <file_vertex_t, file_index_t, file_weight_t
	//new_vertex_t, new_index_t, new_weight_t>
	graph<long, long, int, long, long, char>
	*ginst = new graph
	<long, long, int, long, long, char>
	(beg_file,csr_file,weight_file);

	//number of times to run each algorithm
	const int iterations = 50;
	std::cout << "num iterations: " << iterations <<"\n";
	//run each set of trials with 1-4 threads
	for(int thread_count=1; thread_count<=4; thread_count++)
	  {
	    std::cout << "num threads: " << thread_count <<"\n";
	    auto start = high_resolution_clock::now();
	    for(int i=0; i<iterations; i++)
	      BFSTraverse(ginst, 0);
	    auto stop = high_resolution_clock::now();
	    auto duration = duration_cast<microseconds>(stop - start)*(1.0/iterations);
	    std::cout<<"Status array avg time: " << duration.count() << " microseconds \n";
	
	    start = high_resolution_clock::now();
	    for(int i=0; i<iterations; i++)
	      MultiBFSTraverse(ginst, 0,thread_count);
	    stop = high_resolution_clock::now();
	    duration = duration_cast<microseconds>(stop - start)*(1.0/iterations);
	    std::cout<<"Multi Status array avg time: " << duration.count() << " microseconds \n";
	 
	    
	    start = high_resolution_clock::now();
	    for(int i=0; i<iterations; i++)
	      BFSTraverse2(ginst, 0);
	    stop = high_resolution_clock::now();
	    duration = duration_cast<microseconds>(stop - start)*(1.0/iterations);
	    std::cout<<"Frontier Queue avg time: " << duration.count() << " microseconds \n";
	 
	    start = high_resolution_clock::now();
	    for(int i=0; i<iterations; i++)
	      MultiBFSTraverse2(ginst, 0,thread_count);
	    stop = high_resolution_clock::now();
	    duration = duration_cast<microseconds>(stop - start)*(1.0/iterations);
	    std::cout<<"Multi Frontier Queue avg time: " << duration.count() << " microseconds \n";

	  }

    //**
    //You can implement your single threaded graph algorithm here.
    //like BFS, SSSP, PageRank and etc.
	return 0;	
}


