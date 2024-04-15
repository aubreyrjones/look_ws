cmake_minimum_required(VERSION 3.12)
project(look_ws)

set(CMAKE_CXX_STANDARD 11)

# Add the library search directory
link_directories(./libxreal)


include_directories(./libxreal/include )


# Add the main executable
add_executable(lookws src/main.cpp)

# Link the library
target_link_libraries(lookws xrealAirLibrary Fusion json-c hidapi-libusb)
