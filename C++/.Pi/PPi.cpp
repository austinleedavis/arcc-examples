#include<cstdio>
#include<cstdlib>
#include<cmath>
#include <mpi.h>

using namespace std;

int main (int argc, char *arg[]);

/* Remember to #include "mpi.h" */



   #define send_data_tag 2001
   #define return_data_tag 2002

 
/* It's noteworthy that this is the same Process as the UPi, but distributed */


   main(int argc, char *argv[])
   {

      double sum, partial_sum, partial_pi, pi;
      MPI_Status status;
      int my_id, root_process, ierr, num_procs, num_it, an_id,  numsteps, sender, start, begin, end;
 
      /* Initialize the MPI environment. Important for all tasks in MPI */     
      ierr = MPI_Init (&argc, &argv);



/* Grab process IDs and Number of Processes */
      /* Initializes id to a unique identifier */ 
      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
      /* Assign the number of processes avaialble to a varaible */
      ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

      /* Define Root Process as Rank 0*/
      root_process = 0;


/******************************/
/* Processes to run on Rank 0 */
/*****************************/
      if(my_id == root_process) {
	sum = 0;

		num_it = atoi(argv[1]);
		numsteps = num_it/num_procs;

		printf( "The number of steps to take is %i\n", num_it);
	
	/* Distribute variables, and recieve sums */
	for(an_id=1; an_id < num_procs; an_id++) {

		/* Send the numsteps variable to the slave processes */
		ierr = MPI_Send( &numsteps, 1, MPI_INT, an_id, send_data_tag, MPI_COMM_WORLD);
		/* Receive the partial sums from the slave processes */
		ierr = MPI_Recv(&partial_sum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, return_data_tag, MPI_COMM_WORLD, &status);
		sender = status.MPI_SOURCE;

		/* Print out the partial sums received */
		printf("Partial sum %f returned from process %i\n", partial_sum, sender);
		float step = 1.0/numsteps;
		partial_pi = partial_sum*step;

		/* Return the portion of the result that each sum contributes to */
		/* This just makes the partial sums have a little more meaning to them */
		printf("Partial results %f returned from process %i\n", partial_pi, sender);
		sum += partial_sum;
		pi = step*sum;

	}
		/* Print the final result after calculations are completed */
		printf("The grand total is: %f\n", pi);
}
/* end rank 0 */

/***************************************/
/* Processes to run on all other Ranks */
/***************************************/
	else {
      
		partial_sum = 0;
		float steps = 0;
		/* Receive the numsteps variable from Rank 0 */
		ierr = MPI_Recv( &numsteps, 1, MPI_INT, 
                root_process, send_data_tag, MPI_COMM_WORLD, &status);

		/*Calculate the number of steps each Process will run */
		/* remember that num_procs includes Rank 0 */
		steps = numsteps/(num_procs - 1);
		float step = 1.0/numsteps;
		int i;
                double x;
		
		/* Define intervals - This can also be done by the root Process */
                begin = (my_id - 1)*steps;
                end = my_id*steps;

		/* Run the calculations along those intervals */
                for (i = begin; i < end; i++)
                {
                        x = (i+0.5)*step;
                        partial_sum = partial_sum + 4.0/(1.0+x*x);
                }
 

		/* Send the partial sum to Rank 0 */
		ierr = MPI_Send( &partial_sum, 1, MPI_DOUBLE, root_process, return_data_tag, MPI_COMM_WORLD);
		}
/* end slave processes */


/*     Finalize the MPI execution environment    */
/* Always remember to finalize this environment! */
	ierr = MPI_Finalize();
}


	
/* For more examples of working MPI code in C:
http://condor.cc.ku.edu/~grobe/docs/intro-MPI-C.shtml */
