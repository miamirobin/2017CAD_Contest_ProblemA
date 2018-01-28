#!/usr/bin/python

import sys

def mySplit(s, sep):
  ret = []
  tmp = ""
  for c in s:
    if c in sep:
      if len(tmp) > 0:
        ret.append(tmp)
      tmp = ""
    else:
      tmp += c
  if len(tmp) > 0:
    ret.append(tmp)
  return ret

def parseVerilog(filename):
  f = open(filename, "r")
  name = ""
  inputPin = []
  outputPin = []
  for line in f:
    line = line.strip()
    if line[0:6] == "module":
      name = mySplit(line[6:], " (")[0]
    elif line[0:5] == "input":
      inputPin += mySplit(line[5:], " ,;")
    elif line[0:6] == "output":
      outputPin += mySplit(line[6:], " ,;")
  f.close()
  return name, inputPin, outputPin

def getMatch(filename):
  f = open(filename, "r")
  matchOut = {}
  matchIn = {}
  state = None
  matchPin = None
  for line in f:
    line = line.strip()
    if line == "OUTGROUP":
      state = 1
    elif line == "INGROUP":
      state = 2
    elif line == "CONST0GROUP":
      state = 2
      matchPin = "1'b0"
    elif line == "END":
      state = None
      matchPin = None
    else:
      tokens = line.split(' ')
      assert len(tokens) == 3, line
      if tokens[0] == '1':
        assert tokens[1] == '+', "Match pin in cir1 is not positive"
        assert matchPin == None, "Multiple pin in cir1 in same group"
        matchPin = tokens[2]
      else:
        assert tokens[0] == '2'
        assert matchPin != None, "Match does not exist"
        if state == 1:
          assert tokens[2] not in matchOut, "Multiple match"
          matchOut[tokens[2]] = (tokens[1], matchPin)
        else:
          assert tokens[2] not in matchIn, "Multiple match"
          matchIn[tokens[2]] = (tokens[1], matchPin)
  f.close()
  return matchOut, matchIn

def getPhasePin((phase, name)):
  return ("" if phase == "+" else "!") + name

def genMiter(cir1File, cir2File, matchFile):
  cir1Name, cir1Input, cir1Output = parseVerilog(cir1File)
  cir2Name, cir2Input, cir2Output = parseVerilog(cir2File)
  matchOut, matchIn = getMatch(matchFile)
  # substitute cir2 input
  miterInput = list(cir1Input)
  mark = 1
  for i in range(len(cir2Input)):
    if cir2Input[i] in matchIn:
      cir2Input[i] = getPhasePin(matchIn[cir2Input[i]])
    else:
      pinName = "tmp_input_" + str(mark)
      cir2Input[i] = pinName
      miterInput.append(pinName)
      mark += 1
  # tmp wires
  cir1Dict = {}
  cir2Dict = {}
  wire = []
  xorList = []
  orStr = "or( miter_output"
  mark = 1
  for i in range(len(cir1Output)):
    pinName = "tmp_output_" + str(mark)
    cir1Dict[cir1Output[i]] = pinName
    wire.append(pinName)
    mark += 1
  for i in range(len(cir2Output)):
    pinName = "tmp_output_" + str(mark)
    cir2Dict[cir2Output[i]] = pinName
    wire.append(pinName)
    mark += 1
  for out2 in matchOut:
    (phase, out1) = matchOut[out2]
    pinName = "tmp_output_" + str(mark)
    wire.append(pinName)
    xorList.append("xor( " + pinName + " , " + cir2Dict[out2] + " , " + ("" if phase == "+" else "!") + cir1Dict[out1] + " );")
    orStr += (" , " + pinName)
    mark += 1
  orStr += " );"

  # write miter circuit
  f = open("miter.v", "w")
  f.write("module miter( " + " , ".join(miterInput) + " , miter_output );\n")
  f.write("input " + " , ".join(miterInput) + ";\n")
  f.write("output miter_output;\n")
  f.write("wire " + " , ".join(wire) + ";\n")
  f.write("module_1_" + cir1Name + " inst1( " + " , ".join(cir1Input + [cir1Dict[p] for p in cir1Output]) + " );\n")
  f.write("module_2_" + cir2Name + " inst2( " + " , ".join(cir2Input + [cir2Dict[p] for p in cir2Output]) + " );\n")
  for l in xorList:
    f.write(l + "\n")
  f.write(orStr + "\n")
  f.write("endmodule\n")
  
  f.write("\n\n")
  
  g = open(cir1File)
  for line in g:
    if line[0:6] == "module":
      line = "module module_1_" + line[6:].lstrip()
    f.write(line)
  g.close()

  f.write("\n\n")

  g = open(cir2File)
  for line in g:
    if line[0:6] == "module":
      line = "module module_2_" + line[6:].lstrip()
    f.write(line)
  g.close()

  f.close()

def main():
  if len(sys.argv) != 4:
    print "Usage: " + sys.argv[0] + " <cir1.v> <cir2.v> <match.out>"
    return
  genMiter(sys.argv[1], sys.argv[2], sys.argv[3])

if __name__ == "__main__":
  main()
