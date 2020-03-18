# Plutonium
Plutonium is a learning project into creating a game engine from scratch. The development uses iterative development and has no clear goal other than learning.

# Building
1. Clone the repository
```bash
git clone git@github.com:Arzana/Plutonium.git
```

2. Check compiler versions<br/>
The project uses C++17 with the MSVC compiler, certain extensions are using throughout the project like unnamed structs.
It is also exclusive to Visual Studio and actively tested uing Visual Studio 2017

3. Installing Vulkan SDK<br/>
The full [Vulkan SDK](https://vulkan.lunarg.com/) is not needed to run the project, but glslangValidator is needed to compile shaders.

## Dependencies 
- [Dear ImGui](https://github.com/ocornut/imgui)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [stb image](https://github.com/nothings/stb/blob/master/stb_image.h)
- [std image write](https://github.com/nothings/stb/blob/master/stb_image_write.h)
- [std truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)
- [TinyXML2](https://github.com/leethomason/tinyxml2)
