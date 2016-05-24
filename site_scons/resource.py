import os
from SCons.Variables.PathVariable import PathVariable

def AddResourceOptions(opts, resources):
  added_header_options = set()
  added_library_options = set()
  for (lib, header, package) in resources:
    if lib is None and header is None:
      raise ValueError()

    if lib is not None and package not in added_library_options:
      
      opts.Add( PathVariable( "{}_library_path".format(package), "path to library for {}".format(package),
                              None, PathVariable.PathIsDir ) )
      added_library_options.add( package )

    if header is not None and package not in added_header_options:
      opts.Add( PathVariable( "{}_include_path".format(package), "path to headers for {}".format(package),
                              None, PathVariable.PathIsDir ) )
      added_header_options.add( package )

def LibsAndHeaders(env, resources):
  libs_and_headers = {}
  for (libs, headers, package) in resources:
    if libs is not None:
      if isinstance(libs, tuple):
        name = libs[0]
      else:
        name = libs
    elif headers is not None:
      if isinstance(headers, list):
        name = headers[0]
      else:
        name = headers
    else:
      print "libs and headers are none for package {}".format(package)
      exit(1)
    libs_and_headers[name] = {}


    library_path = "{}_library_path".format(package)
    if library_path in env:
      libs_and_headers[name]["lib_path"] = env[library_path]
    else:
      libs_and_headers[name]["lib_path"] = ""
  
    if libs is not None:
      if isinstance(libs, tuple):
        libs = libs[1] + [libs[0]]
      else:
        libs = [ libs ]
      libs_and_headers[name]["libs"] = libs


    include_path = "{}_include_path".format(package)
    if include_path in env:
      libs_and_headers[name]["include_path"] = env[include_path]
    else:
      libs_and_headers[name]["include_path"] = ""

    if headers is not None:
      libs_and_headers[name]["headers"] = headers

  return libs_and_headers

def AddResource(conf, name, libs_and_headers):
  resource = libs_and_headers[name]

  if "libs" in resource:
    conf.env.PrependUnique( LIBPATH = [resource["lib_path"]] )
  if "headers" in resource:
    conf.env.PrependUnique( CPPPATH = [resource["include_path"]] )

  if "libs" in resource:
    conf.env.AppendUnique( LIBS = resource["libs"] )

  ## SConf is disfunctional wiht regard to including several libraries
  ## at once. We add libraries without checking
  ## on compilers for which the order of libraries does matter,
  ## the following implementation will not work
##  if "libs" in resource and "headers" in resource:
##    print "libs", resource["libs"]
##    check = conf.CheckLibWithHeader(resource["libs"], resource["headers"], language="C++")
##    checked = str(resource["libs"]) + " | " + str(resource["headers"])
##  elif "libs" in resource:
##    check = conf.CheckLib(resource["libs"], language="C++")
##    checked = str(resource["libs"])
##  elif header in resource:
##    check = conf.CheckHeader(resource["headers"], language="C++")
##    checked = str(resource["headers"])
##  if not check:
##    print( "Resource {} not found".format(checked))
##    exit(1)
