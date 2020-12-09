import fileinput
import os

def handle_file(file):
    with open(file, "r") as f :
        filedata = f.read()
    
    filedata = filedata.replace("stdcpp17", "stdcpplatest")
    
    lines = filedata.split("\n")
    filedata = ""
    for l in lines:
        if ".ixx" in l:
            l = l.replace("None Include", "ClCompile Include")
        filedata += l + "\n"
    
    with open(file, "w") as f:
        f.write(filedata)
  

for root, dirs, files in os.walk("./"):
    for file in files:
        if file.endswith(".vcxproj"):
            handle_file(os.path.join(root, file))
            