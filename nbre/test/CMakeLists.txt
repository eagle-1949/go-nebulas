include(GoogleTest)

#set(common_libs ${common_libs} gtest)
#add_subdirectory(link_example)
if(UNIX AND NOT APPLE)
  set(gtest_lib ${PROJECT_SOURCE_DIR}/lib/lib/libgtest.so)
elseif(UNIX)
  set(gtest_lib ${PROJECT_SOURCE_DIR}/lib/lib/libgtest.dylib)
endif()
add_subdirectory(common)
add_subdirectory(fs)
add_subdirectory(core)
add_subdirectory(jit)
add_subdirectory(crypto)
add_subdirectory(util)
add_subdirectory(runtime)
