/**
 * Sequential program to perform matrix-matrix multiplication
 *
 * To run this program:
 * 	(compile): g++ -msse3 -std=c++11 -fopenmp optimized_parallel_new_summary.cpp -o optimized_parallel_new_summary
 * 	(run): ./optimized_parallel <matrix_size>
 *
 * 
 */

#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>
#include <x86intrin.h>


using namespace std::chrono;
using namespace std;

#define S 50

#pragma intrinsic ( _mm_hadd_pd )

int s = S;


double** initMat(int size){
  double** mat = new double*[size];
  for (int i = 0; i < size; i++) {
    mat[i] = new double[size];
  }
  return mat;
}

double** getTranspose(double** matrix, int size){
  for (int row = 0; row < size; row++) {
    for (int col = row+1; col < size; col++) {
      std::swap(matrix[row][col], matrix[col][row]);
    }
  }
  return matrix;
}

void populateMat(double** matrix, int size){
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0,8);//The distribution in range 1-8

  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      matrix[row][col] = dis(gen);
      //cout<<matrix[row][col]<<" ";
    }
    //cout<<endl;
  }
  //cout<<endl;
}

long mat_multiply_compiler_intrinsics(double **matA, double **matB, int size){
  double** matC = initMat(size);

  high_resolution_clock::time_point start = high_resolution_clock::now();//Start clock

  double** trans_matB=getTranspose(matB,size);

   #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            __m128d c = _mm_setzero_pd();

            for (int k = 0; k < size; k += 2) {
                c = _mm_add_pd(c, _mm_mul_pd(_mm_load_pd(&matA[i][k]), _mm_load_pd(&trans_matB[j][k])));
            }
            c = _mm_hadd_pd(c, c);
            _mm_store_sd(&matC[i][j], c);
        }
    }  

  high_resolution_clock::time_point end = high_resolution_clock::now(); //End clock

  long duration = duration_cast<milliseconds>( end - start ).count();   //Get duration in milliseconds
  //cout<<duration<<endl;

  delete matA;
  delete matB;
  delete matC;

  return duration;

}

long matMultiply(int size){
  double** matA = initMat(size);
  double** matB = initMat(size);
  populateMat(matA , size);
  populateMat(matB, size);
  mat_multiply_compiler_intrinsics(matA,matB, size);
}



double getMean(long* runningTimes, int size){
  long sum = 0;
  double mean = 0;
  for(int i=0;i<size;i++){
    sum += runningTimes[i];
  }
  
  mean = sum;   
  mean = mean / size;
  return mean;
}

double getSD(long* runningTimes, int size, double mean){
  double variance = 0, sd =0;
  double* temp =  new double[size]; 
  for (int i = 0; i < size; i++) {
      temp[i] = runningTimes[i] - mean;
      temp[i] = pow(temp[i], 2.0); //to get the (x-average)……2
      variance += temp[i];
  }
  variance = variance / (size-1); // sample variance
  sd = sqrt(variance);
}


int main(int argc, const char* argv[]) {


  int noOfInitialSamples = 5;
  int* noOfSamples = new int[10];
  double* sampleMean = new double[10];  
  double* sampleStandardDeviation = new double[10];

  for(int x=100;x<=1000;x+=100)
  {
    int size = x;
    long* runningTimes = new long[noOfInitialSamples]; 
    double mean = 0, sd =0;
    cout<<"\n====================================================="<<endl;
    cout<<x<<"*"<<x<<" matrix multiplication (optimized parallel tiled)"<<endl;
    cout<<"====================================================="<<endl;
    cout<<"Performing 5 operetions to find the minimum number of required samples"<<endl;
    for(int i=0;i<noOfInitialSamples;i++){
      runningTimes[i]=matMultiply(size);
      cout<<runningTimes[i]<<", ";
    }
    
    mean = getMean(runningTimes,noOfInitialSamples);
    cout<<"\nmean = "<<mean<<endl;
        
    sd = getSD(runningTimes,noOfInitialSamples,mean);
    //cout<<"variance = "<<variance<<endl;
    cout<<"sample SD = "<<sd<<endl;
    
    int noOfRequiredSamples = ceil(pow(((100*1.960*sd)/(5*mean)), 2.0));
    
    cout<<"required no of samples= "<<noOfRequiredSamples<<endl;
    cout<<endl;
    
    if(noOfRequiredSamples<noOfInitialSamples){
    cout<<"*no need to performe more operations since there are already "<<noOfInitialSamples<<" samples"<<endl;
    noOfSamples[x/100 -1] = noOfInitialSamples;
    sampleMean[x/100 -1] = mean;
    sampleStandardDeviation[x/100 -1] = sd;
    }
    else{
    runningTimes = new long[noOfRequiredSamples]; 
    cout<<"Performing "<<noOfRequiredSamples<<" operetions to find the mean and Standard Deviation"<<endl;
    for(int i=0;i<noOfRequiredSamples;i++){
      runningTimes[i]=matMultiply(size);
      cout<<runningTimes[i]<<", ";
    }
    
    mean = getMean(runningTimes,noOfRequiredSamples);
    cout<<"\nmean = "<<mean<<endl;
        
    sd = getSD(runningTimes,noOfRequiredSamples,mean);
    //cout<<"variance = "<<variance<<endl;
    cout<<"sample SD = "<<sd<<endl;
    noOfSamples[x/100 -1] = noOfRequiredSamples;
    sampleMean[x/100 -1] = mean;
    sampleStandardDeviation[x/100 -1] = sd;
    }
    
   }
   
   cout<<"\n\n************ Summary ************"<<endl;
   cout<<"matrix size\t\tsample size\t\tmean\t\tStandard Deviation"<<endl;
   for(int i=0;i<10;i++){
   cout<<(i+1)*100<<"\t\t\t"<<noOfSamples[i]<<"\t\t\t"<<sampleMean[i]<<"\t\t\t"<<sampleStandardDeviation[i]<<endl;
   }
   
    
  
  return 0;
}
