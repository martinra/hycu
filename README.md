HyCu (pronounce like Haiku)
===

Hyperelliptic curve counter implements counts points of hyperelliptic curves over finite fields.


INSTALL INSTRUCTIONS
---

HyCu is built using CMake, and provides four targets: single, threaded, mpi, and merger.

To build and install the threaded version HyCu with no further adjustments into PREFIX/bin use
~~~
cmake -D CMAKE_INSTALL_PREFIX=/home/martin/Documents/Mathematik/Projekte/hycu 
make
~~~

This assumes that the compiler is C++11 enabled by default. Prior GCC 6 you have to set the compiler flags
~~~
cmake -D CMAKE_CXX_FLAGS="-std=c++11" -D CMAKE_INSTALL_PREFIX=/home/martin/Documents/Mathematik/Projekte/hycu -D BuildThreaded=ON
~~~

CMake caches configuration variables, so you do not have to pass them again when you rebuild.

Check for available build options in CMakeList.txt.

USAGE
---

### Single

We assume that HyCu is install in one of the PATHs.

To compute with a single curve over a prime field, for testing purposes mostly, you can use hycu-single.
~~~
hycu-single 7 3 2 6 8 2
~~~
If HyCu was compiled with OpenCL support, you can use it by passing the flag opencl.
~~~
hycu-single --opencl 3001 7 531 63 3 152 476 1002
~~~

If HyCu was compiled with timing support, timing results will be output.

### Threaded and MPI

Threaded and MPI execution of HyCu is a two-step process. We generate unmerged data files in a result directory and then generate a single result file by
~~~
hycu-merger c result/q7g2 result/q7g2.hycu
~~~
This merges results for the path result/q7g2 into the file result/q7g2.hycu. The first option c is the store type, which currently must be c.

Running hycu on 2 threaded, using the configuration in config.yaml, and storing results into the path results:
~~~
hycu-threaded -n2 config.yaml results/
~~~

Running hycu with OpenMPI on 2 nodes with 16 threads each:
~~~
mpirun -n 2 --map-by ppr:1:node hycu-mpi -n 16 config.yaml results/
~~~

Skipping the number of threads lets HyCu use all available cores.

### Configuration file

The config file is a YAML file. Its first entry may be
~~~yaml
StoreType: EC
~~~
EC is the default value and stores results by ramification type, Hasse-Weil offsets, and curve count. It is currently the only value provided and thus you can skip this configuration option.

Sets of curves are described by the moduli section
~~~yaml
Moduli:
  - Prime: 7
    PrimeExponent: 1
    Genus: 2
    ResultPath: q7g2
    PackageSize: 2500
  
  - Prime: 11
    PrimeExponent: 1
    Genus: 2
    ResultPath: q11g2
    PackageSize: 2500
~~~
Fields here should be most self-explanatory. The prime and prime exponent give the size of the base field. Currently, prime exponent 1 is the only one that is tested. Package size is a technical parameter, which should not be chosen too small. A reasonable size for many cases would be q^2 or q^3, where q is the base field size.

### Store type EC

The results are stored as a text file with each line of the form:
~~~
1,1,1,2;-2,4:176
~~~
The first numbers give the degree of factors of the right hand side. The next ones give the offset of the number of points to the Hasse-Weil average. After the colon the curve count is provided.




