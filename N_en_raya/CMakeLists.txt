cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0077 NEW)
project(MySFMLApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module to download dependencies
include(FetchContent)

# Declare SFML to be downloaded from GitHub
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.0
)

# Download and make SFML available
FetchContent_MakeAvailable(SFML)

# Add your executable
add_executable(${PROJECT_NAME} src/MinMaxTree.cpp)

# Link SFML libraries to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)