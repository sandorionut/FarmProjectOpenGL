FarmProject 

The FarmProject  is a 3D graphical project developed in Visual Studio 2022  using the C++ programming language  and OpenGL libraries . This project showcases a virtual farm environment with various interactive elements and advanced graphical features. 
Key Features: 
- Interactive Objects : The scene includes multiple objects such as a barn, farmers, trees, animals, a house, and other interesting structures. Each object is meticulously designed to create an immersive farm environment.
- Collision Detection : The project implements collision detection to ensure that the "camera" cannot pass through objects in the scene. The camera is positioned at a fixed height and can be moved using the W, A, S, and D keys .
- Shovel Interaction : Attached to the camera is a shovel, which moves in sync with the camera. The coordinates of the shovel are dynamically calculated based on the camera's position.
- Dynamic Lighting : 
 - A directional light  represents the sun in the scene. You can move this light source using the J and L keys .
 - A positional light  is represented by a firepit, adding warmth and realism to the environment.
- Animated Farmer : One of the farmers in the scene moves forward and backward when the X key is pressed. The farmer rotates in predefined directions when reaching the edges of its path.
- Skybox and Ground : The skybox is implemented using the Cubemap technique , providing a realistic background. The ground plane complements the scene with detailed textures.
- Advanced Shadows : Shadows in the scene are dependent on the directional light (the sun) and other lighting components such as ambient, diffuse, and specular lighting. A Z-buffer  is used to track the depth of objects relative to the light source, ensuring accurate shadow representation.
- Object Textures : Each object in the scene has associated .obj and .mtl files for defining its geometry and texture mapping. Texture images (.png or .jpg) are applied to give each object a realistic appearance.
     
Controls: 
- Camera Movement : Use W, A, S, D  to navigate the scene.
- Light Control : Press J  and L  to adjust the position of the directional light (sun).
- Farmer Animation : Hold X to make the farmer move and rotate.
- Exit Application: Press ESC

Installation Instructions:
To set up and run the FarmProject  on your local machine, follow these steps: 
Prerequisites: 
- Visual Studio 2022 : Ensure you have Visual Studio 2022 installed.
- OpenGL Libraries : Install the necessary OpenGL libraries and dependencies. These are typically included with Visual Studio, but you may need to configure them manually.
- Git : Install Git if you don't already have it.
     
Steps to Install and Run:
Clone the Repository: Open a terminal and run the following command to clone the repository: git clone https://github.com/yourusername/FarmProject.git
 
Replace yourusername with your actual GitHub username. 

Navigate to the Project Directory: cd FarmProject
 
Open the Project in Visual Studio : 
- Launch Visual Studio 2022.
- Open the .sln file located in the project directory.
     
Install Dependencies : 
- If the project uses external libraries (e.g., OpenGL), ensure they are properly linked in the project settings.
- Configure the include directories and library paths in Visual Studio under Project Properties > C/C++ > General  and Linker > General .
     
Build the Project : 
- In Visual Studio, click Build > Build Solution  to compile the project.
     
Run the Application : 
- After a successful build, press F5  or click Debug > Start Debugging  to run the application.
     
     
