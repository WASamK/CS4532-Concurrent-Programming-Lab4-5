/**
 * Sequential program to perform matrix-matrix multiplication
 *
 * To run this program:
 * 	(compile): g++ -std=c++11 sequential_summary.cpp -o sequential_summary
 * 	(run): ./sequential_summary
 *
 * 
 */

#include <iostream>
#include <random>
#include <chrono>
#include <math.h>

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

/*A method to populate a matrix with random values*/
void populateMat(double** matrix, int size){
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0,8);//The distribution in range 1-8

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      matrix[i][j] = dis(gen);
    }
  }
}

/*A method to perfrom matrix multiplication on given two matrices*/
long multiply(double **matA, double **matB, int size){
  double** resMat = initMat(size);

  high_resolution_clock::time_point start = high_resolution_clock::now();//Start clock

  for(int i = 0; i < size; i++){
    for(int j = 0; j < size; j++){
      resMat[i][j] = 0.0;
      for (int cur = 0; cur < size; cur++) {
        resMat[i][j] += matA[i][cur] * matB[cur][j];
      }
    }
  }


  high_resolution_clock::time_point end = high_resolution_clock::now(); //End clock

  long duration = duration_cast<milliseconds>( end - start ).count();   //Get duration in milliseconds

  delete  matA;     //Free the memory allocated for matA
  delete  matB;     //Free the memory allocated for matB
  delete  resMat;   //Free the memory allocated for resMat

  return duration;

}

/*A method that carries out matrix multiplication of two random matrices of a given size*/
long matMultiply(int size){

  double** matA = initMat(size);    //Initialize matrix A
  double** matB = initMat(size);    //Initialize matrix B
  populateMat(matA , size);         //Populate matrix A with random values
  populateMat(matB, size);          //Populate matrix B with random values
  return multiply(matA,matB, size); //Call the function to multiply the two matrices

}

/*A method to get the mean when an array of running times is given*/
double getMean(long* runningTimes, int size){
  long sum = 0;
  double mean = 0;
  for(int i=0;i<size;i++){         //A loop to add up the values
    sum += runningTimes[i];
  }
  
  mean = sum;                      //Assign the sum which was stored as a long value to a double variable 
  mean = mean / size;
  return mean;
}

/*A method to calculate the standard deviation when the distribution and the mean are given*/
double getSD(long* runningTimes, int size, double mean){
  double variance = 0, sd =0;
  double* temp =  new double[size]; 
  for (int i = 0; i < size; i++) {
      temp[i] = runningTimes[i] - mean;
      temp[i] = pow(temp[i], 2.0); //to get the (x-average)……2
      variance += temp[i];
  }
  variance = variance / (size-1);  // sample variance
  sd = sqrt(variance);
}


int main(int argc, const char* argv[]) {

  int noOfInitialSamples = 5;      //Initially use 5 samples to get the sample deviation to calculate the sample sie required for the given accuracy
  int* noOfSamples = new int[10];  //Since we have to get performance for 10 matrix sizes initialize an array to store the number of samples taken for each run
  double* meanRunTime = new double[10];  //initialize an array to store themean run time for each run
  double* sampleStandardDeviation = new double[10];   //initialize an array to store the sample mean for each run

  for(int x=100;x<=1000;x+=100)
  {
    int size = x,noOfRequiredSamples =0;

    long* runningTimes = new long[noOfInitialSamples]; //An array to store the running times of samples
    double mean = 0, sd =0;                            //Variables to store the mean and the standard deviation for a sample
    cout<<"\n====================================================="<<endl;
    cout<<x<<"*"<<x<<" matrix multiplication (serial)"<<endl;
    cout<<"====================================================="<<endl;
    cout<<"Performing 5 operetions to find the minimum number of required samples"<<endl;

    for(int i=0;i<noOfInitialSamples;i++){            //A loop to carry out first 5 samples and collect the running times
      runningTimes[i]=matMultiply(size);
      cout<<runningTimes[i]<<", ";
    }
    
    mean = getMean(runningTimes,noOfInitialSamples); //Get the mean running time for the initial 5 samples
    cout<<"\nmean = "<<mean<<endl;
        
    sd = getSD(runningTimes,noOfInitialSamples,mean); //Get the standard deviation for the initial 5 samples
    
    noOfRequiredSamples = ceil(pow(((100*1.960*sd)/(5*mean)), 2.0));  //Calculate the required number of samples to gain accuracy of ±5% and 95% confidence level.
    
    cout<<"Required no of samples= "<<noOfRequiredSamples<<endl;
    cout<<endl;
    
    if(noOfRequiredSamples<noOfInitialSamples)       //If we want less than 5 samples use the data from the run of initial five samples
    	cout<<"*no need to performe more operations since there are already "<<noOfInitialSamples<<" samples"<<endl;

    else{           				     //If we want more than 5 samples get required nu of samples                               
    	runningTimes = new long[noOfRequiredSamples];//Initialize the running times arrow to store running times for new sample set
    	cout<<"Performing "<<noOfRequiredSamples<<" operetions to find the mean and Standard Deviation"<<endl;

    	for(int i=0;i<noOfRequiredSamples;i++){     //Carry out multiplication for required number of times
      	   runningTimes[i]=matMultiply(size);
      	   cout<<runningTimes[i]<<", ";
   	 }
    
    	mean = getMean(runningTimes,noOfRequiredSamples);  //Get the new mean run time
    }

    sampleStandardDeviation[x/100 -1]=sd;
    noOfSamples[x/100 -1] = noOfRequiredSamples;
    meanRunTime[x/100 -1] = mean;
  }
   
   cout<<"\n\n************ Summary ************"<<endl;    //Display the results
   cout<<"matrix size\t\tStandard Deviation\t\tsample size\t\tmean"<<endl;
   for(int i=0;i<10;i++){
   cout<<(i+1)*100<<"\t\t\t"<<sampleStandardDeviation[i]<<"\t\t\t"<<noOfSamples[i]<<"\t\t\t"<<meanRunTime[i]<<endl;
}
  
  return 0;
}

