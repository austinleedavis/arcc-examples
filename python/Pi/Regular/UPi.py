#pi_numba.py
import time




def loop(num_steps):
    step = 1.0/num_steps
    sum = 0
    for i in xrange(num_steps):
        x= (i+0.5)*step
        sum = sum + 4.0/(1.0+x*x)
    return sum


def Pi(num_steps ):
    start = time.time()
    step = 1.0/num_steps
    sum =loop(num_steps)
    pi = step * sum
    end = time.time()
    print "Pi with %d steps is %f in %f secs" %(num_steps, pi, end - start)
if __name__ == '__main__':
    Pi(100000000) 
