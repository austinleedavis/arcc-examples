These are the converted linear algebra representations for solving the
Erosion Damage problem.  The system is expressed as K x = f,
where K is the stiffness matrix and f is the loading vector.

Since the system can be non-symmetric and we are using a simple conjugate gradient solver (which requires the matrix to be symmetric and positive definite), we apply the following precondition:

K^t * K * x = K^t * f

Both stiffness matrices are contained in a single tarred, gzipped file due to size.

K-matrices.tgz
  K.ell         stiffness matrix in ELL format  
  Kt-K.ell        symmetric K^t * K matrix in ELL format

f.vct           loading vector
Kt-f.vct        K^t * f vector

constraints.txt  problem constraints used to formulate f' and K'
solutions.vct    true solutions to the problem as given by Dr. Raghavan's group


These problems were developed by Dr. Seetha Raghavan's group as a part of 2011 grant 
with NASA/JPL and Intelligent Automation Inc.
