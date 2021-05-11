[![](glTF.png)](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)

# PBR To glTF 2.0 converter

pbr2gltf2 is a command line tool for converting PBR images to a glTF 2.0 material. The tool is detecting depending on the filename, which PBR information is stored. It swizzles the images and does reassign the channels to a glTF 2.0 image. The tool stores the images plus a minimal, valid glTF 2.0 file containing the required material, textures and images.  

Usage: `pbr2gltf2.exe folder [-m 1.0 -r 1.0 -n true]`

`-m 1.0` Default metallic factor value, if no metallic image was found.  
`-r 1.0` Default roughness factor value, if no roughness image was found.  
`-n true` Keep original normal image data.  


## Software Requirements

* C/C++ 17 compiler e.g. gcc or Visual C++
* [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2021-03/r/eclipse-ide-cc-developers) or  
* [CMake](https://cmake.org/)  


## Supported PBR packages

* [https://cc0textures.com/](https://cc0textures.com/) PBR Materials For Anyone And Any Purpose!
* [https://www.cgbookcase.com/](https://www.cgbookcase.com/) Free PBR Textures


## Import the generated glTF

Import the generated glTF in e.g. [Gestaltor](https://gestaltor.io/) and reuse in your scene.  

A short tutorial can be found here: [Gestaltor - How to rescale a texture](https://docs.gestaltor.io/#rescale-a-texture).  

