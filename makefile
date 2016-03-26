CC = g++ 
CPPFLAGS = -std=c++14 -O2 -pthread -Ibuild/ -I/usr/include/flint -I/usr/include/openmpi-x86_64 -I/usr/include/yaml-cpp
BUILDDIR = build
OBJS_PURE = block_iterator.o curve.o curve_iterator.o fq_element_table.o isogeny_count_store.o isogeny_representative_store.o opencl_interface.o reduction_table.o
OBJS = $(foreach OBJ,$(OBJS_PURE),$(BUILDDIR)/$(OBJ))
OBJS_MPI_PURE = mpi_worker_pool.o 
OBJS_MPI = $(foreach OBJ,$(OBJS_MPI_PURE),$(BUILDDIR)/$(OBJ))
LIBRARIES = -L/usr/lib64/beignet/ -lcl
LIBRARIES+= -lflint
LIBRARIES+= -lyaml-cpp
LIBRARIES_MPI = -lboost_serialization
LIBRARIES_MPI+= -L/usr/lib64/openmpi/lib -lboost_mpi -lmpi_cxx -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags


.PHONY : all, clean, sync_src

all : single legacy mpi

sync_src :
	rsync -a src/ $(BUILDDIR)

single : sync_src $(BUILDDIR)/single.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o single $(BUILDDIR)/single.cc $(OBJS) $(LIBRARIES)

legacy : sync_src $(BUILDDIR)/legacy.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o legacy $(BUILDDIR)/legacy.cc $(OBJS) $(LIBRARIES)

mpi : sync_src $(BUILDDIR)/mpi.cc $(OBJS) $(OBJS_MPI)
	$(CC) $(CPPFLAGS) -o mpi $(BUILDDIR)/mpi.cc $(OBJS) $(OBJS_MPI) $(LIBRARIES) $(LIBRARIES_MPI)

clean :
	rm -f single legacy mpi $(BUILDDIR)/*
