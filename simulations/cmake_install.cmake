# Install script for directory: /Users/max/project/hybrid_fem_bie_papers/simulations

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/parallel_faults/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/tpv14_2d/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/tpv5_2d/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/biegel_exp/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/plasticity/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/find_steady_state/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/biegle_exp_exodus/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/fish_bone/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/fish_bone_plasticity/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/fish_bone_homo/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/fish_bone_bie_domain/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/fish_bone_homo_plasticity/cmake_install.cmake")
  include("/Users/max/project/hybrid_fem_bie_plane_stress/simulations/rate_and_state_generated/cmake_install.cmake")

endif()

