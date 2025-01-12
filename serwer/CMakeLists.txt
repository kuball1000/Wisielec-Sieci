# Minimalna wymagana wersja CMake
cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(NewServerCopy)

# Ustawienie flag kompilatora
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Plik źródłowy
set(SOURCE_FILE newSerwerCopy.cpp)

# Wygenerowanie pliku wykonywalnego
add_executable(serwer ${SOURCE_FILE})
