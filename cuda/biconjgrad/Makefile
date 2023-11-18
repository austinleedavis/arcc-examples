CC=g++ -fPIC
LDFLAGS=-L${CUDA_HOME}/lib64/ -L${CUDA_HOME}/lib64/stubs/
LIBS=-lcuda -lcudart
INCFLAGS=-I${CUDA_HOME}/include -I./inc

CCFLAGS=-fpermissive -D USE_DOUBLE_PREC -Wdeprecated
NVCCFLAGS=-D USE_DOUBLE_PREC -ftz=false -prec-div=true -prec-sqrt=true 
NVCC=nvcc

DEPS=	inc/databuilder.h\
	inc/defs.h\
	inc/ellmatrix.h\
	inc/globalsingleton.h\
	inc/gpumeminterface.h\
	inc/kernelFunctions.h\
	inc/randutils.h\
	inc/vector.h 

OBJ=	obj/vector.o\
	obj/ellmatrix.o\
	obj/randutils.o\
	obj/databuilder.o\
	obj/biconjgrad.o\
	obj/globalsingleton.o

CUDA_OBJ = obj/kernelFunctions.ocu


all:	bin/biconjgrad bin/conjgrad


# Build all C++ object files into ./obj
obj/%.o:	src/%.cpp  $(DEPS)
	$(CC) -c -o $@  $<  $(CCFLAGS)  $(INCFLAGS)

# Build all Cuda object files into ./obj
obj/%.ocu:	src/%.cu  $(DEPS)
	$(NVCC) -c $<  -o $@   $(NVCCFLAGS)  $(INCFLAGS)

# Link to produce the Bi-Conjugate Gradient binary in ./bin (numerically stable)
bin/biconjgrad:	$(OBJ)  $(CUDA_OBJ) 
	$(CC) -o $@  $(OBJ)  $(CUDA_OBJ)  $(LDFLAGS) $(LIBS)

# Link to produce the Conjugate Gradient binary in ./bin (numerically unstable)
bin/conjgrad: $(OBJ)  $(CUDA_OBJ)
	$(CC) -o $@  $(OBJ)  $(CUDA_OBJ)  $(LDFLAGS) $(LIBS)



.PHONY:	clean

clean:
	rm -f obj/*; rm -f bin/*;  rm -f conjgrad biconjgrad

