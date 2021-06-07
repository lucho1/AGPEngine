# AGPEngine
 Little rendering engine for Advanced Graphics Programming (AGP) subject of the Bachelor Degree in Videogames Design & Development at CITM, UPC.

 Made by Lucho Suaya and Joan Marin.

# Controls
The engines have several panels through which to interact with the scene:

    - At the left:
        - Entities: To modify the transformation, name and the material values of each entity loaded in the scene
        - Camera: To modify the editor camera values such as its movement, speed, ...
        - Lights: To modify the values of the scene lights such as position, color, ... They can also be added and deleted. To modify the MaxLights in the Scene, modify the value "s_MaxLights" in the file "Source/Renderer/Utils/RendererUtils.h"
    
    - At the right:
        - Info: For generic app information such as FPS or Memory Usage.
        - Resources: To check the resources in the scene and their usage
        - Renderer: For app's renderer information, hardware information and other options. From here, we can chose to show the lights debug spheres, the rendering type or the GBuffer texture to display. Also, from here we can modify a couple of values of the bloom effect as well as visualize it. In addition, some settings for the skybox are available (changing the textures might be a little tricky, check 'Graphic Features' section).

    - At the center:
        - Scene: The viewport showing the scene. In here, we can move the camera
        - GBuffer View: To display the different textures in the GBuffer (Deferred Rendering must be checked in Renderer Options). From these very same options, we can chose to display the textures of Color, Normals, Position, Materials Smoothness and Depth to see different and useful color information. From here, the camera cannot be moved, it will display the scene from the camera as left in the Scene panel.


Finally, it is important to state the camera controls in order to use the Editor Camera in the Scene Panel. These are the next (MMB, RMB & LMB = Mid, Right & Left Mouse Buttons respectively):

    - Zoom In/Out:                  MMB Scroll or ALT + RMB Drag
    - Focus Center:                 F Key
    - Panning:                      MMB Drag or LMB + RMB (inverse pan!)
    - Orbit:                        ALT + LMB
    - Look Around:                  RMB
    - FPS Movement:                 RMB + WASDEQ Keys
    - Increase/Decrease FPS Speed:  RMB + MMB Scroll
    - Double FPS Speed:             Shift Key while using FPS Movement


# Features
    - Textures Load/Unload (as resources!)
    - Camera Movement & Modification
    - Models loading from code
    - Nice UI with File Dialogues to load textures :D
    - Own internal systems (we recommend to check the function Renderer::LoadDefaultTextures() as in there we create default textures with an only pixel from a HEX code)
    - OpenGL 4.6 with GLAD 4.6
    - Memory Metrics and FPS count :D
    - Uniform and Shader-Uniform Buffers (for camera and lighting)
    - Shader Interface Blocks
    - Scene Entities Modification
    - Extensive OpenGL Debugger

Note: There are many commits from Lucho Suaya from March-April because we still didn't knew that it could be done in couples, then when we agreed to go together, that's why Joan made the biggest part of deferred rendering.

# Graphics Features
    - Materials Customization featuring Normal Mapping with Bumpiness value modification and Bump Mapping with Heightscale & Layers values modification
    - Bloom Effect
    - Skybox (to change its textures, *sustitute* the images inside "Resources/Textures/Skybox" with others with the *same name* and restart the engine)
    - Deferred & Forward Rendering (chose between both)
    - GBuffer view of different textures (including Materials Smoothness Values)
    - Custom Lighting: Add and modify lights
    - Debug Spheres Drawing for point lights

Note that the options of this features may be modified from the engine panels. For instance, the normal mapping is a technique supported by each material, if it has a normal texture, it will be enabled, otherwise, it won't. Also, the deferred rendering can be displayed and observed from the panel aside the scene viewport and the gbuffer texture rendered can be chosen from the rendering panel at the right.

# Shaders
The engine runs, mainly, with three shaders: the Lighting Shader, the Textured Shader and the Deferred Lighting Shader. They can be located in "Resources/Shaders".

The Lighting Shader is a shader supporting basic materials features (albedo texture/color, smoothness...) plus normal mapping and scene lighting. It is used for forward rendering and makes use of certain shader resources like interface blocks or both uniform and shader-storage uniform buffers.

The Texture Shader is similar but it doesn't supports lighting, it fills the textures within the gBuffer for the Deferred Rendering. Although, it's core is more or less the same, since it supports materials basic features plus normal mapping and uses uniform buffer objects and interface blocks (not shader-storage buffers because there's no lights in it).

Then, the Deferred Lighting Shader is the one used for lighting in Deferred Rendering, it gets the gBuffer textures and builds up the final scene appearance by lighting it. As for it, it doesn't has material information, but as it has lighting information, it uses shader-storage uniform buffers and uniform ones as well (for camera data).

We can think of the Lighting Shader as the Forward Rendering Shader and the other two as the first and second passes of the Deferred Rendering.

On the other side, the Skybox Shader handles the rendering of the Skybox, which is drawn before any object in the scene, both for forward and deferred rendering.

Finally, we have the shaders used for the bloom effect. They are 2 shaders, one used to blur the brighter parts of the image resulting from rendering all the scenes (from the previous shaders), and another one to blend the resulting blurred image with the image resulting from rendering. There are a couple of values in the Rendering Panel that allow to modify and visualize the bloom effect, but some settings can be also modified from this shaders, for instance, the matrix of weights used to blur.

# SCREENSHOTS
## Bloom Effect
Effect Active:
![Bloom Active](/ReadmeImages/Bloom.PNGraw=true "Bloom Active")

Bloom Brightness Pass:
![Bloom Brightness](ReadmeImages/BloomBrightImg.png)

Blurry Pass (Bloom Blur):
![Bloom Blur](ReadmeImages/BloomBlurred.png)

Effect Inactive:
![Bloom Inactive](ReadmeImages/BloomNO.png)

## Normal & Relief/Bump/Parallax Mapping
Active, different values on each image:
![Normal & Bump Mapping](ReadmeImages/NormalBumpMapImg.png)
![Normal & Bump Mapping 2](ReadmeImages/NormalBumpMapImg2.png)

Inactive:
![Normal & Bump Mapping Inactive](ReadmeImages/NormalBumpMapImgNO.png)

## Skybox
![Skybox](ReadmeImages/Skybox_Gif.mp4)

## G Buffer

![Color buffer](ReadmeImages/G_Buffer_Color.png)
![Depth buffer](ReadmeImages/G_Buffer_Depth.png)
![Normal buffer](ReadmeImages/G_Buffer_Normal.png)
![Position buffer](ReadmeImages/G_Buffer_Position.png)

## Deferred rendering

![Deferred rendering](ReadmeImages/Deferred_Rendering.png)
![Forward rendering](ReadmeImages/Forward_Rendering.png)
