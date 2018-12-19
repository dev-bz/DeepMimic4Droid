ifndef OUTPUT
OUTPUT:=/data/user/0/org.c.ide/files/tmpdir/a.out
endif

shared:= $(filter -shared, $(CC))
ifeq ($(shared),-shared)
UI:=ui
CPPFLAGS+=-DENABLE_DRAW
endif
DirName:=. render render/lodepng sim util util/json anim scenes $(UI)
FindName:=$(DirName:%=%/*.cpp) $(DirName:%=%/*.c)
RMObjects:= $(wildcard *.o $(DirName:%=%/*.o))
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

CPPFLAGS+= -DCPU_ONLY -U__ARM_NEON -I. -I../thrid -I../thrid/bullet3 -I../thrid/include -Iutil
USE_CBLAS:=1
USE_CAFFE:=2
NetWork:=1
ifeq ($(NetWork),$(USE_CBLAS))
LDFLAGS+= -lopenblas
OPENMP:=-I../thrid/include -DUSE_CBLAS
else ifeq ($(NetWork),$(USE_CAFFE))
LDFLAGS+=  -Wl,--whole-archive -lcaffe -Wl,--no-whole-archive -lcaffeproto -lglog -lgflags -lopenblas -lprotobuf
CPPFLAGS+=-DUSE_CAFFE
OPENMP:=-DUSE_CAFFE
else
LDFLAGS+=-lomp
OPENMP:=-fopenmp
endif
ifdef BaseName
all.suffix:= $(suffix $(Sources))
ifeq ($(words $(BaseName)), $(words $(all.suffix)))
cxx.suffix:= $(filter .cc .cpp .cxx, $(all.suffix))
XCC:= $(if $(cxx.suffix), $(CXX),$(CC))
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
a.out:$(Objects)
	$(XCC) $^ $(LDFLAGS) -o $@ -O3 -L../thrid/lib64 -lbt -lEGL -lGLESv3
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

-include $(DependFiles) ""
#$(error $(DependFiles))

net.o:net.c makefile .cide
	gcc $(CFLAGS) $(CPPFLAGS) -c -o net.o net.c $(OPENMP)