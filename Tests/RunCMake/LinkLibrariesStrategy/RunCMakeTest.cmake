cmake_minimum_required(VERSION 3.30)

include(RunCMake)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
else()
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
endif()

# Detect information from the toolchain:
# - CMAKE_C_LINK_LIBRARIES_PROCESSING
# - CMAKE_C_PLATFORM_LINKER_ID
# - CMAKE_EXECUTABLE_FORMAT
run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(Unknown)

function(run_strategy case exe)
  foreach(cmp0179 ${cmp0179_states})
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-CMP0179-${cmp0179}-build)
    set(RunCMake_TEST_VARIANT_DESCRIPTION "...CMP0179-${cmp0179}")
    if("DEDUPLICATION=ALL" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
      if("ORDER=REVERSE" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING AND cmp0179 STREQUAL "OLD")
        set(RunCMake-stderr-file ${case}-stderr-dedup-reverse.txt)
      else()
        set(RunCMake-stderr-file ${case}-stderr-dedup.txt)
      endif()
    endif()
    run_cmake_with_options(${case} -DCMAKE_POLICY_DEFAULT_CMP0179=${cmp0179})
    unset(RunCMake-stderr-file)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
    if(exe)
      if("DEDUPLICATION=ALL" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING)
        if("ORDER=REVERSE" IN_LIST CMAKE_C_LINK_LIBRARIES_PROCESSING AND cmp0179 STREQUAL "OLD")
          set(RunCMake-stdout-file ${case}-run-stdout-dedup-reverse.txt)
        else()
          set(RunCMake-stdout-file ${case}-run-stdout-dedup.txt)
        endif()
      endif()
      run_cmake_command(${case}-run ${RunCMake_TEST_BINARY_DIR}/${exe})
      unset(RunCMake-stdout-file)
    endif()
  endforeach()
endfunction()

if(CMAKE_C_PLATFORM_LINKER_ID STREQUAL "LLD" AND CMAKE_EXECUTABLE_FORMAT STREQUAL "ELF")
  # deduplication done only if both CMP0156 and CMP0179 ARE NEW
  set(cmp0179_states NEW)
else()
  set(cmp0179_states OLD NEW)
endif()

run_strategy(Basic-REORDER_MINIMALLY "main")
run_strategy(Basic-REORDER_FREELY "main")

run_cmake(Duplicate-REORDER_MINIMALLY)
run_cmake(Duplicate-REORDER_FREELY)
