
find_package(PythonInterp 3.6 REQUIRED)
find_package(PythonLibs 3.6 REQUIRED)

message("PYTHONLIBS_FOUND:${PYTHONLIBS_FOUND}")
message("PYTHON_VERSION_STRING:${PYTHON_VERSION_STRING}")
message("PYTHON_INCLUDE_PATH:${PYTHON_INCLUDE_PATH}")
message("PYTHON_LIBRARIES:${PYTHON_LIBRARIES}")

include_directories(${PYTHON_INCLUDE_PATH})

add_library(leelapy SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/advisor.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/PyLeela.cpp)
#set_property(TARGET leelapy PROPERTY POSITION_INDEPENDENT_CODE ON)

add_dependencies(leelapy objs)

target_link_libraries(leelapy $<TARGET_OBJECTS:objs>)
target_link_libraries(leelapy boost_python3 ${PYTHON_LIBRARIES})
target_link_libraries(leelapy ${Boost_LIBRARIES})
target_link_libraries(leelapy ${BLAS_LIBRARIES})
target_link_libraries(leelapy ${OpenCL_LIBRARIES})
target_link_libraries(leelapy ${ZLIB_LIBRARIES})
target_link_libraries(leelapy ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(leelapy PROPERTIES PREFIX "")
set_target_properties(leelapy PROPERTIES SUFFIX ".so")
