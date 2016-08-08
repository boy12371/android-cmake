cmake_minimum_required(VERSION 3.6.0)
project(AndroidGradleBuild C CXX)

# Pretend we're building for Android.
set(CMAKE_SYSTEM_NAME      Android CACHE "" INTERNAL FORCE)
set(CMAKE_BUILD_TYPE       Debug   CACHE "" INTERNAL FORCE)
set(CMAKE_ANDROID_ARCH_ABI x86_64  CACHE "" INTERNAL FORCE)
# Need to use a real sysroot, so it still compiles.
set(CMAKE_SYSROOT          /       CACHE "" INTERNAL FORCE)

# Force Windows build to generate .lib files.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set(CMAKE_C_COMPILER_ID         ${CMAKE_C_COMPILER_ID}         CACHE "" INTERNAL FORCE)
set(CMAKE_EXECUTABLE_SUFFIX     ${CMAKE_EXECUTABLE_SUFFIX}     CACHE "" INTERNAL FORCE)
set(CMAKE_SHARED_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX} CACHE "" INTERNAL FORCE)
set(CMAKE_SHARED_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE "" INTERNAL FORCE)
set(CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX} CACHE "" INTERNAL FORCE)
set(CMAKE_STATIC_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX} CACHE "" INTERNAL FORCE)
set(CMAKE_SHARED_MODULE_PREFIX  ${CMAKE_SHARED_MODULE_PREFIX}  CACHE "" INTERNAL FORCE)
set(CMAKE_SHARED_MODULE_SUFFIX  ${CMAKE_SHARED_MODULE_SUFFIX}  CACHE "" INTERNAL FORCE)

set(CMAKE_C_FLAGS           -DC_FLAGS)
set(CMAKE_CXX_FLAGS         -DCXX_FLAGS)
set(CMAKE_C_FLAGS_DEBUG     -DC_DEBUG_FLAGS)
set(CMAKE_CXX_FLAGS_DEBUG   -DCXX_DEBUG_FLAGS)
set(CMAKE_C_FLAGS_RELEASE   -DC_RELEASE_FLAGS)
set(CMAKE_CXX_FLAGS_RELEASE -DCXX_RELEASE_FLAGS)

add_definitions(-DDEFINITION)
include_directories(static shared module object)

add_subdirectory(shared)
add_subdirectory(static)
add_subdirectory(module)
add_subdirectory(object)
set(C_LIBRARIES c_shared c_static)
set(CXX_LIBRARIES cxx_shared cxx_static)

add_executable(c_exe main.c exe.c
  $<TARGET_OBJECTS:c_object>)
target_link_libraries(c_exe ${C_LIBRARIES})

add_executable(cxx_exe main.cxx exe.cxx
  $<TARGET_OBJECTS:cxx_object>)
target_link_libraries(cxx_exe ${C_LIBRARIES} ${CXX_LIBRARIES})

add_library(missing SHARED IMPORTED)
add_library(imported SHARED IMPORTED)
set_target_properties(imported PROPERTIES
  IMPORTED_LOCATION /fake/location/libimported.so)
