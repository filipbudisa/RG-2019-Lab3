cmake_minimum_required(VERSION 3.12)
project(Vulk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CMAKE_CXX_STANDARD 17)

AUX_SOURCE_DIRECTORY(src SOURCES)
AUX_SOURCE_DIRECTORY(src/entities SOURCES)
AUX_SOURCE_DIRECTORY(src/player SOURCES)
AUX_SOURCE_DIRECTORY(src/world SOURCES)
AUX_SOURCE_DIRECTORY(src/graphics SOURCES)
AUX_SOURCE_DIRECTORY(src/storage SOURCES)
AUX_SOURCE_DIRECTORY(src/interfaces SOURCES)
AUX_SOURCE_DIRECTORY(src/curves SOURCES)
AUX_SOURCE_DIRECTORY(src/particlesystem SOURCES)
AUX_SOURCE_DIRECTORY(src/springsystem SOURCES)
AUX_SOURCE_DIRECTORY(src/physics SOURCES)

add_executable(VulkanTest ${SOURCES})

set(Vulkan_INCLUDE_DIR /home/filip/Vulkan/1.1.121.1/x86_64/include)
set(Vulkan_LIBRARY /home/filip/Vulkan/1.1.121.1/x86_64)

find_package(Vulkan REQUIRED)
target_include_directories(VulkanTest PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VulkanTest Vulkan::Vulkan)