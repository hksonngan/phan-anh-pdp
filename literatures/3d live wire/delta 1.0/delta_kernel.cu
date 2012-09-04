#ifndef _DELTA_KERNEL_H_
#define _DELTA_KERNEL_H_

#include <stdio.h>
#define DELTA 1.0
#define INF 1e20
#define GN 262144
#define BUCKETSIZE 4096*8 //era 4096*8
#define NUMBUCKETS 1024

#define DOWN  0
#define UP    1
#define RIGHT 2
#define LEFT  3


//#define EMULATION

#ifdef EMULATION
#define DEBUG(x...) printf(x)
#else
#define DEBUG(x...) 
#endif

////////////////////////////////////////////////////////////////////////////////
//! Simple test kernel for device functionality
//! @param g_idata  input data in global memory
//! @param g_odata  output data in global memory
////////////////////////////////////////////////////////////////////////////////
texture<float, 2, cudaReadModeElementType> mytex0;
texture<float, 2, cudaReadModeElementType> mytex1;
texture<float, 2, cudaReadModeElementType> mytex2;
texture<float, 2, cudaReadModeElementType> mytex3;

__device__ void
demptyKernel(){
}

/*
__global__ void
deltaKernel( float* g_idata, float* g_odata) 
{


  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  //  const unsigned int num_threads = blockDim.x;

  // shared memory
  // the size is determined by the host application
  extern __shared__  float sdata[];


  // read in input data from global memory
  sdata[tid] = tex2D(tex,2,2);
  __syncthreads();

  // perform some computations
  sdata[tid] = (float) sdata[tid];
  __syncthreads();

  // write data to global memory
  g_odata[tid] = sdata[tid];
}
*/

__global__ void
labelKernel (int i, int* B,int* BCount,int* BPos, int tw,int th, int* RLoc,int* R, float* dR, float* d,int* vBucketMap){
//ver se não precisa colocar 4 BiCount em algum lugar...

  //todo: try to increase speed using shared memory for RLoc... think more about it (maybe RLoc is too big for shared memory)
  const unsigned int tid = threadIdx.x;
  const unsigned int num_threads = blockDim.x;

  int BiCount = BPos[i];
  int node, row, col, pos,index;
  float cost,f1,f2,fmin;

  //cleaning R
  //4 times because each node can be reached from up, down, left and right directions (and more 4 times because for each node 4 more are open)
//  DEBUG("BiCount %d lastpos %d\n",BiCount,4*(BiCount-1)+3);
  for(int k=0;  (num_threads*k + tid) < 16*BiCount;k++){
    index = 4*(num_threads*k + tid);
    R[index  ]=-1;
    R[index+1]=-1;
    R[index+2]=-1;
    R[index+3]=-1;
  
    dR[index  ]=INF;
    dR[index+1]=INF;
    dR[index+2]=INF;
    dR[index+3]=INF;
  }

  __syncthreads();
  const int dx[4]={0,0,1,-1};
  const int dy[4]={1,-1,0,0};


/*  //down direction
  dx[DOWN]=  0;
  dy[DOWN]=  1;

  //up direction
  dx[UP]=  0;
  dy[UP]= -1;

  //right direction
  dx[RIGHT]=  1;
  dy[RIGHT]=  0;

  //left direction
  dx[LEFT]= -1;
  dy[LEFT]=  0;*/
  

  for(int k=0; num_threads*k + tid < BiCount;k++){
    node = B  [ BUCKETSIZE*i + num_threads*k + tid];
    if(node!=-1){

      DEBUG("(tid %d) node %d(from B[%d] pos %d)\n",tid,node,i,num_threads*k + tid);
      for(int j=0;j<4;j++){
        //TRY TO OPTIMIZE THIS PART REMOVING THE IF AND SETTING IMPOSSIBLE EDGE VALUES TO INFINITY
        switch(j){
	  case 0:
           cost = tex2D(mytex0,node%tw,node/tw);
           break;
	  case 1:
           cost = tex2D(mytex1,node%tw,node/tw);
           break;
	  case 2:
           cost = tex2D(mytex2,node%tw,node/tw);
           break;
	  case 3:
           cost = tex2D(mytex3,node%tw,node/tw);
           break;
        }
//      cost = tex2D(mytex[j],node%tw,node/tw);//todo: change texture
        row = node/tw + dy[j];
        col = node%tw + dx[j];

        if( (row>=0) && (row < th) && (col >= 0) && (col < tw) ){
          RLoc [ row*tw + col ] = 4*(num_threads*k + tid)+j;
          DEBUG("(tid %d)Connecting node %d to be processed by %d\n",tid,row*tw+col,RLoc [ row*tw + col ]);
        }

      }

    }
  }

  __syncthreads();

  //copy Edges to R
  for(int k=0; num_threads*k + tid < BiCount;k++){
    node = B  [ BUCKETSIZE*i + num_threads*k + tid];
    if(node!=-1){
      for(int j=0;j<4;j++){
        switch(j){
  	case 0:
         cost = tex2D(mytex0,node%tw,node/tw);
         break;
	case 1:
         cost = tex2D(mytex1,node%tw,node/tw);
         break;
	case 2:
         cost = tex2D(mytex2,node%tw,node/tw);
         break;
	case 3:
         cost = tex2D(mytex3,node%tw,node/tw);
         break;
      }



        row = node/tw + dy[j];
        col = node%tw + dx[j];
              DEBUG("Pre-candidate in R %d (d=%f)\n",row*tw + col,d[node]+cost,dR [4*RLoc[row*tw + col]+j]);
        if( (row>=0) && (row < th) && (col >= 0) && (col < tw) ){ 
            DEBUG("Candidate in R %d (d=%f)\n",row*tw + col,d[node]+cost,dR [4*RLoc[row*tw + col]+j]);
          if((cost<=DELTA)&&( d[node]+cost < d[row*tw + col])){
              R [4*RLoc[row*tw + col]+j] = row*tw + col;
              dR[4*RLoc[row*tw + col]+j] = d[node]+cost;
              DEBUG("New node in R %d (d=%f,e=%f) in pos %d\n",R [4*RLoc[row*tw + col]+j],dR [4*RLoc[row*tw + col]+j],cost,4*RLoc[row*tw + col]+j);
              vBucketMap[node]=-1;
          }
        }
      }
    }
  }
  __syncthreads();

  //gathering data to find the minimum cost way to get to node n
  //TODO: OPTIMIZE IN SUCH A WAY IT WON'T BE NEEDED TO GO THROUGH THE 4 EDGES, since they store the same value
  for(int k=0;  (num_threads*k + tid) < 16*BiCount;k++){
    f1 = fminf( dR[4*(num_threads*k + tid)  ], dR[4*(num_threads*k + tid)+1] );
    f2 = fminf( dR[4*(num_threads*k + tid)+2], dR[4*(num_threads*k + tid)+3] );
    fmin = fminf(f1,f2);
    dR[4*(num_threads*k + tid)  ]=fmin;
    dR[4*(num_threads*k + tid)+1]=fmin;
    dR[4*(num_threads*k + tid)+2]=fmin;
    dR[4*(num_threads*k + tid)+3]=fmin;
  }
  __syncthreads();


//  demptyKernel();
//  __syncthreads();

  
}
__global__ void 
copyB2SKernel(int i, int* B, int* BCount,int* BPos, int* S, int* SCount){
  //TODO: optimize this code
  //there's an optimized way of doing this, which is by only
  //storing SCount = Scount+ BCount, as output
  //and controlling with local variables thread positions

  const unsigned int tid = threadIdx.x;
  const unsigned int num_threads = blockDim.x;

  int pos;

  int BiCount = BCount[i];
  for(int k=0; num_threads*k + tid < BiCount;k++){
    if(B[i*BUCKETSIZE+num_threads*k+tid]!=-1){
      pos = atomicAdd(&SCount[0],1);
      S[pos] = B[i*BUCKETSIZE+num_threads*k+tid];
    }
  }
  __syncthreads();
  BCount[i]=0;
  BPos[i]=0;
  __syncthreads();
//  DEBUG("(tid %d) SCount %d\n",tid,SCount[0]);


}

//Parallel relax edges
__global__ void
relaxKernel( int RCount, int* B,int* BCount,int* BPos, int* RLoc,int* R,float*  dR,float* d, int* vBucketLoc, int* vBucketMap, float* deb){

  const unsigned int tid = threadIdx.x;
  const unsigned int num_threads = blockDim.x;
  int v,bn,bn_old, index;
  float x;
//  int myAdd=0;
  deb[20]= (float)RCount;

  for(int k=0; num_threads*k + tid < RCount;k++){
    index = num_threads*k + tid;
    deb[2*index]= R[index];
    deb[2*index+1]= dR[index];
  }

//  DEBUG("relaxing RCount %d\n",RCount);

  //remove node from old bucket
//  RCount = RCount /4;



  for(int k=0; num_threads*k + tid < RCount;k++){
    index = num_threads*k + tid;
  //  deb[index]= R[index];

    if(R[index]!=-1){

      x = dR[index];      
      v = R[index];

      if(x<d[v]){

        bn_old = vBucketMap[v];
        if (bn_old != -1) {
          int oldIndex = bn_old*BUCKETSIZE+vBucketLoc[v];
//	  
          B[oldIndex] = -1;//GN;
          int oldc = atomicSub(&BCount[bn_old],1);

//        printf("Removing %d from %d(%d)\n",v,bn_old,oldc);


        }
      }

    }    

  }




  __syncthreads();






  for(int k=0; num_threads*k + tid < RCount;k++){

    if(R[num_threads*k + tid]!=-1){
//deb[0]= (float) (BUCKETSIZE);
      x = dR[num_threads*k + tid];
      v = R[num_threads*k + tid];
      if(x < d[v]){



      bn = (int) (dR[num_threads*k + tid]/DELTA); 
      
   //   printf("Bn %d\n",bn);

      atomicAdd(&BCount[bn],1);
      int pos = atomicAdd(&BPos[bn],1);
      DEBUG("Pos %d BCount[%d] %d node %d (x=%f)\n",pos,bn,BPos[bn],v,x);

      
      B[bn*BUCKETSIZE+pos] = v;
      d[v] = x;

      
      vBucketLoc[v] = pos;  
      vBucketMap[v] = bn;
      RLoc[v]=-1;


  //only debug info
//    for(int i=0;i<BPos[bn];i++){
//      DEBUG("B(%d)=%d ",i,B[bn*BUCKETSIZE+i]);
//    }
//    DEBUG("\n");



      }

    }
  }


  __syncthreads();

  
}


__global__ void
labelHeavyKernel (int i, int* S,int* SCount,int tw, int* RLoc,int* R, float* dR, float* d,int* vBucketMap){
  //todo: try to increase speed using shared memory for RLoc... think more about it (maybe RLoc is too big for shared memory)
  const unsigned int tid = threadIdx.x;
  const unsigned int num_threads = blockDim.x;

  int mySCount = SCount[0];
  DEBUG("Heavy mySCount %d\n",mySCount);
  for(int k=0; num_threads*k + tid < mySCount;k++){
    int node = S  [ num_threads*k + tid];
    //TODO FIX HEAVY EDGES TEX...
    float downCost   = tex2D(mytex0,node%tw,node/tw);//todo: repeat for left,right and down

    int downRow = node/tw +1;
    int downCol = node%tw;
    int down = downRow*tw+downCol;
    DEBUG("Node %d %f\n", node,down);
    if(down<512*512){
      RLoc[ down ] = num_threads*k + tid;
    
      R[num_threads*k + tid]=-1;
    }
//    dR[num_threads*k + tid]=-1.0;
  }
  DEBUG("done (i=%d) %d\n",i,tid);

  __syncthreads();
  //copy Edges to R
  for(int k=0; num_threads*k + tid < mySCount;k++){
    int node = S  [ num_threads*k + tid];
    //TODO FIX HEAVY EDGES TEX...
    float downCost   = tex2D(mytex0,node%tw,node/tw);//todo: repeat for left,right and down
    //float edgeCost = up;
    int downRow = node/tw +1;
    int downCol = node%tw;
    int down = downRow*tw+downCol;

    if(downCost>DELTA){
      if(down<512*512){
      R[RLoc[down]]  = down;
      dR[RLoc[down]] = d[node]+downCost;
      DEBUG("Heavy new node in R %d (d=%f) from node %d\n",down,dR[RLoc[down]],node);
      vBucketMap[node]=-1;
      }
    }
  }
  
  __syncthreads();
  //todo: empty S here in order to be faster
  
}



__global__ void
emptyKernel(){
}
// B is the bucket i vector
// RLoc[n] stores the position of node n in R (so that if more than one attempt to update
// the distance to node n is made at the same time, it can be shifted to 0,1,2 or 3 in the position of R)


__global__ void
labelBisKernel( int i, int* B,int * Bi,int* R, int * RLoc,float* dR, int* S, int BSize,int* BCount, int* BiCount, int tw , float* d,int* vBucketLoc, int* vBucketMap) {  

 int k;
int v,bn, bn_old;
float x;
int pos;
//  BCount[i]--;
//  *BiCount = 1000;
  //This is the labelling part
  //Firstly, we will mark the position for node n
  //If n appears more than once while expanding Bi, and two threads try to 
  //set different positions for the same node, 
  //it is granted that only one of them will succeed,
  //hence, after the labelling part, each node of Bi will
  //be set to only one place in R
  
  const unsigned int tid = threadIdx.x;
  const unsigned int num_threads = blockDim.x;
//  extern __shared__ 
  __shared__ int Btemp[512];

//  for(int k=0;num_threads * k < BiCount; k++){
//  if(tid<BiCount)//tex2D(tex,2,2))





  //  int Btemptid;

  //Btemp[0]=tex2D(tex,0,0);
  

//  Btemp[tid]=tex2D(tex,node%tw,node/tw);//node%tw,node/tw);//B[tid];

//  __syncthreads();
//  }
//  RLoc[tid]=-1;


  //labelling part finishes here 
  //now we are set to copy edges to R

//for(k=0;k<10;k++){ 
int controlVar = 1;

//while(BCount[0]!=0){

//for(int g=0;g<B2;g++){
//printf("Tid %d Bcount[%d] %d\n",tid,i,BCount[i]);
// controlVar=0;
 if( tid < BCount[i]){
//  printf("Tid %d Bcount[%d] %d\n",tid,i,BCount[i]);

while(BCount[i]!=0){  
//  printf("Tid %d Inside\n",tid,i,BCount[i]);
  int node = B[i*BUCKETSIZE+tid];

//  Btemptid=tex2D(tex,node%tw,node/tw);
//  Btemp[tid]=tex2D(tex,0,0);
//  __syncthreads();
//  if(((node%tw)<1) && ((node%tw)>=0) && ((node/tw)<1) && ((node/tw)>=0))
  int b = node/tw;
  int a = node%tw;
  //WARNING TEX IS WRONG HERE
  Btemp[tid]=tex2D(mytex0,a,b);
  //printf("(tid %d)Tex %d\n",tid,Btemp[tid]);
//__syncthreads();

//printf("Tid %d Still alive\n",tid);

  RLoc[Btemp[tid]]=tid;
//  __syncthreads();

//printf("Tid %d Still alive\n",tid);

  R[RLoc[Btemp[tid]]] = Btemp[tid];
  dR[RLoc[Btemp[tid]]] = d[node]+1.1;

//  __syncthreads();

  //copy B[i] to S
  //duplicates are allowed... they will cause no problem when creating heavy R
  //two threads might be doing the same thing at the same time, which might cause 
  //an access conflit, but that's all

  S[tid] = B[i*BUCKETSIZE+tid];
  

  //TODO: check if clear Bi is correct
  atomicSub(&BCount[i],1);
//  BCount[i]=0;
  controlVar=0;

  
  //Parallel relax light edges
  v = R[tid];
  bn = (int) (dR[tid]/DELTA);
  bn_old = vBucketMap[v];
//  __syncthreads();
//  d[tid]=0.0+bn;
//  d[tid] = __int2float_rn(bn_old)+20;
//  printf("Hello world (%d) bn=%d bn_old=%d\n",tid,bn,bn_old);



  //remove v from old bucket
  if (bn_old != -1) {
    B[bn_old*BSize+vBucketLoc[v]] = GN;
    BCount[bn_old]--;
  }

  //insert v in new bucket
  //TODO: fix BCount
  //  int BCount = 100;
  x = dR[tid];//+BCount[i]+i;

  pos  = BCount[i] + RLoc[v];
  //printf("(tid %d) bn %d pos %d totpos %d v%d\n",tid,bn,pos,bn*BUCKETSIZE+pos,v);
  B[bn*BUCKETSIZE+pos] = v;
  d[v] = x;

  vBucketLoc[v] = pos;  
  vBucketMap[v] = bn;
  RLoc[v]=-1;
  atomicAdd(&BCount[bn],1);
//  BCount[bn]++;


//  __syncthreads();
  }
 }
// }
}

#endif // #ifndef _MEMORY_KERNEL_H_
