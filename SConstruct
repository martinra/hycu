root_dir =  Dir(".").abspath
Export("root_dir")

SConscript("src/SConscript", variant_dir = "build")
Import("env")


installable = [
    "single"
  , "legacy"
  , "merge_isogeny_representatives"
  , "mpi"
  ]
for inst in installable:
  env.Alias( inst
           , env.Install("bin", "build/" + inst) )


env.SConscript("test/SConscript", variant_dir = "build_test")
Import("test")
Import("env_test")

test = env_test.Alias( "test", test, test[0].get_abspath() )
AlwaysBuild(test)


Default()
