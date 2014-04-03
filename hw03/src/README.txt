HOMEWORK 3: RAY TRACING, RADIOSITY, & PHOTON MAPPING

NAME:  < Max Espinoza >


TOTAL TIME SPENT:  < 40+ hours? I'm not even sure anymore >

Please estimate the number of hours you spent on this assignment.


COLLABORATORS: 
You must do this assignment on your own, as described in the 
Academic Integrity Policy.  If you did discuss the problem or errors
messages, etc. with anyone, please list their names here.

Everyone in the lab! :)



RAYTRACING:
< insert comments on the implementation, known bugs, extra credit >

	=====Side comments=====
	Implementation was elegant and reminded me a lot of physics. Just a comment on my visualization, I altered the colors of the raytree to better suit my understand. Here's a quick run down: Green (I am shadow) White (I am not a shadow) Red( I Hit Something and Bounced and/or am the main ray).

	==== In your README.txt discuss how you generated those random points (on the light source and within the pixel). ===
	For generating points in the pixel I at one point used stratified sampling, however I deleted that portion of the code and moved on because it didn't make any noticeable difference aside from slow down my ray tracing. Now I use a jitter method, where I start at the center of a pixel and jitter it either left or right with in some range and run the ray tracer through there collecting samples to average later. As for the random points on the light source I used the provided randomPoint function in the Face class.

	==== Extra credit ====
	I failed at water simulation last homework, at least the 3D one, so that stuck with me. So I tried to make a static water like rendering. I noticed the pebbles were a texture and used that to make a 3d water molecule! I altered reflections on the texture and took way to long aligning those spheres.
	./render -input src/textured_water_molecule.obj -num_bounces 1 -num_shadow_samples 9 -num_antialias_samples 9

	==== Bugs ====
	No known bugs at the time. However feel free to let me know if it crashes! 



RADIOSITY:
< insert comments on the implementation, known bugs, extra credit >

	=====Side comments=====
	I used progressive refinement to solve the radiosity matrix like I think everyone else probably did. I did find/use an altered version of the form factor equation (Wallace et al. 1989) that I think gave me more visually appealing results (very slim difference). Besides that had trouble making sure my form factors were correct, so I made a small visualizer to show you if your normals/rays are correct when you calculated them for the form factor.


	==== In your README.txt file, discuss the performance quality tradeoffs between the number of patches and the complexity of computing a single form factor. ====
	Complexity of a single form factor given you have n number of patches will be n^2 and this is significant slowdown when dealing with high resolution meshes. 90% of the time running radiosity in the form factor computation, the rest as in actually solving for lighting simulations can happen interactively through progressive refinement. For example in the file with sphere in the cornell box, subdividing a few times to get good results usually resulted in wait times for 1 minutes before anything rendered on screen. (thats no even really that high resolution)

	======Extra Credit=======
	Visualization tool that shows the normals and rays tracing from centroid to centroid of each patch helps debug issues with angles between those rays, and understand how to make your form factors. To run it on any of the radiocity files just subdivided once at least, turn on wireframe, and hit 'x'. It will take the object with the most undistributed light and show you the rays from it to all other patch in relation to their normals.

	======Known bugs==========
	Have to restart after running homebrew visualization.
	Sometimes I feels like forever to for the solution to converge.



PHOTON MAPPING:
< insert comments on the implementation, known bugs, extra credit >
Nope!



OTHER NEW FEATURES OR EXTENSIONS FOR EXTRA CREDIT:
Include instructions for use and test cases and sample output as appropriate.
