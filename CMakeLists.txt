# ======================================================================
#  larcorealg main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/larcorealg/ups/setup_for_development <-d|-p>
#  buildtool [-I /install/path] -bt[i] -jN
# ======================================================================


# use cmake 3.14 or later
cmake_minimum_required (VERSION 3.12...3.14 FATAL_ERROR)

project(lardataalg VERSION 08.04.02 LANGUAGES CXX)
set(fcl_dir job)
find_package( cetmodules )
include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS CAUTIOUS
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs
)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_package( ROOT REQUIRED COMPONENTS Core EG Hist)
find_package(Boost REQUIRED COMPONENTS date_time filesystem graph program_options regex system unit_test_framework)
find_package( lardataobj REQUIRED)
find_package( larcorealg REQUIRED)

# macros for artdaq_dictionary and simple_plugin
include(ArtDictionary)
include(CetMake)
include(BasicPlugin)

# source
add_subdirectory(lardataalg)

# tests
add_subdirectory(test)

add_subdirectory(cmakeConfig)
