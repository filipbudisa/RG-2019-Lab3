name = SimulacijaTkanine

srcFiles = $(shell find src/ -name '*.cpp')
headerfiles = $(shell find src/ -name '*.h')
objects = $(srcFiles:.cpp=.o)
depends = $(srcFiles:.cpp=.d)

shaderFiles = $(shell find shaders/ -name '*.glsl')
shaderObjects = $(shaderFiles:.glsl=.spv)

VULKAN_SDK_PATH = /home/filip/Vulkan/1.1.121.1/x86_64

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan -lpthread
#LDFLAGS = -L$(VULKAN_SDK_PATH)/lib -lvulkan -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor

SCENE ?= 1
POINTS ?= 10

.PHONY: all clean

all: $(name) shaders

clean:
	rm -f $(name) $(shaderObjects) $(objects) $(depends)

$(name): $(objects)
	g++ -g -DDEBUG $(CFLAGS) -o $(name) $(objects) $(LDFLAGS)

shaders/%.spv: shaders/%.glsl
	$(VULKAN_SDK_PATH)/bin/glslangValidator -V $< -o $@

test: all
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./$(name) $(SCENE) $(POINTS)
	#./$(name) ${SCENE}

debug: all
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d gdb ./$(name) $(SCENE) $(POINTS)

shaders: $(shaderObjects)

-include $(depends)

src/%.o: src/%.cpp
	g++ -g -DDEBUG $(CFLAGS) -MMD -MP -c -o $@ $<