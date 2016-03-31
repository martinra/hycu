env = Environment()
env["root_dir"] = Dir(".").abspath
Export("env")


env.SConscript("src/SConscript", variant_dir = "build")

installable = [
    "single"
  , "legacy"
  , "mpi"
  ]
for inst in installable:
  env.Alias( inst
           , env.Install("bin", "build/" + inst) )


env.SConscript("test/SConscript", variant_dir = "build_test")
Import("test")

test = env.Alias( "test", test, test[0].get_abspath() )
AlwaysBuild(test)


Default()
