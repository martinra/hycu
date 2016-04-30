import os
from resource import *


## define all variables for the build
opts = Variables(['scons_variables.cache'], ARGUMENTS)

opts.Add( "CXX", None )
opts.Add( PathVariable("prefix", "prefix for installation", "/usr/", PathVariable.PathIsDir) )
opts.Add( PathVariable("mpicxx_path", "path to configuration programm mpicxx", "/usr/bin/mpic++", PathVariable.PathIsFile) )


resources = \
  [ ( "boost_filesystem", "boost/filesystem.hpp", "boost" )
  , ( "boost_mpi", "boost/mpi.hpp", "boost" )
  , ( "boost_serialization", "boost/serialization/serialization.hpp", "boost" )
  , ( "boost_system", None, "boost" )
  , ( "boost_unit_test_framework", "boost/test/unit_test.hpp", "boost" )
  , ( "OpenCL", "CL/cl.h", "opencl" )
  , ( "flint", "flint/flint.h", "flint" )
  , ( "gmp", "gmp.h", "gmp" )
  , ( "yaml-cpp", "yaml-cpp/yaml.h", "yaml_cpp" )
  ]
AddResourceOptions(opts, resources)

opts.Add( "opencl_library", "library for opencl", "OpenCL" )

## we construct a main environment, from which all subsystems can diverge
env = Environment(variables = opts)

## cache variables for next build
opts.Save('scons_variables.cache', env)

## list of libraries and headers attached to resources
libs_and_headers = LibsAndHeaders(env, resources)

## opencl requires extra configuration
libs_and_headers["OpenCL"]["lib"] = env["opencl_library"]


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

  conf.env["CXXPATH"] = [ env["prefix"] + "/include" ]
  conf.env["LIBPATH"] = [ env["prefix"] + "/lib" ]

  ## standard compiler arguments
  conf.env.Append(
      CXXFLAGS  = "-std=c++11 -O2 -pthread"
    , CPPPATH   = [ Dir("#/src") ]
    , LINKFLAGS = "-pthread"
    )

  for resource in [ "boost_system", "boost_filesystem",
							      "OpenCL", "flint", "gmp", "yaml-cpp" ]:
    AddResource(conf, resource, libs_and_headers)

  env = conf.Finish()
   


env_mpi = env.Clone()
if not GetOption("clean"):
  conf = env_mpi.Configure()

  ## mpi compiler flags
  env_mpi.ParseConfig(env_mpi["mpicxx_path"] + " --showme:compile")
  env_mpi.ParseConfig(env_mpi["mpicxx_path"] + " --showme:link")

  ## check libaries
  for resource in [ "boost_mpi", "boost_serialization" ]:
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
