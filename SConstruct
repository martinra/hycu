import os


## define all variables for the build
opts = Variables(['scons_variables.cache'], ARGUMENTS)

opts.Add( "CXX", None )
opts.Add( PathVariable("libboost_path", "path to libboost", "/usr/lib/", PathVariable.PathIsDir) )
opts.Add( PathVariable("libcl_path", "path to libcl", "/usr/lib/", PathVariable.PathIsDir) )
opts.Add( PathVariable("libflint_path", "path to libflint", "/usr/lib/", PathVariable.PathIsDir) )
opts.Add( PathVariable("libgmp_path", "path to libgmp", "/usr/lib/", PathVariable.PathIsDir) )
opts.Add( PathVariable("libyamlcpp_path", "path to libyaml-cpp", "/usr/lib/", PathVariable.PathIsDir) )
opts.Add( PathVariable("mpicxx_path", "path to configuration programm mpicxx", "mpic++", PathVariable.PathIsFile) )
opts.Add( PathVariable("prefix", "prefix for installation", "/usr/", PathVariable.PathIsDir) )


## we construct a main environment, from which all subsystems can diverge
env = Environment(variables = opts)

## cache variables for next build
opts.Save('scons_variables.cache', env)



if not GetOption("clean"):
  conf = env.Configure()

  ## find working compiler
  if not env["CXX"]:
    for CXX in [ "clang++", "g++" ]:
      if conf.CheckProg(CXX):
        env["CXX"] = CXX
        break
    else:
      print( "No known compiler is installed" )
      Exit(1)
  if not conf.CheckCXX():
    print( "Invalid compiler {}".format(env["CXX"]) )
    Exit(1)


  ## check libaries
  libs_and_paths = [ (lib, env["lib" + lib + "_path"])
                     for lib in [ "cl", "flint", "gmp" ] ]
  libs_and_paths.append( ("yaml-cpp", env["libyamlcpp_path"]) )
  libs_and_paths += [ (lib, env["libboost_path"])
                      for lib in ["boost_filesystem", "boost_system"] ]
  for (lib,path) in libs_and_paths :
    conf.env.AppendUnique( LIBPATH = [path] )
    if not conf.CheckLib(lib, language="C++"):
      print( "Library {} not found".format(lib) )
    conf.env.Append( LIBS = [lib] )

  env = conf.Finish()
   
  ## standard compiler arguments
  env.Append(
## debug:
      CXXFLAGS = "-std=c++11 -g -pthread" # -O2
    , CPPPATH  = [ Dir("#/src") ]
## debug:
    , LINKFLAGS = "-pthread"
    )


env_merger = env.Clone()
if not GetOption("clean"):
  conf = env_merger.Configure()

  ## check libaries
  libs_and_paths = [ (lib, env["libboost_path"])
                     for lib in ["boost_filesystem", "boost_system"] ]
  for (lib,path) in libs_and_paths :
    conf.env.AppendUnique( LIBPATH = [path] )
    if not conf.CheckLib(lib, language="C++"):
      print( "Library {} not found".format(lib) )
    conf.env.Append( LIBS = [lib] )

  env_merger = conf.Finish()


env_mpi = env.Clone()
if not GetOption("clean"):
  conf = env_mpi.Configure()

  ## mpi compiler flags
  env_mpi.ParseConfig(env_mpi["mpicxx_path"] + " --showme:compile")
  env_mpi.ParseConfig(env_mpi["mpicxx_path"] + " --showme:link")

  ## check libaries
  libs_and_paths = [ (lib, env["libboost_path"])
                     for lib in ["boost_filesystem", "boost_mpi", "boost_serialization", "boost_system"] ]
  for (lib,path) in libs_and_paths :
    conf.env.AppendUnique( LIBPATH = [path] )
    if not conf.CheckLib(lib, language="C++"):
      print( "Library {} not found".format(lib) )
    conf.env.Append( LIBS = [lib] )

  env_mpi = conf.Finish()


env_test = env.Clone()
if not GetOption("clean"):
  conf = env_test.Configure()

  ## check libaries
  libs_and_paths = [ (lib, env["libboost_path"])
                     for lib in ["boost_unit_test_framework"] ]
  for (lib,path) in libs_and_paths :
    conf.env.AppendUnique( LIBPATH = [path] )
    if not conf.CheckLib(lib, language="C++"):
      print( "Library {} not found".format(lib) )
    conf.env.Append( LIBS = [lib] )

  env_test = conf.Finish()

  env_test.Append( CPPPATH = [Dir("#/test").abspath] )
  env_test["ENV"]["LD_LIBRARY_PATH"] = ":".join([
      env_test["libcl_path"]
    , env_test["libflint_path"]
    , os.environ.get("LD_LIBRARY_PATH", "")
    ])



env.SConscript("src/SConscript", variant_dir = "build", exports = ["env", "env_merger", "env_mpi"] )
env.SConscript("test/SConscript", variant_dir = "build_test", exports = "env_test")


Default()
