CC = g++ 
CPPFLAGS = -std=c++14 -O2 -pthread -Ibuild/ -I/usr/include/flint -I/usr/include/openmpi-x86_64 -I/usr/include/yaml-cpp
BUILDDIR = build
OBJS_PURE = opencl_interface.o reduction_table.o curve.o mpi_worker_pool.o isogeny_type_store.o
OBJS = $(foreach OBJ,$(OBJS_PURE),$(BUILDDIR)/$(OBJ))
LIBRARIES = -L/usr/lib64/beignet/ -lcl
LIBRARIES+= -lflint
LIBRARIES+= -lyaml-cpp
LIBRARIES+= -lboost_serialization
LIBRARIES+= -L/usr/lib64/openmpi/lib -lboost_mpi -lmpi_cxx -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags


.PHONY : all, clean, sync_src

all : single block mpi

sync_src :
	rsync -a src/ $(BUILDDIR)

single : sync_src $(BUILDDIR)/single.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o single $(BUILDDIR)/single.cc $(OBJS) $(LIBRARIES)

block : sync_src $(BUILDDIR)/block.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o block $(BUILDDIR)/block.cc $(OBJS) $(LIBRARIES)

mpi : sync_src $(BUILDDIR)/mpi.cc $(OBJS)
	$(CC) $(CPPFLAGS) -o mpi $(BUILDDIR)/mpi.cc $(OBJS) $(LIBRARIES)

clean :
	rm -f single block mpi $(BUILDDIR)/*
