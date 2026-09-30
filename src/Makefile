CXX=g++
CC=gcc
CXXOPTFLAGS=-Wall -O2 -fsigned-char -fomit-frame-pointer -Wno-write-strings -Wno-unused -Wno-parentheses -fpermissive
CXXFLAGS=$(CXXOPTFLAGS) -include glibc_compat.h -I../enet/include `sdl2-config --cflags`
CFLAGS=-O2 -I../enet/include -DHAS_SOCKLEN_T -DHAS_FCNTL -DHAS_POLL -DHAS_MSGHDR_FLAGS -DHAS_GETHOSTBYNAME_R -DHAS_GETHOSTBYADDR_R

# GLU is linked statically (no CFW ships it); GL comes from gl4es at run time.
CLIENT_LIBS=-Wl,-Bstatic -lGLU -Wl,-Bdynamic -lGL `sdl2-config --libs` -lSDL2_mixer -lz
ENET_OBJS=../enet/callbacks.o ../enet/host.o ../enet/list.o ../enet/memory.o ../enet/packet.o \
	../enet/peer.o ../enet/protocol.o ../enet/unix.o
CLIENT_OBJS= \
	client.o \
	clientextras.o \
	clientgame.o \
	clients2c.o \
	command.o \
	console.o \
	editing.o \
	entities.o \
	main.o \
	menus.o \
	monster.o \
	physics.o \
	rendercubes.o \
	renderextras.o \
	rendergl.o \
	rendermd2.o \
	renderparticles.o \
	rendertext.o \
	rndmap.o \
	savegamedemo.o \
	server.o \
	serverbrowser.o \
	serverms.o \
	serverutil.o \
	sound.o \
	stb_image.o \
	tools.o \
	weapon.o \
	world.o \
	worldio.o \
	worldlight.o \
	worldocull.o \
	worldrender.o

default: client

client: $(CLIENT_OBJS) $(ENET_OBJS)
	$(CXX) $(CXXFLAGS) -s -o cube_client $(CLIENT_OBJS) $(ENET_OBJS) $(CLIENT_LIBS)

clean:
	-rm -f $(CLIENT_OBJS) $(ENET_OBJS) cube_client
