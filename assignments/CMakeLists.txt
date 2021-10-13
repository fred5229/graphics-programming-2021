# obtain the list of subdirectories
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------------
# Executable and target include/link libraries
# ---------------------------------------------------------------------------------
# list of libraries
set(libraries glad glfw)

# add all subdirectories (it expects a CMakeLists.txt file in all of them)
FOREACH(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
ENDFOREACH()