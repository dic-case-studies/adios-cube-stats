import adios2

with adios2.open("casa.bp", "r") as fh:
   i = 0
   for fstep in fh:
      # inspect variables in current step
      step_vars = fstep.available_variables()

      # print variables information
      for name, info in step_vars.items():
            print("variable_name: " + name)
            for key, value in info.items():
               print("\t" + key + ": " + value)
            print("\n")

      # track current step
      step = fstep.current_step()
      if( step == 0 ):
         size_in = fstep.read("size")
      
      print("Fstep " , i , " Done")
      i += 1
         