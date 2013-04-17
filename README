Tesseract is a fork of the Cube 2: Sauerbraten engine. The goal of Tesseract
is to make mapping more fun by using modern dynamic rendering techniques, so
that you can get instant feedback on lighting changes, not just geometry.

No more long calclight pauses... just plop down the light, move it, change its
color, or do whatever else with it. It all happens in real-time now.

Tesseract removes the static lightmapping system of Sauerbraten and replaces
it with completely dynamic lighting system based on deferred shading and
shadowmapping.

It provides a bunch of new rendering features such as:

* deferred shading
* omnidirectional point lights using cubemap shadowmaps
* perspective projection spotlight shadowmaps
* orthographic projection sunlight using cascaded shadowmaps
* HDR rendering with tonemapping and bloom
* real-time diffuse global illumination for sunlight (radiance hints)
* screen-space ambient occlusion
* screen-space reflections and refractions for water and glass (use as many water planes as you want now!)
* screen-space refractive alpha cubes
* deferred MSAA/CSAA, subpixel morphological anti-aliasing (SMAA 1x, T2x, S2x, and 4x), FXAA, and temporal AA
* runs on both OpenGL Core (3.0+) and legacy (2.1+) contexts

Install:

Currently, to use Tesseract you must have a copy of the "packages" directory
from a recent checkout of Sauerbraten SVN. To get a copy of Sauerbraten SVN,
please see the following page: http://sourceforge.net/p/sauerbraten/code

If you are on Linux or other unixoid, just make a symlink to the Sauerbraten
packages directory inside your Tesseract directory. You can also edit the
tesseract_unix file to point to an existing Sauerbraten SVN checkout.

On Windows, do a checkout of the Sauerbraten SVN inside the Tesseract directory,
or edit the tesseract.bat file to point to an existing Sauerbraten SVN checkout
you might have.

Editing:

To make shadowmapped point lights, just make light entities as normal. Optionally you can control the shadowing properties of the light using attribute 5: 0 = shadows, 1 = no shadows, 2 = static shadows - world and mapmodels only (no playermodels or pickups). Shadowmaps are cached from frame to frame if no dynamic entities such as playermodels are rendered into them, so static lights (2) will generally be much faster than normal lights (0) during gameplay and should be used where possible. Unshadowed lights (1) are even faster as they further reduce shading costs, so they should also be used where possible.

To make cascaded shadowmapped sunlight, just use the normal sunlight commands:
sunlight
sunlightyaw
sunlightpitch
You can also set sunlightyaw and sunlightpitch from your current direction with the following command:
getsundir

You can control the diffuse global illumination of sunlight using the following parameters:
giscale - scales the contribution of indirect light
gidist - limits the distance which indirect light travels (smaller values are more intense, but higher values allow farther travel)
giaoscale - controls the contribution of atmospheric light
skylight - controls the color of atmospheric light

Screen-space ambient occlusion affects mostly ambient light and to a lesser degree sunlight,
but it does not affect point lights. If you want to make use of SSAO in your map, just use 
the ambient command to set an appropriate ambient light color/intensity.

To make refractive alpha cubes:
First mark the cubes with the alpha material. Use valpha or texalpha to set the transparency level as normal.
Then use either of the following commands:
vrefract K [R G B]
K is the strength of the refraction, on a scale of 0 to 1, 1 meaning distortion spanning the entire screen, 0.5 spanning half the screen, etc.
The direction of the distortion is taken from the normal-map of the texture.
R G B are optional and specify a color for the refraction, with each component being on a scale of 0 to 1 as well.

After editing operations your map may be full of small cubes and triangles, and you will notice that your map looks faceted.
To fix this you still need to use the calclight command, however, it no longer generates lightmaps anymore.
It just does a remip optimization pass as well as calculates smoothed normals to make things look better.
Eventually this may be done differently using some sort of background optimization while editing.

