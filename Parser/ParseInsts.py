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
from pathlib import Path
import argparse

def load_design(design_name, design_path):
  tech = Tech()
  libDir = Path("./ASAP7/LIB/")
  lefDir = Path("./ASAP7/LEF/")
  techlefDir = Path("./ASAP7/techlef/")
  designDir = Path(design_path)

  print("### techlefFile ###") 
  tech.readLef("%s/%s"%(techlefDir.as_posix(), "asap7_tech_1x_201209.lef"))
  print("### lefFiles ###")
  for lefFile in lefDir.glob('*.lef'):
    tech.readLef(lefFile.as_posix())
  design = Design(tech)

  print("### defFile ###")
  defFile = "%s/%s.def"%(designDir.as_posix(), design_name)
  design.readDef(defFile)
  return tech, design

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--design_name", type=str, required=True)
  parser.add_argument("--design_path", type=str, required=True)
  parser.add_argument("--output_nodes", type=str, required=True)
  parser.add_argument("--output_pl", type=str, required=True)
  pyargs = parser.parse_args()

  tech, design = load_design(pyargs.design_name, pyargs.design_path)
  block = design.getBlock()

  insts = block.getInsts()
  bterms = block.getBTerms()

#   dbu = tech.getDbUnitsPerMicron()

  def to_microns(val):
      return val / 1000

  # Write .nodes file
  with open(pyargs.output_nodes, 'w') as f_nodes:
    f_nodes.write("UCLA nodes 1.0\n\n")
    # Bookshelf NumNodes is total cells + terminals, NumTerminals is IOs
    f_nodes.write(f"NumNodes\t:\t{len(insts) + len(bterms)}\n")
    f_nodes.write(f"NumTerminals\t:\t{len(bterms)}\n\n")

    for inst in insts:
      master = inst.getMaster()
      clean_name = inst.getName().replace('\\', '')
      f_nodes.write(f"\t{clean_name}\t{master.getWidth()}\t{master.getHeight()}\n")

    for bterm in bterms:
        # BTerms don't have a direct width/height like a standard cell.
        width, height = 0, 0
        if len(bterm.getBPins()) > 0 and len(bterm.getBPins()[0].getBoxes()) > 0:
            bbox = bterm.getBPins()[0].getBoxes()[0]
            width = bbox.xMax() - bbox.xMin()
            height =    bbox.yMax() - bbox.yMin()
        clean_name = bterm.getName().replace('\\', '')
        f_nodes.write(f"\t{clean_name}\t{width}\t{height}\tterminal\n")

  # Write .pl file
  with open(pyargs.output_pl, 'w') as f_pl:
    f_pl.write("UCLA pl 1.0\n\n")
    for inst in insts:
      loc = inst.getOrigin()
      orient = inst.getOrient()
      clean_name = inst.getName().replace('\\', '')
      f_pl.write(f"{clean_name}\t{inst.getBBox().xMin()}\t{inst.getBBox().yMin()}\t: N\n")

    for bterm in bterms:
      if len(bterm.getBPins()) > 0:
          pin = bterm.getBPins()[0]
          loc = (pin.getBBox().xMin()+ pin.getBBox().xMax() )/ 2, (pin.getBBox().yMin()+ pin.getBBox().yMax() )/ 2
          status = " /FIXED_NI"
          clean_name = bterm.getName().replace('\\', '')
          f_pl.write(f"{clean_name}\t{loc[0]}\t{loc[1]}\t: N{status}\n")

  design.evalTclString("exit")
