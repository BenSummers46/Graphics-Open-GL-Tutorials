# Graphics-Open-GL-Tutorials
Screenshots for the main features of my fantasy graphical scene made in C++ and OpenGL.\
YouTube Link: https://youtu.be/edW9Jexb6TY

## Water Shader
![image](https://user-images.githubusercontent.com/115077902/219670920-44add09e-910a-4ba0-bd11-d9053c6a5269.png)\
This effect was created by loading up a “blank” or flat heightmap and then manipulating the vertex positions of the mesh in the vertex shader to give the effect of waves. The wave effect is not static and moves with the input of a delta time uniform. Within the vertex shader I also needed to calculate the new vertex normal to be passed into the fragment shader so that the reflections on the water were affected by the new vertex position. In the fragment shader I also changed the colour of the fragment at the top of the waves to simulate foam, or the waves breaking.

## Multi-texture Blending
![image](https://user-images.githubusercontent.com/115077902/219671172-312ebc59-feaf-4155-8f41-f2a359428e53.png)
<br/>\
![image](https://user-images.githubusercontent.com/115077902/219671223-435a8b7b-21c0-4752-9f46-c6676e70d53a.png)\
For my terrain I am using multiple textures that I am blending within the fragment shader based on the height of the fragment on the mesh. In total I am blending a total of 4 textures (sand, grass, rocks and snow) along with their corresponding bump maps to allow for accurate lighting calculations on the texture.

## Post-Processing (Gaussian Blur)
![image](https://user-images.githubusercontent.com/115077902/219671555-11255a9d-9a8c-4dae-9e85-f7e9dbc7a983.png)\
I used the post processing effect to simulate the effect of being under the water within my scene. This was done with the use of multiple user generated buffers and textures. This effect can be toggled on and off with the key bind that was mentioned at the beginning of the document.

## Animated Mesh
![image](https://user-images.githubusercontent.com/115077902/219671675-7b8f6e5f-9fab-4164-afcb-74af5bbba784.png)\
The animated mesh is a spider that walks down the right flank of the heightmap. It has its own animation that continues to play throughout the scene’s life cycle. The mesh also has a shadow that moves with the mesh as the scene plays.

## Shadow Mapping
![image](https://user-images.githubusercontent.com/115077902/219671790-8489b6ab-e342-4715-9515-89ca6e8f4b22.png)\
I use a shadow map for each of the meshes that make up my scene apart from the water. This means that everything in the scene will cast a shadow on the terrain.

## Semi Transparent Forest
![image](https://user-images.githubusercontent.com/115077902/219672098-23db1aca-edf3-4ee5-9789-21a1635461b3.png)\
The forest within my scene is all part of a scene graph for efficient rendering of around 50 trees. They are all rendered based on their distance to the camera so that the transparency effect works for all of them. The effect is used to make it look like the trees are dying and are withering away from the top down. All the trees in the forest are also procedurally placed within the specified area to make it look more natural.



