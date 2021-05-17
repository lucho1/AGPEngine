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
        - Info: For generic app information such as FPS or Memory Usage
        - Resources: To check the resources in the scene and their usage
        - Renderer: For app's renderer and hardware information and other options. From here, we can chose to show the lights debug spheres, the rendering type or the GBuffer texture to display.

    - At the center:
        - Scene: The viewport showing the scene. In here, we can move the camera
        - GBuffer View: To display the different textures in the GBuffer (Deferred Rendering must be checked in Renderer Options). From these very same options, we can chose to display the textures of Color, Normals, Position, Materials Smoothness and Depth to see different and useful color information. From here, the camera cannot be moved, it will display the scene from the camera as left in the Scene panel.


Finally, it is important to state the camera controls in order to use the Editor Camera in the Scene Panel and see the scene from different points of view. These are (MMB, RMB & LMB = Mid, Right & Left Mouse Buttons respectively):

    - Zoom In/Out:                  MMB Scroll or ALT + RMB Drag
    - Focus Center:                 F Key
    - Panning:                      MMB Drag or LMB + RMB (inverse pan!)
    - Orbit:                        ALT + LMB
    - Look Around:                  RMB
    - FPS Movement:                 RMB + WASDEQ Keys
    - Increase/Decrease FPS Speed:  RMB + MMB Scroll
    - Double FPS Speed:             Shift Key while using FPS Movement


 # Features
    - Materials Modificator Values & Textures Load/Unload (as resources!)
    - Normal Mapping
    - Deferred & Forward Rendering (chose between both and compare speeds)
    - Camera Movement & Modification
    - Custom Lighting: Add and modify lights
    - Models loading from code
    - GBuffer view of different textures (including Materials Smoothness Values)
    - Debug Spheres Drawing for point lights
    - Nice UI with File Dialogues to load textures :D
    - Own internal systems
    - OpenGL 4.6 with GLAD 4.6
    - Memory Metrics and FPS count :D
    - Uniform and Shader-Uniform Buffers (for camera and lighting)
    - Shader Interface Blocks
    - Scene Entities Modification
    - Extensive OpenGL Debugger

Note: There are many commits from Lucho Suaya from March-April because we still didn't knew that it could be done in couples, then when we agreed to go together, that's why Joan made the biggest part of deferred rendering.