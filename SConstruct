import os
from resource import *


## define all variables for the build
opts = Variables(['scons_variables.cache'], ARGUMENTS)

opts.Add( "CXX", None )
opts.Add( PathVariable("cxx_include_path", "path to C++ STL include files", None, PathVariable.PathIsDir) )
opts.Add( PathVariable("cxx_library_path", "path to C++ library", None, PathVariable.PathIsDir) )

opts.Add( PathVariable("prefix", "prefix for installation", "/usr/", PathVariable.PathIsDir) )

opts.Add( "with_opencl", default="y" )
opts.Add( "opencl_library", "library for opencl", "OpenCL" )

opts.Add( "openmpicxx_path", "path to OpenMPI mpicxx", None )
opts.Add( "mpi_compiler_flags", "extra compiler flags for MPI", None )
opts.Add( "mpi_linker_flags", "extra linker flags for MPI", None )


resources = \
  [ ( ( "boost_filesystem", [ "boost_system" ] ), "boost/filesystem.hpp", "boost" )
  , ( ( "boost_mpi", [ "boost_serialization" ] ), "boost/mpi.hpp", "boost" )
  , ( "boost_program_options", "boost/program_options.hpp", "boost" )
  , ( "boost_unit_test_framework", "boost/test/unit_test.hpp", "boost" )
  , ( "OpenCL", "CL/cl.h", "opencl" )
  , ( "flint", "flint/flint.h", "flint" )
  , ( "gmp", "gmp.h", "gmp" )
  , ( "yaml-cpp", "yaml-cpp/yaml.h", "yaml_cpp" )
  ]
AddResourceOptions(opts, resources)


## we construct a main environment, from which all subsystems can diverge
env = Environment(variables = opts)
for (key,value) in os.environ.items():
  if key not in env:
    try:
      env['ENV'][key] = value
    except:
       pass

## cache variables for next build
opts.Save('scons_variables.cache', env)

## list of libraries and headers attached to resources
libs_and_headers = LibsAndHeaders(env, resources)

## opencl requires extra configuration
libs_and_headers["OpenCL"]["lib"] = env["opencl_library"]


if env["with_opencl"] in ["y", "yes"]:
  env["with_opencl"] = True
  env.Append( CPPDEFINES = "WITH_OPENCL" )
elif env["with_opencl"] in ["n", "no"]:
  env["with_opencl"] = False
else:
  print( "Invalid value {} of option with_opencl".format(env["with_opencl"]) )
  Exit(1)
  

if not GetOption("clean"):
  conf = env.Configure()

  ## find working compiler
  if not env["CXX"]:
    for CXX in [ "clang++", "g++" ]:
      if conf.CheckProg(CXX):
        conf.env["CXX"] = CXX
        break
    else:
      print( "No known compiler is installed" )
      Exit(1)
  if not conf.CheckCXX():
    print( "Invalid compiler {}".format(conf.env["CXX"]) )
    Exit(1)

  conf.env["CPPPATH"] = [ env["prefix"] + "/include" ]
  conf.env["LIBPATH"] = [ env["prefix"] + "/lib" ]

  if "cxx_include_path" in env:
    conf.env.AppendUnique( CPPPATH = env["cxx_include_path"] )
  if "cxx_library_path" in env:
    conf.env.AppendUnique( CPPPATH = env["cxx_library_path"] )

  ## standard compiler arguments
  conf.env.Append(
      CXXFLAGS  = "-O2 -pthread"
    , CPPPATH   = [ Dir("#/src") ]
    , LINKFLAGS = "-pthread"
    )

  for resource in [ "boost_program_options",
                    "boost_filesystem",
                    "flint", "gmp", "yaml-cpp" ]:
    AddResource(conf, resource, libs_and_headers)
  if env["with_opencl"]:
    AddResource(conf, "OpenCL", libs_and_headers)

  env = conf.Finish()
   


env_mpi = env.Clone()
if not GetOption("clean"):
  conf = env_mpi.Configure()

  if "openmpicxx_path" in env_mpi:
    env_mpi.ParseConfig(env_mpi["openmpicxx_path"] + " --showme:compile")
    env_mpi.ParseConfig(env_mpi["openmpicxx_path"] + " --showme:link")

  if "mpi_compiler_flags" in env_mpi:
    conf.env.AppendUnique( CXXFLAGS = env_mpi["mpi_compiler_flags"] )
  if "mpi_linker_flags" in env_mpi:
    conf.env.AppendUnique( LINKFLAGS = env_mpi["mpi_linker_flags"] )

  ## check libaries
  for resource in [ "boost_mpi" ]:
    AddResource(conf, resource, libs_and_headers)

  env_mpi = conf.Finish()


env_test = env.Clone()
if not GetOption("clean"):
  conf = env_test.Configure()

  for resource in [ "boost_unit_test_framework" ]:
    AddResource(conf, resource, libs_and_headers)

  env_test = conf.Finish()

  env_test.Append( CPPPATH = [Dir("#/test").abspath] )
  env_test["ENV"]["LD_LIBRARY_PATH"] = ":".join([
      libs_and_headers["OpenCL"]["lib_path"]
    , libs_and_headers["flint"]["lib_path"]
    , os.environ.get("LD_LIBRARY_PATH", "")
    ])



env.SConscript("src/SConscript", variant_dir = "build", exports = ["env", "env_mpi"] )
env.SConscript("test/SConscript", variant_dir = "build_test", exports = "env_test")


Default()
