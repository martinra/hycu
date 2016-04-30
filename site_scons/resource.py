import os
from SCons.Variables.PathVariable import PathVariable

def AddResourceOptions(opts, resources):
  added_resource_options = set()
  for (lib, header, package) in resources:
    if lib is None and header is None:
      raise ValueError()

    if package in added_resource_options: continue
    added_resource_options.add(package)

    if lib is not None:
      opts.Add( PathVariable( "{}_library_path".format(package), "path to library for {}".format(package),
                              "/usr/lib", PathVariable.PathIsDir ) )
    if header is not None:
      opts.Add( PathVariable( "{}_include_path".format(package), "path to headers for {}".format(package),
                              "/usr/include", PathVariable.PathIsDir ) )

def LibsAndHeaders(env, resources):
  libs_and_headers = {}
  for (lib, header, package) in resources:
    if lib is not None:
      name = lib
    else:
      name = header
    libs_and_headers[name] = {}
  
    if lib is not None:
      libs_and_headers[name]["lib_path"] = env["{}_library_path".format(package)]
      libs_and_headers[name]["lib"] = lib
    else:
      libs_and_headers[name]["lib_path"] = ""
  
    if header is not None:
      libs_and_headers[name]["include_path"] = env["{}_include_path".format(package)]
      libs_and_headers[name]["header"] = header
    else:
      libs_and_headers[name]["include_path"] = ""

  return libs_and_headers

def AddResource(conf, name, libs_and_headers):
  resource = libs_and_headers[name]

  conf.env.AppendUnique( CXXPATH = [resource["include_path"]],
                         LIBPATH = [resource["lib_path"]] )

  if "lib" in resource and "header" in resource:
    check = conf.CheckLibWithHeader(resource["lib"], resource["header"], language="C++")
    checked = resource["lib"] + " | " + resource["header"]
  elif "lib" in resource:
    check = conf.CheckLib(resource["lib"], language="C++")
    checked = resource["lib"]
  elif header in resource:
    check = conf.CheckHeader(resource["header"], language="C++")
    checked = resource["header"]
  if not check:
    print( "Resource {} not found".format(checked))
    exit(1)

  if "lib" in resource:
    conf.env.Append( LIBS = [resource["lib"]] )
