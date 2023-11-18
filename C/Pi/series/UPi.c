#include<stdio.h>
#include<stdlib.h>
#include<math.h>


double loop(int num_steps, int i);


int main(int argc, char *argv[]){  //need to get input for amount of terms

    int i, numsteps;
    double neg, pi, sum;
    sum = 0;  //setting sum to zero
    neg = -1;
    numsteps = atoi(argv[1]);  // Having the input be the number of steps. A nice feature, but not neccesary
    float step = 1.0/numsteps;  // Step = the size of the individual areas to approximate
    sum = loop(numsteps, i);  // Summation of the collective y values at the appropriate intervals
    pi = step * sum; // Finally multiply the collective y values with the Differential size
    printf("The approximation of pi is: %.16lf\n", pi); //Output Pi

    return 0;
}

double loop(int num_steps, int i) { 
	double x; // Declaring variables
        float step = 1.0/num_steps; //Redeclaration of step, which could also be passed on through the definition
        double sum = 0; //Set sum = 0
        for(i = 1; i < num_steps; i++)  // loop for adding all the y values The range is the number of steps
	{
		x = (i-0.5)*step; // We're doing a midpoint integral approximation
		sum = sum + 4.0/(1.0+x*x); // adding the value y values together
	}   
	return sum;
}
