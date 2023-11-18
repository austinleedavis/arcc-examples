(*Limits Mathematica to requested resources*)
Unprotect[$ProcessorCount];$ProcessorCount = 30;
 
(*Prints all Mersenne PRime numbers less than 20000*)
Print[Parallelize[Select[Range[20000],PrimeQ[2^#-1]&]]];
