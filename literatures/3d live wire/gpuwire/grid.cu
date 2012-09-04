//compile with
//nvcc delta.cu -I /home/baggio/NVIDIA_CUDA_SDK/common/inc/ -L /home/baggio/NVIDIA_CUDA_SDK/lib/ -lcuda -lcudart -lcutil -lGL -lGLU
// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

// includes, project
#include <cutil.h>
#include <cuda_gl_interop.h>
//#define TIMER

// includes, kernels
#include <delta_kernel.cu>



#define INF 1e20


float* pixels = NULL;
cudaArray *array = NULL;

struct edge{
  int dNode[4];
  float weight[4];
};

edge* nodes;





void
runTest( int argc, char** argv);

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv) 
{

    runTest( argc, argv);

    CUT_EXIT(argc, argv);
}

void loadTexture(int iw, int ih, float* data){
    cudaChannelFormatDesc desc = cudaCreateChannelDesc<float>();
    CUDA_SAFE_CALL(cudaMallocArray(&array, &desc, iw, ih));
    CUDA_SAFE_CALL(cudaMemcpyToArray(array, 0, 0, data, sizeof(float)*iw*ih, cudaMemcpyHostToDevice));
    // Bind the array to the texture
    cudaBindTextureToArray( tex, array, desc);

}

void loadGraphEdges(){
  int n,m;
  FILE* in = fopen("512.graph","r");//myFile,"r");
  fscanf(in,"%d\n",&n);
  
  while(1){
    int source, dest;
    double eWeight;
    fscanf(in,"%d",&source);
//    printf("Reading %d\n",source);
    if(source==-1) break;
    fscanf(in,"%d%lf\n",&dest,&eWeight);

    nodes[source].dNode[0]= dest;
    nodes[source].weight[0]= (float)eWeight;

    nodes[dest].dNode[0] = source;
    nodes[dest].weight[0] = (float)eWeight;

  }
  
}


void loadGraph(int imgWidth,int imgHeight){

    pixels = (float*) malloc (imgWidth*imgHeight*sizeof(float));
    for(int i=0;i<imgWidth*imgHeight;i++){
        pixels[i]=nodes[i].weight[0];//(2*i);
//        printf("Loading edge %f real %f\n",nodes[i].weight[0],pixels[i]);
    }
    loadTexture(imgWidth,imgHeight,pixels);

}

////////////////////////////////////////////////////////////////////////////////
//! Run a simple test for CUDA
////////////////////////////////////////////////////////////////////////////////
void
runTest( int argc, char** argv) 
{



    //initialize the device
    cudaSetDevice(0);

    unsigned int num_threads = 256;//512;
//    unsigned int mem_size = sizeof( float) * num_threads;

    // setup execution parameters
    dim3  grid( 1, 1, 1);
    dim3  threads( num_threads, 1, 1);
    int iw=512, ih=512;

    nodes = (edge*) malloc(GN*sizeof(edge));
//	printf("Before loading\n");   
    loadGraphEdges();
//	printf("After loading\n");
    loadGraph(iw,ih);
//	printf("After loading\n");
    //allocate Bucket memory






    float* dDist;
    cudaMalloc( (void**) &dDist, GN*sizeof(float));
//    float hDist[Gn];
    float* hDist = (float*) malloc(GN*sizeof(float));
    
    for(int i=0;i<GN;i++){
      hDist[i]=INF;
    }
    for(int i=0;i<200;i++){
      hDist[i]=0.0;
    }

    cudaMemcpy( dDist, hDist, GN*sizeof(float), cudaMemcpyHostToDevice);

    int* dBucketMap;
    cudaMalloc( (void**) &dBucketMap, GN*sizeof(int));
    int* hBucketMap = (int*) malloc(GN*sizeof(int));
    
     
    for(int i=0;i<GN;i++){
      hBucketMap[i]=-1;
    }

    cudaMemcpy( dBucketMap, hBucketMap, GN*sizeof(int), cudaMemcpyHostToDevice);

    int* dBucketPos;
    cudaMalloc( (void**) &dBucketPos, GN*sizeof(int));
    int* hBucketPos = (int*) malloc(GN*sizeof(int));

    int* dB;
    cudaMalloc( (void**) &dB, BUCKETSIZE*NUMBUCKETS*sizeof(int));
    int* hB = (int*) malloc(BUCKETSIZE*NUMBUCKETS*sizeof(int));
  

    printf("Bucket size %d\n",BUCKETSIZE);

    int* hBi = hB;
    int hBiCount[1];
    hBiCount[0] = 200;
    for(int i=0;i<BUCKETSIZE*NUMBUCKETS;i++)
      hBi[i]=-1;
    for(int i=0;i<hBiCount[0];i++)
      hBi[i]=i;

    //duplicated values to test
//    hBi[0]=8;
//    hBi[31]=8;



    // copy host memory to device
    cudaMemcpy( dB, hBi, BUCKETSIZE*NUMBUCKETS*sizeof(int), cudaMemcpyHostToDevice);


    int* BCount;
    cudaMalloc( (void**) &BCount, NUMBUCKETS * sizeof(int));
    int* hBCount;
    hBCount = (int*) malloc(NUMBUCKETS*sizeof(int));
    for(int i=0;i<NUMBUCKETS;i++)
      hBCount[i]=0;

    hBCount[0]=200;

    // copy host memory to device
    cudaMemcpy( BCount, hBCount, NUMBUCKETS*sizeof(int), cudaMemcpyHostToDevice);

    // allocate device memory for result
    int * dRLoc;
    cudaMalloc( (void**) &dRLoc, GN*sizeof(int));
    int * dR;
    cudaMalloc( (void**) &dR, 4*8*BUCKETSIZE*sizeof(int)); //R is 4 times bigger than RLoc, because UP,DOWN,LEFT and RIGHT will each have a pos in R

    int* hR = (int*) malloc(4*8*BUCKETSIZE*sizeof(int));
    int* hRLoc = (int*) malloc(GN*sizeof(int));
    float* hDistR= (float*) malloc(4*8*BUCKETSIZE*sizeof(float));


    float* dDistR;
    cudaMalloc( (void**) &dDistR, 4*8*BUCKETSIZE*sizeof(float));


    int * dS;
    cudaMalloc( (void**) &dS, 8*BUCKETSIZE*sizeof(int));
    int * hS= (int*) malloc(8*BUCKETSIZE*sizeof(int));
    int * dSCount;
    cudaMalloc( (void**) &dSCount, 1*sizeof(int));
    int * hSCount= (int*) malloc(1*sizeof(int));

    printf("Starting timer\n");

  unsigned int nvtimer = 0;
    cutCreateTimer( &nvtimer);
    cutStartTimer( nvtimer);


  unsigned int laptimer = 1;
    cutCreateTimer( &laptimer);
  float ktime;

       float* lido;
    cudaMalloc( (void**) &lido, GN*sizeof(float));
     float* Hlido = (float*) malloc(GN*sizeof(float)) ;
    


  
    for(int i=0;i<1024;i++){
#ifdef TIMER
      printf("%d\n",i);
#endif
      int* RCount;
      RCount = (int*) malloc(1*sizeof(int));
      cudaMemcpy( RCount,  &BCount[i],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;    


      //S <- EMPTY
      hSCount[0]=0;
      cudaMemcpy( dSCount, hSCount,1*sizeof(int), cudaMemcpyHostToDevice);
    
//  printf("Before RCount %d(i=%d)\n",RCount[0],i);
  //    printf("Still safe%d\n",i);

      //While B[i] != EMPTY
      int sameCount = 0;
      cutStartTimer( laptimer);
      while(RCount[0]!=0){
//      if(RCount[0]==0) i++;
        sameCount++;

#ifdef TIMER
        cutStartTimer( laptimer);
#endif

       

        labelKernelDebug    <<<grid, threads >>> ( i, dB, BCount, iw, dRLoc, dR, dDistR, dDist,dBucketMap,lido);
        cudaThreadSynchronize();
        labelKernelDebug1   <<<grid, threads >>> ( i, dB, BCount, iw, dRLoc, dR, dDistR, dDist,dBucketMap,lido);

//        labelKernel    <<<grid, threads >>> ( i, dB, BCount, iw, dRLoc, dR, dDistR, dDist,dBucketMap);
        cudaThreadSynchronize();

if(i>100){
	cudaMemcpy( hB, dB, BUCKETSIZE*NUMBUCKETS*sizeof(int),cudaMemcpyDeviceToHost);
	        for(int j=0;j<RCount[0];j++){ 
		  if(hB[BUCKETSIZE*i+j]%512>=200){
		  printf("Alarm!!! B[%d]=%d (i=%d)\n",j,hB[BUCKETSIZE*i+j],i);
 		  }
	        }
	int* mytcount = (int*) malloc(1*sizeof(int));
	cudaMemcpy( mytcount,  &BCount[i+1],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;    
	        for(int j=0;j<mytcount[0];j++){ 
		  if(hB[BUCKETSIZE*(i+1)+j]%512>=200){
		  printf("Alarm(lookahead)!!! B[%d]=%d (i=%d)\n",j,hB[BUCKETSIZE*(i+1)+j],i+1);
 		  }
	        }

}




#ifdef TIMER
        ktime = cutGetTimerValue( laptimer );
        printf("Label         kernel %f\n",ktime);	
        cutStartTimer( laptimer);
#endif

        copyB2SKernel  <<<grid, threads >>> ( i, dB, BCount, dS, dSCount);
        cudaThreadSynchronize();


if(i>100){
      int mytemp[1];
      cudaMemcpy( mytemp,  &dSCount[0],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;

	cudaMemcpy( hS, dS,8*BUCKETSIZE*sizeof(int),cudaMemcpyDeviceToHost);
	        for(int j=0;j<mytemp[0];j++){ 
		  if(hS[j]%512>=200){
		  printf("Heavyalarm!!! S[%d]=%d (i=%d)\n",j,hS[j],i);
 		  }
		  else{
                    printf("S[%d]=%d (i=%d)\n",j,hS[j],i);
		  }
	        }
}


#ifdef TIMER
        ktime = cutGetTimerValue( laptimer );
        printf("CopyB2S       kernel %f\n",ktime);	
#endif

#ifdef TIMER
        cutStartTimer( laptimer);
#endif

        emptyKernel    <<<grid, threads >>> ();

#ifdef TIMER
        ktime = cutGetTimerValue ( laptimer );
	printf(" Empty        kernel %f\n",ktime);
#endif

        //todo: correct RCount
        cutStartTimer( laptimer);

        relaxKernel    <<<grid, threads >>> ( RCount[0], dB, BCount, dRLoc, dR, dDistR, dDist,dBucketPos, dBucketMap,lido);
        cudaThreadSynchronize();

#ifdef TIMER
        ktime = cutGetTimerValue( laptimer );
        printf("Relaxing      kernel %f\n",ktime);
#endif


        cudaMemcpy( RCount,  &BCount[i],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;
        cudaMemcpy( hSCount,  dSCount,  1*sizeof(int),       cudaMemcpyDeviceToHost) ;        

       printf("After RCount %d (i=%d) | SCount %d\n",RCount[0],i,hSCount[0]);

	

#ifdef TIMER
        printf("After RCount %d (i=%d) | SCount %d\n",RCount[0],i,hSCount[0]);
#endif


      }

//      ktime = cutGetTimerValue( laptimer );
#ifdef TIMER
      printf("Same called %d times.\n",sameCount);
#endif

//      printf("Label kernel %f\n",ktime);	
// 	printf("Before heavy label%d\n",i);      

	printf("Before heavy relaxing\n");

if(i>100){
      int mytemp[1];
      cudaMemcpy( mytemp,  &dSCount[0],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;

	cudaMemcpy( hS, dS,8*BUCKETSIZE*sizeof(int),cudaMemcpyDeviceToHost);
	        for(int j=0;j<mytemp[0];j++){ 
		  if(hS[j]%512>=200){
		  printf("before heavy label Heavyalarm!!! S[%d]=%d (i=%d)\n",j,hS[j],i);
 		  }
		  else{
                    printf("before heavy label S[%d]=%d (i=%d)\n",j,hS[j],i);
		  }
	        }
}

      cutStartTimer( laptimer);
      labelHeavyKernel    <<<grid, threads >>> ( i, dS, dSCount, iw, dRLoc, dR, dDistR, dDist,dBucketMap);
      cudaThreadSynchronize();

if(i>100){
      int mytemp[1];
      cudaMemcpy( mytemp,  &dSCount[0],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;

	cudaMemcpy( hR, dR,4*8*BUCKETSIZE*sizeof(int),cudaMemcpyDeviceToHost);
	        for(int j=0;j<mytemp[0];j++){ 
		  if(hR[j]%512>=200){
		  printf("heavylab Heavyalarm!!! R[%d]=%d (i=%d)\n",j,hR[j],i);
 		  }
		  else{
                    printf("heavylab R[%d]=%d (i=%d)\n",j,hR[j],i);
		  }
	        }
}




      ktime = cutGetTimerValue( laptimer );
#ifdef TIMER
      printf("Labelling heavy kernel %f\n",ktime);
#endif

// 	printf("After heavy label%d\n",i);      
      //todo: correct SCount

#ifdef TIMER
      cutStartTimer( laptimer);
#endif
      int temp[1];
      cudaMemcpy( temp,  &dSCount[0],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;
      relaxKernel         <<<grid, threads >>> ( temp[0], dB, BCount, dRLoc, dR, dDistR, dDist,dBucketPos, dBucketMap,lido);
      ktime = cutGetTimerValue( laptimer );
#ifdef TIMER
      printf("Relaxing heavy  kernel %f\n",ktime);
#endif

if(i>100){
	cudaMemcpy( hB, dB, BUCKETSIZE*NUMBUCKETS*sizeof(int),cudaMemcpyDeviceToHost);
	        for(int j=0;j<RCount[0];j++){ 
		  if(hB[BUCKETSIZE*i+j]%512>=200){
		  printf("hrelax Alarm!!! B[%d]=%d (i=%d)\n",j,hB[BUCKETSIZE*i+j],i);
 		  }
	        }
	int* mytcount = (int*) malloc(1*sizeof(int));
	cudaMemcpy( mytcount,  &BCount[i+1],  1*sizeof(int),       cudaMemcpyDeviceToHost) ;    
	        for(int j=0;j<mytcount[0];j++){ 
		  if(hB[BUCKETSIZE*(i+1)+j]%512>=200){
		  printf("hrelax Alarm(lookahead)!!! B[%d]=%d (i=%d)\n",j,hB[BUCKETSIZE*(i+1)+j],i+1);
 		  }
	        }

}



	


// 	printf("After heavy relax%d\n",i);      

//      printf("Done here. i = %d\n",i);


    }
    // check if kernel execution generated and error

   CUT_CHECK_ERROR("Kernel execution failed");

 

    ktime = cutGetTimerValue( nvtimer );
    cudaMemcpy( hRLoc,  dRLoc,  1024*sizeof(int),       cudaMemcpyDeviceToHost) ;    
    cudaMemcpy( hRLoc,  dRLoc,  1024*sizeof(int),       cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hR,     dR,     4*1024*8*sizeof(int),   cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hDistR, dDistR, 4*1024*8*sizeof(float), cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hS,     dS,     1024*8*sizeof(float),   cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hDist,  dDist,  GN*sizeof(int),       cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hBCount, BCount,  NUMBUCKETS*sizeof(int),       cudaMemcpyDeviceToHost) ;
    cudaMemcpy( hB, dB,           2*NUMBUCKETS*sizeof(int),       cudaMemcpyDeviceToHost) ;

	cudaMemcpy( Hlido, lido, GN*sizeof(float),cudaMemcpyDeviceToHost);
	cudaMemcpy( hR, dR, GN*sizeof(int),cudaMemcpyDeviceToHost);
        for(int j=0;j<GN;j++){ 
	  printf("lido[%d]=%f\n",j,Hlido[j]);
        }



      for(int j=0;j<200;j++){

        printf("R[%d] %d dR[%d] %f RLoc[%d]=%d\n",j,hR[j],j,hDistR[j],j,hRLoc[j]);
      }
    for(int i=0;i<512*512;i++){
      if(i%512==0) printf("%3d",i/512);
      if(hDist[i]<INF)
        printf("%5.1f ",hDist[i]);
      else
        printf("INFINI ",hDist[i]);
      if(i%512==511) printf("\n");
    }      


  /*  printf("Num threads %d\n",num_threads);
    for(int i=0;i< 20;i++){
      printf("RLoc[%d] = %d\n",i,hRLoc[i]);
    }
    for(int i=0;i< 33;i++){
      printf("R[%d] = %d (%f)\n",i,hR[i],hDistR[i]);
    }
    for(int i=0;i< 20;i++){
      printf("S[%d] = %d \n",i,hS[i]);
    }
    for(int i=0;i< 128;i++){
      printf("d[%d] = %f \n",i,hDist[i]);
    }
    printf("BiCount %d\n",hBiCount[0]);
    
    for(int i=0;i< 20;i++){
      printf("hBCount[%d] = %d \n",i,hBCount[i]);
    }

    for(int i=0;i< 20;i++){
      printf("hB[%d] = %d \n",i,hB[i]);
    }
    for(int i=BUCKETSIZE;i<BUCKETSIZE+ 20;i++){
      printf("hB[%d] = %d \n",i,hB[i]);
    }
*/




    printf("It took %f ms\n", ktime);

    cudaFreeArray(array);
    cudaFree(dB);
    cudaFree(dRLoc);
    cudaFree(dDist);
    cudaFree(dBucketMap);
    cudaFree(dBucketPos);

    cudaFree(BCount);

    cudaFree(dR);
    cudaFree(dDistR);
    cudaFree(dS);
    cudaFree(dSCount);
  




  /*




    // allocate host memory
    float* h_idata = (float*) malloc( mem_size);

    // initalize the memory
    for( unsigned int i = 0; i < num_threads ; ++i) 
    {
        h_idata[i] = (float) i;
    }


    //float initTime = cutGetTimerValue( timer );

//    printf("Host init time %f ms\n", initTime);
    printf("Here\n");
    unsigned int timer = 0;
    CUT_SAFE_CALL( cutCreateTimer( &timer));
    CUT_SAFE_CALL( cutStartTimer( timer));

    cutStartTimer(timer);



    // allocate device memory
    float* d_idata;
    cudaMalloc( (void**) &d_idata, mem_size);


    float initTime = cutGetTimerValue( timer );
    printf("Allocate device memory time %f ms\n", initTime);
    cutStartTimer(timer);

    // copy host memory to device
    cudaMemcpy( d_idata, h_idata, mem_size,
                                cudaMemcpyHostToDevice) ;

    initTime = cutGetTimerValue( timer );
    printf("Copy host to device time %f ms\n", initTime);
    cutStartTimer(timer);


    // allocate device memory for result
    float* d_odata;
    cudaMalloc( (void**) &d_odata, mem_size);

    initTime = cutGetTimerValue( timer );
    printf("Allocate device memory for result time %f ms\n", initTime);
    cutStartTimer(timer);


   


    // execute the kernel
    deltaKernel<<< grid, threads, mem_size >>>( d_idata, d_odata);

    initTime = cutGetTimerValue( timer );
    printf("Executing kernel time %f ms\n", initTime);
    cutStartTimer(timer);


    // check if kernel execution generated and error
    CUT_CHECK_ERROR("Kernel execution failed");

    // allocate mem for the result on host side
    float* h_odata = (float*) malloc( mem_size);
    // copy result from device to host
    cudaMemcpy( h_odata, d_odata, sizeof( float) * num_threads,
                                cudaMemcpyDeviceToHost) ;

    initTime = cutGetTimerValue( timer );
    printf("Readback time %f ms\n", initTime);
    cutStartTimer(timer);

    cutStopTimer( timer);
    printf( "Processing time: %f (ms)\n", cutGetTimerValue( timer));
    cutDeleteTimer( timer);

    for(int i=0;i< num_threads;i++){
      printf("%f\n",h_odata[i]);
    } 
    // cleanup memory
    free( h_idata);
    free( h_odata);
    cudaFree(d_idata);
    cudaFree(d_odata);*/
}
