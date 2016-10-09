
/**
 * Sequential program to perform matrix-matrix multiplication
 *
 * To run this program:
 * 	(compile): g++ -mavx  -std=c++11 -fopenmp optimized_parallel.cpp -o optimized_parallel
 * 	(run): ./optimized_parallel
 *
 * 
 */

#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>


using namespace std::chrono;
using namespace std;


/*A method to initialize a matrix)*/
double** initMat(int size){
  double** mat = new double*[size];
  for (int i = 0; i < size; i++) {
    mat[i] = new double[size];
  }
  return mat;
}

/*A method to get the transpose matrix of a given matrix*/
double** getTranspose(double** matrix, int size){
  for (int row = 0; row < size; row++) {
    for (int col = row+1; col < size; col++) {
      std::swap(matrix[row][col], matrix[col][row]);
    }
  }
  return matrix;
}

/*A method to populate a matrix with random values*/
void populateMat(double** matrix, int size){
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0,8);//The distribution in range 1-8

  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      matrix[row][col] = dis(gen);
    }
  }
}

/*A method to perfrom matrix multiplication on given two matrices*/
double mat_multiply_avx(double **matA, double **matB, int size){
  double** matC = initMat(size);

  high_resolution_clock::time_point start = high_resolution_clock::now();//Start clock

  double** trans_matB=getTranspose(matB,size);

   #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            __m256d c = _mm256_setzero_pd();   //initialize 256 bit vector           
            double tempresult[4];               //array of 4 double elements to recieve the 256 result

            for (int k = 0; k < size; k += 4) {
                c = __builtin_ia32_addpd256(c, __builtin_ia32_mulpd256(__builtin_ia32_loadupd256(&matA[i][k]), __builtin_ia32_loadupd256(&trans_matB[j][k])));  //256 vector operations to multiply and sum
            }
            __builtin_ia32_storeupd256(tempresult, c);  //store result to the temp result array
            matC[i][j] = tempresult[0]+tempresult[1]+tempresult[2]+tempresult[3];  // take the sum of all four elements (4*64 = 256) and store 
        }
    }  

  high_resolution_clock::time_point end = high_resolution_clock::now(); //End clock

  double duration = (double)duration_cast<nanoseconds>( end - start ).count()/1000000;   //Get duration in nano seconds

  delete matA;     //Free the memory allocated for matA
  delete matB;     //Free the memory allocated for matB
  delete matC;     //Free the memory allocated for matC

  return duration;

}

/*A method that carries out matrix multiplication of two random matrices of a given size*/
double matMultiply(int size){
  double** matA = initMat(size);    //Initialize matrix A
  double** matB = initMat(size);    //Initialize matrix B
  populateMat(matA , size);         //Populate matrix A with random values
  populateMat(matB, size);          //Populate matrix B with random values
  return mat_multiply_avx(matA,matB, size); //Call the function to multiply the two matrices
}


/*A method to get the mean when an array of running times is given*/
double getMean(double* runningTimes, int size){
  double sum = 0;
  double mean = 0;
  for(int i=0;i<size;i++){         //A loop to add up the values
    sum += runningTimes[i];
  }
   
  mean = sum / size;
  return mean;
}

/*A method to calculate the standard deviation when the distribution and the mean are given*/
double getSD(double* runningTimes, int size, double mean){
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


  int noOfInitialSamples = 10;      //Initially use 10 samples to get the sample deviation to calculate the sample sie required for the given accuracy
  int* noOfSamples = new int[10];  //Since we have to get performance for 10 matrix sizes initialize an array to store the number of samples taken for each run
  double* sampleMean = new double[10];    //initialize an array to store themean run time for each run
  double* sampleStandardDeviation = new double[10];   //initialize an array to store the sample mean for each run

  for(int x=100;x<=1000;x+=100)
  {
    int size = x;
    double* runningTimes = new double[noOfInitialSamples];  //An array to store the running times of samples
    double mean = 0, sd =0;                                 //Variables to store the mean and the standard deviation for a sample

    cout<<"\n====================================================="<<endl;
    cout<<x<<"*"<<x<<" matrix multiplication (Optimized parallel AVX)"<<endl;
    cout<<"====================================================="<<endl;
    cout<<"Performing "<<noOfInitialSamples<<" operetions to find the minimum number of required samples"<<endl;

    for(int i=0;i<noOfInitialSamples;i++)            //A loop to carry out first 10 samples and collect the running times
      runningTimes[i]=matMultiply(size);
    
    mean = getMean(runningTimes,noOfInitialSamples); //Get the mean running time for the initial 10 samples
    cout<<"\nmean = "<<mean<<endl;
        
    sd = getSD(runningTimes,noOfInitialSamples,mean); //Get the standard deviation for the initial 10 samples
    //cout<<"variance = "<<variance<<endl;
    cout<<"sample SD = "<<sd<<endl;
    
    int noOfRequiredSamples = ceil(pow(((100*1.960*sd)/(5*mean)), 2.0));
    
    cout<<"required no of samples= "<<noOfRequiredSamples<<endl;
    cout<<endl;
    
    if(noOfRequiredSamples<noOfInitialSamples){       //If we want less than 10 samples use the data from the run of initial 10 samples
    	cout<<"*no need to performe more operations since there are already "<<noOfInitialSamples<<" samples"<<endl;
    	noOfSamples[x/100 -1] = noOfInitialSamples;
    }

    else{           				     //If we want more than 5 samples get required nu of samples  
    	runningTimes = new double[noOfRequiredSamples];//Initialize the running times arrow to store running times for new sample set 
    	cout<<"Performing "<<noOfRequiredSamples<<" operetions to find the mean and Standard Deviation"<<endl;

    	for(int i=0;i<noOfRequiredSamples;i++)     //Carry out multiplication for required number of times
        runningTimes[i]=matMultiply(size);
    
   	mean = getMean(runningTimes,noOfRequiredSamples);  //Get the new mean run time
    	cout<<"\nmean = "<<mean<<endl;
        
    	sd = getSD(runningTimes,noOfRequiredSamples,mean);
    	cout<<"sample SD = "<<sd<<endl;
    	noOfSamples[x/100 -1] = noOfRequiredSamples;
    }

    sampleMean[x/100 -1] = mean;
    sampleStandardDeviation[x/100 -1] = sd;
    
   }
   
   cout<<"\n\n************ Summary ************"<<endl;    //Display the results
   cout<<"matrix size\t\tsample size\t\tmean\t\tStandard Deviation"<<endl;
   for(int i=0;i<10;i++){
   cout<<(i+1)*100<<"\t\t\t"<<noOfSamples[i]<<"\t\t\t"<<sampleMean[i]<<"\t\t\t"<<sampleStandardDeviation[i]<<endl;
   }
   
    
  
  return 0;
}
