CC = g++ 
CPPFLAGS = -std=c++14 -O2 -pthread -I. -I/usr/include/flint -I/usr/include/openmpi-x86_64 -I/usr/include/yaml-cpp
OBJS = opencl_interface.o reduction_table.o curve.o mpi_worker_pool.o
LIBRARIES = -L/usr/lib64/beignet/ -lcl
LIBRARIES+= -lflint
LIBRARIES+= -lyaml-cpp
LIBRARIES+= -lboost_serialization
LIBRARIES+= -L/usr/lib64/openmpi/lib -lboost_mpi -lmpi_cxx -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags


.PHONY : all, clean

all : single block mpi

single : single.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o single single.cc $(OBJS) $(LIBRARIES)

block : block.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o block block.cc $(OBJS) $(LIBRARIES)

mpi : mpi.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o mpi mpi.cc $(OBJS) $(LIBRARIES)

clean :
	rm -f single single.o block block.o mpi mpi.o $(OBJS)
