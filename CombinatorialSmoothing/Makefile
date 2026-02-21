TARGET= CombinatorialSmoothing
SOURCE= CombinatorialSmoothing.cpp

COMPILER ?= gcc-15
#COMPILER ?= clang

# Detect architecture and OS
ARCH := $(shell uname -m)
UNAME := $(shell uname -s)

# Set architecture-specific flags
ifeq ($(ARCH),x86_64)
	ARCH_FLAGS = -msse2
else
	ARCH_FLAGS =
endif

# Set OS-specific flags
ifeq ($(UNAME),Darwin)
	# macOS: Use frameworks instead of -l flags for OpenGL, but still need GLEW library
	# GL wrapper directory will be added first in include path to override bundled GL headers
	GL_LIBS = -framework OpenGL -framework GLUT -lGLEW
	GL_INCLUDE = -I./../Include/ -I./../ThirdParty/Include/ -I.
	# Add Homebrew library path for libpng, libjpeg, etc.
	BREW_LIB_PATH = -L/opt/homebrew/lib -L/usr/local/lib
else
	# Linux: Use traditional library flags
	GL_LIBS = -lGL -lGLU -lglut -lGLEW
	GL_INCLUDE = -I./../Include/ -I./../ThirdParty/Include/ -I.
	BREW_LIB_PATH =
endif

ifneq (,$(findstring gcc,$(COMPILER)))
	CFLAGS += -fpermissive -fopenmp -Wno-deprecated -Wno-unused-result -Wno-format $(ARCH_FLAGS) -std=c++17
	LFLAGS += $(BREW_LIB_PATH) -lgomp -lz $(GL_LIBS) -lpng -ljpeg
	CC=$(COMPILER)
	CXX=$(subst gcc,g++,$(COMPILER))
else
	CFLAGS += -fpermissive -Wno-deprecated -Wno-unused-result -Wno-format $(ARCH_FLAGS) -std=c++17
	LFLAGS += $(BREW_LIB_PATH) -lgomp -lz $(GL_LIBS) -lpng -ljpeg
	CC=clang
	CXX=clang++
endif

CFLAGS_DEBUG = -DDEBUG -g3
LFLAGS_DEBUG =

CFLAGS_RELEASE = -O3 -DRELEASE -funroll-loops -DNDEBUG
LFLAGS_RELEASE = -O3

SRC = ./
BIN = ./../Bin/Linux/
BIN_O = ./
INCLUDE = $(GL_INCLUDE)
## -I/usr/include/

MD=mkdir

# On macOS, create GL wrapper headers to redirect to system frameworks
ifeq ($(UNAME),Darwin)
GL_WRAPPER_DIR = ./GL_wrappers
$(shell mkdir -p $(GL_WRAPPER_DIR)/GL)
$(shell printf '%s\n' '#include <OpenGL/gl.h>' > $(GL_WRAPPER_DIR)/GL/gl.h)
$(shell printf '%s\n' '#include <OpenGL/glu.h>' > $(GL_WRAPPER_DIR)/GL/glu.h)
$(shell printf '%s\n' '#include <GLUT/glut.h>' > $(GL_WRAPPER_DIR)/GL/glut.h)
INCLUDE := -I$(GL_WRAPPER_DIR) $(INCLUDE)
endif

OBJECTS=$(addprefix $(BIN_O), $(addsuffix .o, $(basename $(SOURCE))))

all: CFLAGS += $(CFLAGS_RELEASE)
all: LFLAGS += $(LFLAGS_RELEASE)
all: $(BIN)
all: $(BIN)$(TARGET)

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: LFLAGS += $(LFLAGS_DEBUG)
debug: $(BIN)
debug: $(BIN)$(TARGET)

clean:
	rm -f $(BIN)$(TARGET)
	rm -f $(OBJECTS)
ifeq ($(UNAME),Darwin)
	rm -rf $(GL_WRAPPER_DIR)
endif

$(BIN):
	$(MD) -p $(BIN)

$(BIN)$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LFLAGS)

$(BIN_O)%.o: $(SRC)%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDE) $<

$(BIN_O)%.o: $(SRC)%.cpp
	$(CXX) -c -o $@ $(CFLAGS) $(INCLUDE) $<


