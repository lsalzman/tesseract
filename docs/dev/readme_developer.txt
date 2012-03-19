
Sauerbraten initial development documentation
=============================================

This document describes the state of sauerbraten at the moment
of its initial open source release. Anyone who wants to program
as part of the sauerbraten project should read this document VERY
carefully.


Sauerbraten History
===================
Sauerbraten was started as an effort to take the ideas behind Cube
further: both allow significantly more geometrical possibilities 
while at the same time being simplified compared to Cube's internal
structure. Read all about this on the Sauerbraten homepage:

http://wouter.fov120.com/sauerbraten/index.html

Sauerbraten is based on the cube source code of roughly the november
2002 state, and both code bases have diverged significantly since.

The base Cube code was cleaned up a bit, and all code specific to
the cube world structure removed or commented out. Some other
parts of cube dependent on this were also commented out, but most
code is in the sauerbraten base unmodifed. All gamecode, client/
server code, physics (dynamics, not collision), sound, monsters,
weapons, console/scripting, entities, menus, md2 / hud / particle
rendering is all there pretty much unmodified from cube and either
already works, or can be easily be made to work again as sauerbraten
progresses.

This document assumes some familiarity with the original Cube source
code. 

The major areas that are new have to do with world structure, like
rendering, editing etc. A description of each follows:

World Structure
===============
The new world structure is the ultimate in elegance, if I may say so
myself ;) "octa.h" contains the definition: the only data structure
is the new struct "cube". a "cube" is the central component of Sauerbraten's
octree structure (if you don't know octree's, google them and read some
documents about them first).

If a "cube" is a leaf, i.e. the smallest element that has been edited
as a single cube at that level in the octree, the edges/faces fields
define the shape of the cube using the aforementioned "edge spans".
A cube has 12 edges, each having 1 edge span occupying 1 byte.
The single byte is split in two 4bit quanties each storing a number
between 0 and 8, the lower one being the start of the span, and the
higher being the end of the span.

A maximally exstended cube has all edge spans with value 0x80, by
definition. A completely empty cube (space) can be defined in many
different ways (all edges of 0x00, 0x11 .. 0x88), but as a convention
only the value 0x00 is used to allow for fast testing. All current
functions enforce this already.

if a "cube" is not a leaf but is 8-fold subdivided, it has a pointer
to its 8 children allocated as one chunk. The other fields of a non-leaf
cube are meant to hold the LOD version of its children, but this is
currently not implemented.

note the lack of a "type" field in the "cube" struct: there is only
one type of cube (unlike the 7 different types in the Cube engine).
This greatly simplifies the code.

It may take a while to wrap your head around the idea of edge spans and
the way they are stored. Reading the algorithms that handle them 
should help.

"octa.cpp" contains some of the basic functions of dealing with this
structure. Particularly study lookupcube(), as it is the central function
used by all other code to access the octree and to further subdivide it
if needed.

"worldio.cpp" works as before but now with the new world structure,
"map" and "savemap" work as expected.


World Rendering
===============
"octarender.cpp" contains all functions that draw the octree & cubes
to the screen. It consists of two parts:

gencubeverts() and all the code above it takes the octree structure and
turns it into a vertex array and index lists ready to be rendered. It currently
does this for the entire level at once after a level load or edit, ideally
it should be done for small parts of the level at a time (see below). This
function cannot be called every frame (as it is in Cube) as it is quite
expensive.

The actual structure of the code is to take each cube, compute the 8 vertices
for it by intersecting the 6 faces defined by the 12 edge spans (the intersection
functions are in "geom.c"). It also attempts to cull unneeded faces such as
those of equal size and touching eachother. The hashtable at the top is used
to not put duplicate vertices in the array.

This code is not easy to follow, but does a lot in very little code, so
you will just have to read it to understand the details. As with everything
in Cube/Sauerbraten :)

octarender() and the 2 functions above it render the scene, and as you can
see are super simple (after all the work done by gencubeverts()). It renders
all quads using any particular texture together, and uses texgen for the
texture coordinates (which makes the vertex array very small at just 16
bytes per vertex).


World Editing
=============
This part I was in the middle of programming when I had to suspend work on it,
so contains the most bugs/unfinished parts and crappy code.

How editing works is briefly explained in "readme_player_mapper.txt". Play
with the editing functionality to get a feel of how it works.

cursorupdate() is the main function that gets called every frame in edit mode
to update the rectangle, and a dragged selection if there is one.

editface() then implements the actual geometry modification with the mousewheel
and the different key combinations.

Again, not easy to follow code, but not a lot of it considering that it implements
pretty much the entire set of editing operations.


Thing to be implemented
=======================
What follows is a list of items that would probably need to be implemented to make 
Sauerbraten into a fully usable engine. I have listed the items below roughly in
the order which I think would be useful they would happen.

Quite a few of the items can be worked on by different people without interfering
too much, i.e. one person can work on rendering issues, one on editing, on on
collision etc.

- make the current editing features work in all situations (they currently work in
  some of the 6 orientations and not in others).
  
- add texture selection feature (should be easy). Suggested is a "move to front"
  texture list much like Cube, that is saved with the map. Texture slots and
  texture loading is already in the engine just like Cube. Maybe later a fancy
  texture selection window can be added.

- once the editing is fully working, remove the redundancy in the various
  editing related functions

- currently only NV_VERTEX_ARRAY_RANGE is used to upload vertex arrays to video memory
  after the level changes. ATI_VERTEX_BUFFER_OBJECT and ARB_VERTEX_BUFFER_OBJECTS
  should also be supported.
  
- collision detection. The physics dynamics sort of work from Cube, but there is no
  proper collision detection (there is an ad-hoc collision detection which treats the
  player as a "needle").

- currently the entire level is rendered into one big vertex array every time the
  level is loaded or edited. Not only is this very inefficient for editing, it doesn't
  allow frustrum culling and occlusion culling. The level should be split up at certain
  levels in the octree, depending on the number of nodes below it (i.e. a high detail
  area will result in smaller chunks and more split up than a low detail area).
  The target amount of cubes in a chunk should be tunable: large enough so that there
  are enough polys in it such that rendering it at once is efficient, and small
  enough so that culling algorithms are effective.
  To make this work with VAR and VBO, a simple memory manager should be implemented
  on top of the VAR or VBO which allocates parts of its memory for these chunks
  as needed.
  Once this works, frustrum culling should be implemented. Occlusion culling is more
  complicated and can be left for later.
  
- lighting. Currently there is a provision for vertex lighting in the "cube" structure
  which is being filled with random values at the moment as you can see in the
  levels. The lighting model is debatable, but for the kind of engine Cube is,
  doing things like shadow volumes or shadow maps is probably not feasible yet,
  meaning that again some form of lightmap or subdivided vertex lighting must be used.
  I would suggest that for surfaces below a certain size only vertex light be used,
  and for the larger faces the engine accumulates lightdata in a series of lightmap
  textures as required (subdivision would probably be unwise given the potential
  scale sauerbraten worlds can have).

- fix all those small little issues from the former cube gameplay code which stopped
  working (should not cost a lot of time). If all the above items are implemented,
  Sauerbraten should be able to run the old Cube multiplayer and singleplayer game
  without much problems, in much more impressive worlds!

- occlusion culling. Will be quite complicated to do well, should be left as last
  item to do until all other parts of the engine are matured. Once we get to this
  point I'd be happy to help brainstorm on possible good algorithms.

- many interesting features can be implemented beyond this point. We'll think of them
  once we get there :)


Code style and simplicity
=========================
Many people may work on this code, all with their own coding styles and coding philosophies.
If everyone just goes their own way things will become a mess, and noone will be happy.

Coding style is least problematic. It would be easiest if everyone just followed my style
of formatting, identifiers etc. However if everyone agrees that they prefer a different
style that would be fine too, as long the style is applied consistently (i.e. all old
code is change to reflect the new style). One particular style I will forbid however, and
that is the so-called "hungarian notation".

Coding philosophy is a harder topic. I am however assuming that people working on
sauerbraten do so because they like Cube and its minimalistic engine design, and are
excited by the possibilities Sauebraten's new world structure brings. That means that
they are people that can agree with Cube's coding philosophy and may enjoy working with it.

Even though Cube's source code is not the prettiest (it is downright ugly in places),
it overal VERY easy to read, which almost entirely due to its incredibly small size
and simple algorithms. Lets face it, if you have to understand a physics system, there's
no way you are going to easily find your way around a physics system that stretches 20
.cpp and .h files with several pages of code each (even if its wonderfully designed),
compared to Cube's 1 or 2 pages in a single file (even if its ugly).

Ever wonder why an engine that is simpler than Doom2 still gets so much attention in
2004? And why nobody appears to do anything with all those hundreds of advanced engine
projects out there? its because of this kind of thing. 

Cube is about doing more with less (a lot less!) and Sauerbraten is no different. Its
about elegant tiny algorithms, and using not a single line of code more than needed.
I hope the programmers working on Sauerbraten will embrace this, and join in the fun
"sport" that is simplicity, even if by their background they prefer to create collosal
class hierarchies that include every version of the kitchen sink.

A misconception is that Cube's code philosphy is "anti-OO". This is not relevant at all.
I'd be happy for people to convert all of sauerbraten into a neat bunch of classes.
The _form_ of the code does not matter, as long as its the minimal, simplest, least
redundant one. The problem with classes as used by most people is that they require
to split up a part of the code between .h and .cpp, and also invite people to get
serious with data hiding, introducing the dreaded get/set methods. All of this
introduces useless clutter that will bloat up the code size of an engine several
fold for no gain other than being "correct". And readability for anyone except the
original writer will suffer.

If you want to create a class, I suggest you put it in a .h file in its entirety,
improving readability immensely by keeping all code together. Really, the design of
classes and the .h/.cpp idea don't work well together at all. Don't worry about executable
code bloat by the appearent "inline" nature of this, most linkers have gotten very good
at filtering this out nowadays. And unless you do the entire engine this way, you are
not going to feel the compilations speed difference. And please forget about get/set.
A small example is the "vec" class that is still a bunch of macros in the Cube code base.
Important to notice is that it is still exactly the same amount of code as in Cube,
however just a bit more readable than before.

Another think people worry about is the use of global variables in the code, because
global variables are certified evil (tm). But think of it: an engine intrinsically has
some global data it needs to store. So you put all of this in a class (or classes). But
this engine really only uses one instance of this class, so you make it a singleton.
Now you need to access this data. You write get() methods, and you litter your code
with calls to them. You have just bloated your code, made it less readable, and all
for what? for no change in actual structure whatsoever. In cube, a .cpp file is often
implicitly a "singleton" already. Don't worry too much about being Politically Correct,
worry about the actual structure of the code.

Doesn't all this hacking reduce maintainability? On the contrary. By keeping the code
the simplest possible and accepting no redundancy, most modifications you want to make
are simple because the code that is related to it is easy to isolate. Abstractions can
be made when they are first really needed, rather than ahead of time and forgotten.

So when you work on Sauerbraten, try taking these steps:

- Make it a challenge thinking about the algorithm design before you start coding.
  What is the simplest thing that could possibly work? How can I make it such it
  requires less data structures or code, or simpler data structures and code?
  Don't think in terms of just getting features done. Just because all other engines
  make a certain feature look complex doesn't mean we have to follow.

- Once you got a cool base idea, discuss with other Sauerbraten programmers. There
  will be a thread on the messageboard for this. Maybe they have further simplification
  ideas.
  
- Implement using exactly the code needed to implement the algorithm without redundancy.
  Refrain from implementing any big "systems" or "managers" to help your cause. Make
  it a sport to be efficient with the amount of code your algorithm needs.
  
- spend as much time refactoring as you can. If things get a bit messy, don't just
  let it become worse but take some time to restructure things.

- Be careful to not implement tons of special purpose features that sound neat. Focus
  on features that have more universal power and can be used for a multitude of things.
  Once players/mappers start to use Sauerbraten, they will ask for endless features,
  most of them not very well thought out. Resist the temptation :)


==========
Wouter van Oortmerssen
wouter@fov120.com


