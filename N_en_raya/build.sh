#!/bin/bash
# Este script configura y compila el proyecto usando CMake en modo Release.
# Funciona tanto en Windows como en Linux (por ejemplo, Linux Mint).
# No se modifica la funcionalidad: se crea la carpeta de build, se configura con CMake,
# se compila, se añade la carpeta de las DLL de SFML (solo en Windows) y se ejecuta el programa.
#
# Requisitos:
#   - Para Windows: el ejecutable se llama MySFMLApp.exe y se asume que las DLL de SFML se encuentran en
#     ./build/_deps/sfml-build/lib/Debug (ajusta esta ruta si es necesario).
#   - Para Linux: el ejecutable se llamará MySFMLApp (sin extensión), y se asume que las bibliotecas se enlazan
#     adecuadamente (por ejemplo, instaladas en el sistema o enlazadas estáticamente).

# Configuración
EXECUTABLE_NAME="MySFMLApp"
BUILD_DIR="build"

# Crear la carpeta de build si no existe
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creando directorio $BUILD_DIR ..."
    mkdir "$BUILD_DIR"
fi

# Configurar el proyecto con CMake en modo Release
echo "Configurando el proyecto..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "La configuración con CMake falló. Presiona cualquier tecla para salir..."
    read -n 1 -s
    exit 1
fi

# Compilar el proyecto
echo "Compilando el proyecto..."
cmake --build "$BUILD_DIR"
if [ $? -ne 0 ]; then
    echo "La compilación falló. Presiona cualquier tecla para salir..."
    read -n 1 -s
    exit 1
fi

# Detectar el sistema operativo usando uname
OS_TYPE=$(uname)
if [[ "$OS_TYPE" == "Linux" ]]; then
    # En Linux se asume que el ejecutable no tiene extensión
    EXECUTABLE_PATH="./$BUILD_DIR/$EXECUTABLE_NAME"
else
    # En Windows, se añade la carpeta de las DLL de SFML al PATH
    SFML_DLL_DIR="./build/_deps/sfml-build/lib/Debug"
    echo "Agregando directorio de DLL de SFML al PATH: $SFML_DLL_DIR"
    export PATH="$SFML_DLL_DIR:$PATH"
    
    EXECUTABLE_PATH="./$BUILD_DIR/Debug/$EXECUTABLE_NAME.exe"
    if [ ! -f "$EXECUTABLE_PATH" ]; then
        EXECUTABLE_PATH="./$BUILD_DIR/$EXECUTABLE_NAME"
    fi
fi

# Ejecutar el programa si se encontró el ejecutable
if [ -f "$EXECUTABLE_PATH" ]; then
    echo "Ejecutando $EXECUTABLE_NAME..."
    "$EXECUTABLE_PATH"
else
    echo "Ejecutable no encontrado en $EXECUTABLE_PATH."
fi

echo "Build complete. Presiona cualquier tecla para salir..."
read -n 1 -s
