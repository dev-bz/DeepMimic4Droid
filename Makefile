ifndef OUTPUT
OUTPUT:=/data/user/0/org.c.ide/files/tmpdir/a.out
endif

ProjectDir:=$(PWD)

shared:= $(filter -shared, $(LINK))
ifeq ($(shared),-shared)
UI:=ui
CPPFLAGS+=-DENABLE_DRAW
endif
DirName:=. render render/lodepng sim util util/json anim scenes $(UI)
FindName:=$(DirName:%=%/*.cpp) $(DirName:%=%/*.c)
RMObjects:= $(wildcard $(DirName:%=%/*.o))
ifdef MORE
	MoreBaseName:=$(sort $(basename $(MORE)))
endif
MoreObjects:=$(wildcard $(MoreBaseName:%=%.o) $(OUTPUT))
ifdef MoreObjects
RMObjects+=$(MoreObjects)
endif
ifdef RMObjects
RMObjects:=@rm $(RMObjects)
endif

Sources:=$(wildcard $(FindName))  $(MORE)
BaseName:=$(basename $(Sources))
ObjDir=.
DEPEND_OPTIONS = -MM -MF "$*.d" \
         -MT "$*.o" -MT "$*.d"
DEPEND_MOVEFILE = then $(MV) -f "$(ObjDir)/$*.d.tmp" "$(ObjDir)/$*.d"; \
                  else $(RM) "$(ObjDir)/$*.d.tmp"; exit 1; fi

CPPFLAGS+= -DCPU_ONLY -U__ARM_NEON -I. -Ithrid/bullet3-2.87/build/local/include/bullet -Iutil -Ilocal/include/eigen3
USE_CBLAS:=1
USE_CAFFE:=2
NetWork:=1
ifeq ($(NetWork),$(USE_CBLAS))
LDFLAGS+=  -Llocal/lib64 -lopenblas
OPENMP:=-Ilocal/include/openblas -DUSE_CBLAS
OPENBLAS:=local/lib64/libopenblas.a
else ifeq ($(NetWork),$(USE_CAFFE))
LDFLAGS+=  -Wl,--whole-archive -lcaffe -Wl,--no-whole-archive -lcaffeproto -lglog -lgflags -lopenblas -lprotobuf
CPPFLAGS+=-DUSE_CAFFE
OPENMP:=-DUSE_CAFFE
else
LDFLAGS+=-lomp
OPENMP:=-fopenmp
endif

BULLET3LIB:=-lLinearMath -lBulletDynamics -lBulletCollision -Lthrid/bullet3-2.87/build/local/lib

ifdef BaseName
all.suffix:= $(suffix $(Sources))
ifeq ($(words $(BaseName)), $(words $(all.suffix)))
cxx.suffix:= $(filter .cc .cpp .cxx, $(all.suffix))
XCC:= $(if $(cxx.suffix), $(CXX),$(CC)) $(LINK)

Objects:=$(BaseName:%=%.o)
DependSourceFiles := $(basename $(filter %.cpp %.c %.cc %.cxx, $(Sources)))
DependFiles := $(DependSourceFiles:%=%.d)
DependRmFiles:=$(wildcard $(DependFiles))
#$(warning $(DependRmFiles) hello)
MoreObjects+= $(DependFiles)
all: $(OUTPUT)
#	rm $(DependRmFiles)
#	@echo  hello $(DEPEND_OPTIONS)
#	@echo $(Objects)
$(OUTPUT):a.out
	strip -s $<
	cp $< $@
	chmod 700 $@
a.out:$(Objects) $(OPENBLAS) thrid/bullet3-2.87/build/local
	$(XCC) $(Objects) $(LDFLAGS) -o $@ -O3 $(BULLET3LIB) -lEGL -lGLESv3
else
all:
	clear
	@echo 某些文件名含有空格,终止编译
endif
else
all:
	clear
	@echo 项目中没有找到源代码文件
endif
clean:
	@echo Clean Objects
	$(RMObjects)

%.d: %.c
	gcc -MM -MG -MF $@  -MT "$*.o" -MT "$*.d" $(CPPFLAGS) $<
%.d: %.cc
	g++ -MM -MG -MF $@ -MT "$*.o" -MT "$*.d" $(CPPFLAGS) $<
%.d: %.cxx
	g++ -MM -MG -MF $@ -MT "$*.o" -MT "$*.d" $(CPPFLAGS) $<
%.d: %.cpp
	g++ -MM -MG -MF $@ -MT "$*.o" -MT "$*.d" $(CPPFLAGS) $<
ifdef OPENBLAS
$(OPENBLAS):$(HOME)/cblas/Makefile
	@echo build openblas by Makefile
	make install -j4 -C $(HOME)/cblas
	touch -r $@ $<

$(HOME)/cblas/Makefile:cblas/CMakelists.txt
	cd $(HOME);mkdir -p cblas;cd cblas;cmake -D NOFORTRAN=1 -D CMAKE_INSTALL_PREFIX="$(ProjectDir)/local" -G "Unix Makefiles" $(ProjectDir)/cblas
endif
local/include/eigen3:eigen-git/build/Makefile
	cd eigen-git/build;make install

eigen-git/build/Makefile:eigen-git/CMakelists.txt
	mkdir -p eigen-git
	mkdir -p eigen-git/build
	cd eigen-git/build;cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../local

thrid/bullet3-2.87/build/local:thrid/bullet3-2.87/build/Makefile
	make install -j4 -C thrid/bullet3-2.87/build
	touch -r $@ $<

thrid/bullet3-2.87/build/Makefile:thrid/bullet3-2.87/CMakelists.txt
	mkdir -p thrid/bullet3-2.87/build
	cd thrid/bullet3-2.87/build;cmake -G "Unix Makefiles" .. -DCMAKE_INSTALL_PREFIX=local -DBUILD_BULLET2_DEMOS=OFF -DBUILD_BULLET3=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_CLSOCKET=OFF -DBUILD_ENET=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_EXTRAS=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_PYBULLET=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release

thrid/bullet3-2.87/CMakelists.txt:thrid/bullet3.tar.gz
	/system/bin/tar -C thrid -xf thrid/bullet3.tar.gz
	touch -r thrid/bullet3-2.87/CMakelists.txt thrid/bullet3.tar.gz

thrid/bullet3.tar.gz:
	mkdir -p thrid
	curl -o thrid/bullet3.tar.gz -L https://github.com/bulletphysics/bullet3/archive/2.87.tar.gz

-include $(DependFiles) ""
#$(error $(DependFiles))
net.o:net.c Makefile .cide $(OPENBLAS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o net.o net.c $(OPENMP)