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

## Then we parse the default shader project and generate build task for an other shader compiler:
for shader in root.iter(namespace + "FxCompile"):
    for shaderprofile in shader.iter(namespace + "ShaderType"):

        profile = shaderprofile.text
        name = shader.attrib["Include"]
        
        print (profile + ":   " + name)

        file.write("..\shadercompilers\dxc " + name + " -T ")

        
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

        file.write(" -D SHADER_MODEL_6 ");

        # filepath[0],fullflname[0] = os.path.splitext(name)[0]
        # filename[0],ext[0] = os.path.splitext(fullflname[0])

        file.write(" -flegacy-macro-expansion -Fo " + "shaderCSO/" + outputdir + "/"+ os.path.splitext(name)[0]  + ".cso ")

        ## Append to error log:
        file.write(" 2>>../build_HLSL6_errors.log \n")

        
file.write("cd .. \n")
    
file.close()
