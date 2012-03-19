CXXFLAGS= -Os
override CXXFLAGS+= -Wall -fsigned-char -fno-exceptions -fno-rtti

PLATFORM= $(shell uname -s)
PLATFORM_PREFIX= native

INCLUDES= -I../shared -I../engine -I../rpggame -I../enet/include

STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
  STRIP=strip
endif
endif

MV=mv

ifneq (,$(findstring MINGW,$(PLATFORM)))
WINDRES= windres
CLIENT_INCLUDES= $(INCLUDES) -I../include
CLIENT_LIBS= -mwindows -L../lib -lSDL -lSDL_image -lSDL_mixer -lzdll -lopengl32 -lenet -lws2_32 -lwinmm
else	
CLIENT_INCLUDES= $(INCLUDES) -I/usr/X11R6/include `sdl-config --cflags`
CLIENT_LIBS= -L../enet/.libs -lenet -L/usr/X11R6/lib -lX11 `sdl-config --libs` -lSDL_image -lSDL_mixer -lz -lGL
endif
ifeq ($(PLATFORM),Linux)
CLIENT_LIBS+= -lrt
endif
CLIENT_OBJS= \
	../shared/crypto.o \
	../shared/geom.o \
	../shared/stream.o \
	../shared/tools.o \
	../shared/zip.o \
	../engine/3dgui.o \
	../engine/bih.o \
	../engine/blend.o \
	../engine/blob.o \
	../engine/client.o	\
	../engine/command.o \
	../engine/console.o \
	../engine/cubeloader.o \
	../engine/decal.o \
	../engine/dynlight.o \
	../engine/glare.o \
	../engine/grass.o \
	../engine/lightmap.o \
	../engine/main.o \
	../engine/material.o \
	../engine/menus.o \
	../engine/movie.o \
	../engine/normal.o	\
	../engine/octa.o \
	../engine/octaedit.o \
	../engine/octarender.o \
	../engine/physics.o \
	../engine/pvs.o \
	../engine/rendergl.o \
	../engine/rendermodel.o \
	../engine/renderparticles.o \
	../engine/rendersky.o \
	../engine/rendertext.o \
	../engine/renderva.o \
	../engine/server.o	\
	../engine/serverbrowser.o \
	../engine/shader.o \
	../engine/shadowmap.o \
	../engine/sound.o \
	../engine/texture.o \
	../engine/water.o \
	../engine/world.o \
	../engine/worldio.o \
	../rpggame/ent.o \
	../rpggame/entities.o \
	../rpggame/objset.o \
	../rpggame/rpg.o

CLIENT_PCH= ../shared/cube.h.gch ../engine.h.gch ../rpggame/rpg.h.gch

ifeq ($(PLATFORM),SunOS)
CLIENT_LIBS+= -lsocket -lnsl -lX11
endif

default: all

all: client

../enet/Makefile:
	cd ../enet; ./configure --enable-shared=no --enable-static=yes
	
libenet: ../enet/Makefile
	$(MAKE)	-C ../enet/ all

clean-enet: enet/Makefile
	$(MAKE) -C ../enet/ clean

clean:
	-$(RM) $(CLIENT_PCH) $(CLIENT_OBJS) rpg_client

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -o $@.tmp $(subst .h.gch,.h,$@)
	$(MV) $@.tmp $@

$(CLIENT_OBJS): CXXFLAGS += $(CLIENT_INCLUDES)
$(filter ../shared/%,$(CLIENT_OBJS)): $(filter ../shared/%,$(CLIENT_PCH))
$(filter ../engine/%,$(CLIENT_OBJS)): $(filter ../engine/%,$(CLIENT_PCH))
$(filter ../rpggame/%,$(CLIENT_OBJS)): $(filter ../rpggame/%,$(CLIENT_PCH))

ifneq (,$(findstring MINGW,$(PLATFORM)))
client: $(CLIENT_OBJS)
	$(WINDRES) -I ../vcpp -i ../vcpp/mingw.rc -J rc -o ../vcpp/mingw.res -O coff 
	$(CXX) $(CXXFLAGS) -o ../../bin/rpg.exe ../vcpp/mingw.res $(CLIENT_OBJS) $(CLIENT_LIBS)
ifneq (,$(STRIP))
	$(STRIP) ../../bin/rpg.exe
endif
else
client:	libenet $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o rpg_client $(CLIENT_OBJS) $(CLIENT_LIBS)
endif

depend:
	makedepend -Y -I../shared -I../engine -I../rpggame $(subst .o,.cpp,$(CLIENT_OBJS))
	makedepend -a -o.h.gch -Y -I../shared -I../engine -I../rpggame $(subst .h.gch,.h,$(CLIENT_PCH))

# DO NOT DELETE

../shared/crypto.o: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../shared/crypto.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../shared/crypto.o: ../shared/igame.h
../shared/geom.o: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../shared/geom.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../shared/geom.o: ../shared/igame.h
../shared/stream.o: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../shared/stream.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../shared/stream.o: ../shared/igame.h
../shared/tools.o: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../shared/tools.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../shared/tools.o: ../shared/igame.h
../shared/zip.o: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../shared/zip.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../shared/zip.o: ../shared/igame.h
../engine/3dgui.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/3dgui.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/3dgui.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/3dgui.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/3dgui.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/3dgui.o: ../engine/textedit.h
../engine/bih.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/bih.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/bih.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/bih.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/bih.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/blend.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/blend.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/blend.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/blend.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/blend.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/blob.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/blob.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/blob.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/blob.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/blob.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/client.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/client.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/client.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/client.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/client.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/command.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/command.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/command.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/command.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/command.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/console.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/console.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/console.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/console.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/console.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/cubeloader.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/cubeloader.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/cubeloader.o: ../shared/iengine.h ../shared/igame.h
../engine/cubeloader.o: ../engine/world.h ../engine/octa.h
../engine/cubeloader.o: ../engine/lightmap.h ../engine/bih.h
../engine/cubeloader.o: ../engine/texture.h ../engine/model.h
../engine/cubeloader.o: ../engine/varray.h
../engine/decal.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/decal.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/decal.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/decal.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/decal.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/dynlight.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/dynlight.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/dynlight.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/dynlight.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/dynlight.o: ../engine/texture.h ../engine/model.h
../engine/dynlight.o: ../engine/varray.h
../engine/glare.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/glare.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/glare.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/glare.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/glare.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/glare.o: ../engine/rendertarget.h
../engine/grass.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/grass.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/grass.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/grass.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/grass.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/lightmap.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/lightmap.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/lightmap.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/lightmap.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/lightmap.o: ../engine/texture.h ../engine/model.h
../engine/lightmap.o: ../engine/varray.h
../engine/main.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/main.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/main.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/main.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/main.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/material.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/material.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/material.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/material.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/material.o: ../engine/texture.h ../engine/model.h
../engine/material.o: ../engine/varray.h
../engine/menus.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/menus.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/menus.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/menus.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/menus.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/movie.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/movie.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/movie.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/movie.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/movie.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/normal.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/normal.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/normal.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/normal.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/normal.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/octa.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/octa.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/octa.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/octa.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/octa.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/octaedit.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/octaedit.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/octaedit.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/octaedit.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/octaedit.o: ../engine/texture.h ../engine/model.h
../engine/octaedit.o: ../engine/varray.h
../engine/octarender.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/octarender.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/octarender.o: ../shared/iengine.h ../shared/igame.h
../engine/octarender.o: ../engine/world.h ../engine/octa.h
../engine/octarender.o: ../engine/lightmap.h ../engine/bih.h
../engine/octarender.o: ../engine/texture.h ../engine/model.h
../engine/octarender.o: ../engine/varray.h
../engine/physics.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/physics.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/physics.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/physics.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/physics.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/physics.o: ../engine/mpr.h
../engine/pvs.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/pvs.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/pvs.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/pvs.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/pvs.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/rendergl.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/rendergl.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/rendergl.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/rendergl.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/rendergl.o: ../engine/texture.h ../engine/model.h
../engine/rendergl.o: ../engine/varray.h
../engine/rendermodel.o: ../engine/engine.h ../shared/cube.h
../engine/rendermodel.o: ../shared/tools.h ../shared/geom.h ../shared/ents.h
../engine/rendermodel.o: ../shared/command.h ../shared/iengine.h
../engine/rendermodel.o: ../shared/igame.h ../engine/world.h ../engine/octa.h
../engine/rendermodel.o: ../engine/lightmap.h ../engine/bih.h
../engine/rendermodel.o: ../engine/texture.h ../engine/model.h
../engine/rendermodel.o: ../engine/varray.h ../engine/ragdoll.h
../engine/rendermodel.o: ../engine/animmodel.h ../engine/vertmodel.h
../engine/rendermodel.o: ../engine/skelmodel.h ../engine/md2.h
../engine/rendermodel.o: ../engine/md3.h ../engine/md5.h ../engine/obj.h
../engine/rendermodel.o: ../engine/smd.h ../engine/iqm.h
../engine/renderparticles.o: ../engine/engine.h ../shared/cube.h
../engine/renderparticles.o: ../shared/tools.h ../shared/geom.h
../engine/renderparticles.o: ../shared/ents.h ../shared/command.h
../engine/renderparticles.o: ../shared/iengine.h ../shared/igame.h
../engine/renderparticles.o: ../engine/world.h ../engine/octa.h
../engine/renderparticles.o: ../engine/lightmap.h ../engine/bih.h
../engine/renderparticles.o: ../engine/texture.h ../engine/model.h
../engine/renderparticles.o: ../engine/varray.h ../engine/rendertarget.h
../engine/renderparticles.o: ../engine/depthfx.h ../engine/explosion.h
../engine/renderparticles.o: ../engine/lensflare.h ../engine/lightning.h
../engine/rendersky.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/rendersky.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/rendersky.o: ../shared/iengine.h ../shared/igame.h
../engine/rendersky.o: ../engine/world.h ../engine/octa.h
../engine/rendersky.o: ../engine/lightmap.h ../engine/bih.h
../engine/rendersky.o: ../engine/texture.h ../engine/model.h
../engine/rendersky.o: ../engine/varray.h
../engine/rendertext.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/rendertext.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/rendertext.o: ../shared/iengine.h ../shared/igame.h
../engine/rendertext.o: ../engine/world.h ../engine/octa.h
../engine/rendertext.o: ../engine/lightmap.h ../engine/bih.h
../engine/rendertext.o: ../engine/texture.h ../engine/model.h
../engine/rendertext.o: ../engine/varray.h
../engine/renderva.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/renderva.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/renderva.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/renderva.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/renderva.o: ../engine/texture.h ../engine/model.h
../engine/renderva.o: ../engine/varray.h
../engine/server.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/server.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/server.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/server.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/server.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/serverbrowser.o: ../engine/engine.h ../shared/cube.h
../engine/serverbrowser.o: ../shared/tools.h ../shared/geom.h
../engine/serverbrowser.o: ../shared/ents.h ../shared/command.h
../engine/serverbrowser.o: ../shared/iengine.h ../shared/igame.h
../engine/serverbrowser.o: ../engine/world.h ../engine/octa.h
../engine/serverbrowser.o: ../engine/lightmap.h ../engine/bih.h
../engine/serverbrowser.o: ../engine/texture.h ../engine/model.h
../engine/serverbrowser.o: ../engine/varray.h
../engine/shader.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/shader.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/shader.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/shader.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/shader.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/shadowmap.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/shadowmap.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/shadowmap.o: ../shared/iengine.h ../shared/igame.h
../engine/shadowmap.o: ../engine/world.h ../engine/octa.h
../engine/shadowmap.o: ../engine/lightmap.h ../engine/bih.h
../engine/shadowmap.o: ../engine/texture.h ../engine/model.h
../engine/shadowmap.o: ../engine/varray.h ../engine/rendertarget.h
../engine/sound.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/sound.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/sound.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/sound.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/sound.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/texture.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/texture.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/texture.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/texture.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/texture.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/texture.o: ../engine/scale.h
../engine/water.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/water.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/water.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/water.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/water.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/world.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/world.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/world.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/world.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/world.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../engine/worldio.o: ../engine/engine.h ../shared/cube.h ../shared/tools.h
../engine/worldio.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../engine/worldio.o: ../shared/iengine.h ../shared/igame.h ../engine/world.h
../engine/worldio.o: ../engine/octa.h ../engine/lightmap.h ../engine/bih.h
../engine/worldio.o: ../engine/texture.h ../engine/model.h ../engine/varray.h
../rpggame/ent.o: rpg.h ../shared/cube.h ../shared/tools.h ../shared/geom.h
../rpggame/ent.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../rpggame/ent.o: ../shared/igame.h stats.h rpgent.h rpgobj.h
../rpggame/entities.o: rpg.h ../shared/cube.h ../shared/tools.h
../rpggame/entities.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../rpggame/entities.o: ../shared/iengine.h ../shared/igame.h stats.h rpgent.h
../rpggame/entities.o: rpgobj.h
../rpggame/objset.o: rpg.h ../shared/cube.h ../shared/tools.h
../rpggame/objset.o: ../shared/geom.h ../shared/ents.h ../shared/command.h
../rpggame/objset.o: ../shared/iengine.h ../shared/igame.h stats.h rpgent.h
../rpggame/objset.o: rpgobj.h
../rpggame/rpg.o: rpg.h ../shared/cube.h ../shared/tools.h ../shared/geom.h
../rpggame/rpg.o: ../shared/ents.h ../shared/command.h ../shared/iengine.h
../rpggame/rpg.o: ../shared/igame.h stats.h rpgent.h rpgobj.h

../shared/cube.h.gch: ../shared/tools.h ../shared/geom.h ../shared/ents.h
../shared/cube.h.gch: ../shared/command.h ../shared/iengine.h
../shared/cube.h.gch: ../shared/igame.h
../rpggame/rpg.h.gch: ../shared/cube.h ../shared/tools.h ../shared/geom.h
../rpggame/rpg.h.gch: ../shared/ents.h ../shared/command.h
../rpggame/rpg.h.gch: ../shared/iengine.h ../shared/igame.h stats.h rpgent.h
../rpggame/rpg.h.gch: rpgobj.h
