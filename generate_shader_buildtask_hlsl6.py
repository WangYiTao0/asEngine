import os
import xml.etree.ElementTree as ET

tree = ET.parse('asEngine_Shaders/asEngine_Shaders.vcxproj')
root = tree.getroot()

## Hardcode visual studio namespace for now...
namespace = "{http://schemas.microsoft.com/developer/msbuild/2003}"

file = open("build_HLSL6.bat", "w")

outputdir = "hlsl6"

## First, set error log output:
file.write("2>build_HLSL6_errors.log cls \n")

## Then, ensure that we have the output directory:
file.write("mkdir asEngine_Shaders/shaderCSO \n")
file.write("cd asEngine_Shaders/shaderCSO \n")
file.write("mkdir " + outputdir + "\n")
file.write("cd .. \n")
file.write("cd .. \n")
file.write("cd 3rdPart \n")
file.write("cd shadercompilers \n")

## Then we parse the default shader project and generate build task for an other shader compiler:
for shader in root.iter(namespace + "FxCompile"):
    for shaderprofile in shader.iter(namespace + "ShaderType"):

        profile = shaderprofile.text
        name = shader.attrib["Include"]
        
        print (profile + ":   " + name)
# -T <profile>            Set target profile.
        file.write("dxc " + " ../../asEngine_Shaders/" + name + " -T ")

        
        if profile == "Vertex":
            file.write("vs")
        if profile == "Pixel":
            file.write("ps")
        if profile == "Geometry":
            file.write("gs")
        if profile == "Hull":
            file.write("hs")
        if profile == "Domain":
            file.write("ds")
        if profile == "Compute":
            file.write("cs")

        file.write("_6_1 ")
#-D <value>              Define macro
        file.write(" -D SHADER_MODEL_6 ");

        filepath,fullflname = os.path.split(name)
        filename,ext = os.path.splitext(fullflname)
#  -flegacy-macro-expansion
#  Expand the operands before performing token-pasting operation (fxc behavior)
        file.write(" -flegacy-macro-expansion -Fo " + "../../asEngine_Shaders/shaderCSO/" + outputdir + "/"+ filename + ".cso ")

        ## Append to error log:
        file.write(" 2>>../build_HLSL6_errors.log \n")

        
file.write("cd .. \n")
    
file.close()
