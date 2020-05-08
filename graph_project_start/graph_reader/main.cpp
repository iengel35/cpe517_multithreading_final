#include <iostream>
#include "graph.h"
#include <time.h>
#include <sys/sysinfo.h>
#include <omp.h>

const int thread_count = 4;

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
     if (nextFrontierSize == 0) { return statusArray; }
     //Swap current and next frontier queue;
     currFrontierSize = nextFrontierSize;
     myFrontierIndex = 0;
     nextFrontierSize = 0;
   }
 int* temp = currFrontierQueue;
 currFrontierQueue = nextFrontierQueue;
 nextFrontierQueue = temp;
 currLevel++;
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
char* MultiBFSTraverse2(graph<long, long, int, long, long, char>* ginst,int source)
{
  
  int ptr;
  int j;
  char* statusArray = new char[ginst->vert_count];
  char* my_statusArray = new char[ginst->vert_count];
  for (int i = 0; i < ginst->vert_count; i++){
    statusArray[i] = -1; //-1 means unvisited;
    my_statusArray[i] = -1; //-1 means unvisited;
  }
    
  statusArray[source] = 0;
  int myFrontierCount = 0;
  int currLevel = 0;
  //OMP vars
  #pragma omp num_threads (thread_count)
  {
    int num = ginst->vert_count;
    int work_per_thread = num/thread_count;
    int my_thread_id = omp_get_thread_num();
    int my_beg = my_thread_id * work_per_thread;
    int my_end = my_beg + work_per_thread;
    
    
    
  while (true)
    {
      ptr = 0;
      while (ptr < ginst->vert_count) {
	if (statusArray[ptr] == currLevel) {
	  int beg = ginst->beg_pos[ptr];
	  int end = ginst->beg_pos[ptr + 1];
	  
	  if (my_thread_id == thread_count - 1)
	    {
	      my_end = end;
	    }
	  //std::cout << "my_beg: " << my_beg << '\n';
	  //std::cout << "my_end: " << my_end << '\n';
	  while (my_beg < my_end)
	    {
	      if (statusArray[ginst->csr[my_beg]] == -1) {
		
		statusArray[ginst->csr[my_beg]] = currLevel+1;
		//my_res += a[my_beg];
		//std::cout << "my_beg: " << my_beg << '\n';
	      }
	      my_beg ++;
	    }
	  
	/*for (j = beg; j < end; j++) {
	if (statusArray[ginst->csr[j]] == -1) {
	  statusArray[ginst->csr[j]] = currLevel+1;
	}
	}*/
	}
    else if (statusArray[ptr] != currLevel) {
      myFrontierCount++;
    }
	ptr++;
      }
    
  currLevel++;
  
    
    

  if (myFrontierCount == ginst->vert_count) {
    break;
  }
  myFrontierCount = 0;
    }
  }
  __sync_fetch_and_add(&statusArray, my_statusArray);
  return statusArray;
}

//Algorithm 4: Multi-threaded BFS frontier queue
char* MultiBFSTraverse(graph<long, long, int, long, long, char>* ginst,int source)
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
     if (nextFrontierSize == 0) { return statusArray; }
     //Swap current and next frontier queue;
     currFrontierSize = nextFrontierSize;
     myFrontierIndex = 0;
     nextFrontierSize = 0;
   }
 int* temp = currFrontierQueue;
 currFrontierQueue = nextFrontierQueue;
 nextFrontierQueue = temp;
 currLevel++;
}



int main(int args, char **argv)
{
	std::cout<<"Input: ./exe beg csr weight\n";
	if(args!=4){std::cout<<"Wrong input\n"; return -1;}
	
	const char *beg_file=argv[1];
	const char *csr_file=argv[2];
	const char *weight_file=argv[3];
	//vars for timing function
	clock_t start, end;
	double cpu_time_used1, cpu_time_used2,  cpu_time_used3,  cpu_time_used4;
	
	//template <file_vertex_t, file_index_t, file_weight_t
	//new_vertex_t, new_index_t, new_weight_t>
	graph<long, long, int, long, long, char>
	*ginst = new graph
	<long, long, int, long, long, char>
	(beg_file,csr_file,weight_file);
	start = clock();
	for(int i=0; i<100; i++)
	  BFSTraverse(ginst, 0);
	end = clock();
	cpu_time_used1 = ((double) (end - start)) / (CLOCKS_PER_SEC*100);
	std::cout<<"Algorithm 1 avg time: " << cpu_time_used1 << " sec \n";
	start = clock();
	for(int j=0; j<100; j++)
	  BFSTraverse2(ginst, 0);
	end = clock();
	cpu_time_used2 = ((double) (end - start)) / (CLOCKS_PER_SEC*100);
	std::cout<<"Algorithm 2 avg time: " << cpu_time_used2 << " sec \n";
	start = clock();
	for(int j=0; j<1000; j++)
	  MultiBFSTraverse2(ginst, 0);
	end = clock();
	cpu_time_used3 = ((double) (end - start)) / (CLOCKS_PER_SEC*1000);
	std::cout<<"Algorithm 3 avg time: " << cpu_time_used3 << " sec \n";

    //**
    //You can implement your single threaded graph algorithm here.
    //like BFS, SSSP, PageRank and etc.
	return 0;	
}

	
    //for(int i = 0; i < ginst->vert_count+1; i++)
    //{
    //    int beg = ginst->beg_pos[i];
    //    int end = ginst->beg_pos[i+1];
    //    std::cout<<i<<"'s neighor list: ";
    //    for(int j = beg; j < end; j++)
    //        std::cout<<ginst->csr[j]<<" ";
    //    std::cout<<"\n";
    //} 
     

	

