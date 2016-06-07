HyCu (pronounce like Haiku)

Hyperelliptic curve counter implements counts points of hyperelliptic curves over finite fields.


INSTALL INSTRUCTIONS
===

HyCu is built using scons, and provides four targets: single, threaded, mpi, and merger.

To build and install the threaded version HyCu with no further adjustments into PREFIX/bin use
~~~
scons threaded prefix=$PREFIX
~~~

This assumes that the compiler is C++11 enabled by default. Prior GCC 6 you have to set the compiler
~~~
scons threaded CXX="g++ -std=c++11" prefix=$PREFIX
~~~

Scons caches configuration variables, so you do not have to pass them again when you rebuild.

Available build options
---

- prefix: Installation, header, and library prefix.
- CXX: E.g "g++ -std=c++11" of "clang++ -std=c++11".
- cxx_include_path: Use only if C++ headers are in non-standard path.
- cxx_library_path: Use only if C++ libraries are in non-standard path.
- with_opencl: "y","yes" or "n","no".
- openmpicxx_path: Path to OpenMPI mpicxx, if that is used.
- mpi_compiler_flags: Extra flags for building with MPI version. This can be skipped if openmpicxx_path is set.
- mpi_linker_flags: Extra flags for building with MPI version. This can be skipped if openmpicxx_path is set.
- boost_include_path: Path to Boost headers if no in prefix/include path.
- boost_library_path: Path to Boost libraries if no in prefix/lib path.
- opencl_include_path: Path to OpenCL headers if no in prefix/include path.
- opencl_library_path: Path to OpenCL libraries if no in prefix/lib path.
- opencl_library: If the OpenCL library does not follow the Linux naming convention. Default is "OpenCL".
- flint_include_path: Path to Flint headers if no in prefix/include path.
- flint_library_path: Path to Flint libraries if no in prefix/lib path.
- gmp_include_path: Path to GMP/MPIR headers if no in prefix/include path.
- gmp_library_path: Path to GMP/MPIR libraries if no in prefix/lib path.
- yaml_cpp_include_path: Path to Yaml-Cpp headers if no in prefix/include path.
- yaml_cpp_library_path: Path to Yaml-Cpp libraries if no in prefix/lib path.


USAGE
===

Single
---

We assume that HyCu is install in one of the PATHs.

To compute with a single curve over a prime field, for testing purposes mostly, you can use hycu-single.
~~~
hycu-single 7 3 2 6 8 2
~~~
If HyCu was compiled with OpenCL support, you can use it by passing the flag opencl. To provide timings, use the flag time. For example, type
~~~
hycu-single --opencl --time 3001 7 531 63 3 152 476 1002
~~~

Threaded and MPI
---

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

Configuration file
---

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

Store type EC
---

The results are stored as a text file with each line of the form:
~~~
1,1,1,2;-2,4:176
~~~
The first numbers give the degree of factors of the right hand side. The next ones give the offset of the number of points to the Hasse-Weil average. After the colon the curve count is provided.




