[![](glTF.png)](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)

# PBR To glTF 2.0 converter

pbr2gltf2 is a command line tool for converting PBR images to a glTF 2.0 material. The tool is detecting depending on the filename, which PBR information is stored. It swizzles the images and does reassign the channels to a glTF 2.0 texture. The tools stores the images plus a minimal, valid glTF 2.0 file containing the material, textures and images.  

Usage: `pbr2gltf2.exe [folder]`


## Software Requirements

* C/C++ 17 compiler e.g. gcc
* [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2021-03/r/eclipse-ide-cc-developers)  


## Supported PBR packages

* [https://cc0textures.com/](https://cc0textures.com/) PBR Materials For Anyone And Any Purpose!


## Import the generated glTF

Import the generated glTF in e.g. [Gestaltor](https://gestaltor.io/).  

