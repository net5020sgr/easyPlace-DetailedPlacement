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
  
  block = design.getBlock()
  nets = block.getNets()
  
  total_pins = 0
  net_outputs = []

  for net in nets:
      if net.isSpecial(): # Skip special nets like power/ground
          continue

      iterms = net.getITerms()
      bterms = net.getBTerms()
      
      degree = len(iterms) + len(bterms)
      if degree < 2: # Skip single-pin nets
          continue
          
      total_pins += degree
      
      net_info = []
      net_info.append(f"NetDegree : {degree} {net.getName()}")

      # Process instance terminals
      for iterm in iterms:
          inst = iterm.getInst()
          mterm = iterm.getMTerm()
          
          direction = mterm.getIoType()
          if direction == "INPUT":
              dir_char = "I"
          elif direction == "OUTPUT":
              dir_char = "O"
          else: # INOUT, FEEDTHRU, etc. -> map to B for Bookshelf
              dir_char = "B"

          # Get pin offset from instance origin. MTerm geometry is relative to cell origin.
          inst_origin = (inst.getBBox().xMin()+ inst.getBBox().xMax() )/ 2, (inst.getBBox().yMin()+ inst.getBBox().yMax() )/ 2
          pin_abs_pos = iterm.getAvgXY()
          x_offset = 0; #pin_abs_pos[1] - inst_origin[0]
          y_offset = 0; #pin_abs_pos[2] - inst_origin[1]
          
          pin_name = mterm.getName()
          inst_name = inst.getName()
          clean_inst_name = inst_name.replace('\\', '')

          net_info.append(f"  {clean_inst_name} {dir_char} : {x_offset:.0f} {y_offset:.0f} : 0.0 0.0 p_{pin_name}")

      # Process block terminals (ports)
      for bterm in bterms:
          direction = bterm.getIoType()
          if direction == "INPUT":
              dir_char = "I"
          elif direction == "OUTPUT":
              dir_char = "O"
          else: # INOUT, etc.
              dir_char = "B"

          # For bterms, the "instance" is the block itself. Location is absolute.
          x_loc, y_loc = 0, 0
          if len(bterm.getBPins()) > 0:
              # A bterm can have multiple pins (shapes). Taking center of first one.
              bbox = bterm.getBPins()[0].getBBox()
              x_loc = (bbox.xMin() + bbox.xMax()) / 2
              y_loc = (bbox.yMin() + bbox.yMax()) / 2
          
          bterm_name = bterm.getName()
          clean_bterm_name = bterm_name.replace('\\', '')
          # For bterms, instance name is the same as pin name. Offset is the absolute location.
          net_info.append(f"  {clean_bterm_name} {dir_char} : {x_loc:.0f} {y_loc:.0f} : 0.0 0.0 p_")
          
      net_outputs.append("\n".join(net_info))

  # Write to file
  if pyargs.output_file:
      with open(pyargs.output_file, 'w') as f:
          f.write("UCLA nets 1.0\n\n")
          f.write(f"NumNets : {len(net_outputs)}\n")
          f.write(f"NumPins : {total_pins}\n\n")
          for net_data in net_outputs:
              f.write(net_data + "\n")
  else:
      # Print to stdout if no file specified
      print("UCLA nets 1.0\n\n")
      print(f"NumNets : {len(net_outputs)}\n")
      print(f"NumPins : {total_pins}\n\n")
      for net_data in net_outputs:
          print(net_data)

  design.evalTclString("exit")

  