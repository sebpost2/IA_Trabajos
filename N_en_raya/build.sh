#!/bin/bash

# Configuration
EXECUTABLE_NAME="MySFMLApp"
BUILD_DIR="build"

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating $BUILD_DIR directory..."
    mkdir "$BUILD_DIR"
fi

# Configure the project with CMake
echo "Configuring the project..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    echo "Press any key to exit..."
    read -n 1 -s
    exit 1
fi

# Build the project
echo "Building the project..."
cmake --build "$BUILD_DIR"
if [ $? -ne 0 ]; then
    echo "Build failed."
    echo "Press any key to exit..."
    read -n 1 -s
    exit 1
fi

# Añadir al PATH la carpeta de las DLL de SFML
SFML_DLL_DIR="./build/_deps/sfml-build/lib/Debug"
echo "Adding SFML DLL directory to PATH: $SFML_DLL_DIR"
export PATH="$SFML_DLL_DIR:$PATH"

# Determinar la ruta del ejecutable
# En Visual Studio (generador multi-config) suele estar en una carpeta como Debug o Release.
EXECUTABLE_PATH="./$BUILD_DIR/Debug/$EXECUTABLE_NAME.exe"
if [ ! -f "$EXECUTABLE_PATH" ]; then
    # Si no se encuentra en Debug, probar directamente en $BUILD_DIR (para generadores monoconfiguración)
    EXECUTABLE_PATH="./$BUILD_DIR/$EXECUTABLE_NAME"
fi

# Ejecutar el programa si se encontró el ejecutable
if [ -f "$EXECUTABLE_PATH" ]; then
    echo "Running $EXECUTABLE_NAME..."
    "$EXECUTABLE_PATH"
else
    echo "Executable not found at $EXECUTABLE_PATH."
fi

echo "Build complete."
echo "Press any key to exit..."
read -n 1 -s
