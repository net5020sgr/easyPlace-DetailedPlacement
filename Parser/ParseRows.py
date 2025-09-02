#BSD 3-Clause License
#
#Copyright (c) 2024, ASU-VDA-Lab
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from openroad import Tech, Design
import openroad as ord
from collections import defaultdict
from pathlib import Path
import os, argparse, sys



def load_design(design_name, design_path, verilog = False):
  tech = Tech()
  libDir = Path("./ASAP7/LIB/")
  lefDir = Path("./ASAP7/LEF/")
  techlefDir = Path("./ASAP7/techlef/")
  designDir = Path(design_path)
# ./after_buffered/aes
  # Read technology files
  libFiles = libDir.glob('*.lib')
  lefFiles = lefDir.glob('*.lef')
  
  # print("### libFiles ###")
  # for libFile in libFiles:
  #   print(libFile.as_posix())
  #   tech.readLiberty(libFile.as_posix())
  print("### techlefFile ###") 
  tech.readLef("%s/%s"%(techlefDir.as_posix(), "asap7_tech_1x_201209.lef"))
  print("### lefFiles ###")
  for lefFile in lefFiles:
    tech.readLef(lefFile.as_posix())
  design = Design(tech)

  print("### defFile ###")
  defFile = "%s/%s.def"%(designDir.as_posix(), design_name)
  design.readDef(defFile)


  return tech, design


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Output file path and the design name")
  parser.add_argument("--file_path", type = Path, default="./file", action = "store")
  parser.add_argument("--design_name", type = str, default="NaN", action = "store")
  parser.add_argument("--design_path", type=str, required=True)
  parser.add_argument("--dump_def", default = False, action = "store_true")
  parser.add_argument("--output_file", type = Path, default = None, action = "store")
  pyargs = parser.parse_args()
  
  tech, design = load_design(pyargs.design_name, pyargs.design_path, False)
  rows = design.getBlock().getRows()

  if pyargs.output_file:
    with open(pyargs.output_file, 'w') as f:
      f.write(str(len(rows)) + "\n")  
      for row in rows:
        # f.write(f"{row.getName()} {row.getOrigin()[0]} {row.getOrigin()[1]} {row.getOrigin()[0] + row.getSite().getWidth()}\n")
        f.write(f"{row.getName()} {row.getBBox().xMin()} {row.getBBox().yMin()} {row.getBBox().xMax()}\n")

  else:
    print(len(rows))
    for row in rows:
      print(row.getName(),row.getOrigin()[0],row.getOrigin()[1],row.getOrigin()[0] + row.getSite().getWidth())

  design.evalTclString("exit")

  