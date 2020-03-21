ifeq ($(OS),)
OS = $(shell uname -s)
endif
PREFIX = /usr/local
CC   = gcc
CPP  = g++
AR   = ar
LIBPREFIX = lib
LIBEXT = .a
ifeq ($(OS),Windows_NT)
BINEXT = .exe
SOEXT = .dll
else ifeq ($(OS),Darwin)
BINEXT =
SOEXT = .dylib
else
BINEXT =
SOEXT = .so
endif
INCS = -Iinclude
CFLAGS = $(INCS) -Os
CPPFLAGS = $(INCS) -Os
STATIC_CFLAGS = -DDIRTRAV_GENERATE -DBUILD_DIRTRAV_STATIC
SHARED_CFLAGS = -DDIRTRAV_GENERATE -DBUILD_DIRTRAV_DLL
LIBS =
LDFLAGS =
ifeq ($(OS),Darwin)
STRIPFLAG =
else
STRIPFLAG = -s
endif
MKDIR = mkdir -p
RM = rm -f
RMDIR = rm -rf
CP = cp -f
CPDIR = cp -rf
DOXYGEN := $(shell which doxygen)

OSALIAS := $(OS)
ifeq ($(OS),Windows_NT)
ifneq (,$(findstring x86_64,$(shell gcc --version)))
OSALIAS := win64
else
OSALIAS := win32
endif
endif

libdirtrav_OBJ = src/dirtrav.o
libdirtrav_LDFLAGS = 
libdirtrav_SHARED_LDFLAGS =
ifneq ($(OS),Windows_NT)
SHARED_CFLAGS += -fPIC
endif
ifeq ($(OS),Windows_NT)
libdirtrav_SHARED_LDFLAGS += -Wl,--out-implib,$@$(LIBEXT)
endif
ifeq ($(OS),Darwin)
OS_LINK_FLAGS = -dynamiclib -o $@
else
OS_LINK_FLAGS = -shared -Wl,-soname,$@ $(STRIPFLAG)
endif

TOOLS_BIN = tree$(BINEXT) rdir$(BINEXT) folderstats$(BINEXT)
EXAMPLES_BIN = test1$(BINEXT)

COMMON_PACKAGE_FILES = README.md LICENSE Changelog.txt
SOURCE_PACKAGE_FILES = $(COMMON_PACKAGE_FILES) Makefile CMakeLists.txt doc/Doxyfile include/*.h src/*.c build/*.workspace build/*.cbp build/*.depend

default: all

all: static-lib shared-lib

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) 

%.static.o: %.c
	$(CC) -c -o $@ $< $(STATIC_CFLAGS) $(CFLAGS) 

%.shared.o: %.c
	$(CC) -c -o $@ $< $(SHARED_CFLAGS) $(CFLAGS)

static-lib: $(LIBPREFIX)dirtrav$(LIBEXT)

shared-lib: $(LIBPREFIX)dirtrav$(SOEXT)

$(LIBPREFIX)dirtrav$(LIBEXT): $(libdirtrav_OBJ:%.o=%.static.o)
	$(AR) cr $@ $^

$(LIBPREFIX)dirtrav$(SOEXT): $(libdirtrav_OBJ:%.o=%.shared.o)
	$(CC) -o $@ $(OS_LINK_FLAGS) $^ $(libdirtrav_SHARED_LDFLAGS) $(libdirtrav_LDFLAGS) $(LDFLAGS) $(LIBS)
	
#tree.o: src/tree.c
#	$(CC) -c -o $@ $< $(CFLAGS) 

#tree$(BINEXT).o: tree.o $(LIBPREFIX)dirtrav$(LIBEXT)
#	$(CC) -o $@ $< $(CFLAGS) 

tools: $(TOOLS_BIN)

examples: $(EXAMPLES_BIN)


test1$(BINEXT): src/test1.static.o $(LIBPREFIX)dirtrav$(LIBEXT)
	$(CC) -o $@ src/$(@:%$(BINEXT)=%.static.o) $(LIBPREFIX)dirtrav$(LIBEXT) $(libdirtrav_LDFLAGS) $(LDFLAGS)

.PHONY: doc
doc:
ifdef DOXYGEN
	$(DOXYGEN) doc/Doxyfile
endif

install: all doc
	$(MKDIR) $(PREFIX)/include $(PREFIX)/lib $(PREFIX)/bin
	$(CP) include/*.h $(PREFIX)/include/
	$(CP) *$(LIBEXT) $(PREFIX)/lib/
ifeq ($(OS),Windows_NT)
	$(CP) *$(SOEXT) $(PREFIX)/bin/
else
	$(CP) *$(SOEXT) $(PREFIX)/lib/
endif
ifdef DOXYGEN
	$(CPDIR) doc/man $(PREFIX)/
endif

.PHONY: version
version:
	sed -ne "s/^#define\s*DIRTRAV_VERSION_[A-Z]*\s*\([0-9]*\)\s*$$/\1./p" include/dirtrav_version.h | tr -d "\n" | sed -e "s/\.$$//" > version

#.PHONY: package
#package: version
#	tar cfJ libdirtrav-$(shell cat version).tar.xz --transform="s?^?libdirtrav-$(shell cat version)/?" $(SOURCE_PACKAGE_FILES)

.PHONY: package
package: version
	tar cfJ libdirtrav-$(shell cat version).tar.xz --transform="s?^?libdirtrav-$(shell cat version)/?" $(SOURCE_PACKAGE_FILES)

.PHONY: package
binarypackage: version
	$(MAKE) PREFIX=binpkg_$(OSALIAS)_temp install DIRTRAV_GENERATE_WIDE=1
ifneq ($(OS),Windows_NT)
	tar cfJ "libdirtrav-$(shell cat version)-$(OSALIAS).tar.xz" --transform="s?^binpkg_$(OSALIAS)_temp/??" $(COMMON_PACKAGE_FILES) binpkg_$(OSALIAS)_temp/*
else
	rm -f libdirtrav-$(shell cat version)-$(OSALIAS).zip
	cp -f $(COMMON_PACKAGE_FILES) binpkg_$(OSALIAS)_temp/
	cd binpkg_$(OSALIAS)_temp && zip -r -9 "../libdirtrav-$(shell cat version)-binary-$(OSALIAS).zip" $(COMMON_PACKAGE_FILES) * && cd ..
endif
	rm -rf binpkg_$(OSALIAS)_temp

.PHONY: clean
clean:
	$(RM) src/*.o *$(LIBEXT) *$(SOEXT) $(EXAMPLES_BIN) version libdirtrav-*.tar.xz doc/doxygen_sqlite3.db
	$(RMDIR) doc/html doc/man
