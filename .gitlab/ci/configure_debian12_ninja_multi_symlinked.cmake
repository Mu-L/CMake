if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_IAR_TOOLCHAINS "/opt/iarsystems" CACHE PATH "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_symlinked_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_debian12_ninja_common.cmake")
set(CMake_TEST_UseSWIG "OFF" CACHE BOOL "")
