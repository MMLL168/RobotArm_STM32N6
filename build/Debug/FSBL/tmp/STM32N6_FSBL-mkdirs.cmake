# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Work_202506/RobotArm/STM32N6/FSBL")
  file(MAKE_DIRECTORY "D:/Work_202506/RobotArm/STM32N6/FSBL")
endif()
file(MAKE_DIRECTORY
  "D:/Work_202506/RobotArm/STM32N6/FSBL/build"
  "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL"
  "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/tmp"
  "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/src/STM32N6_FSBL-stamp"
  "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/src"
  "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/src/STM32N6_FSBL-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/src/STM32N6_FSBL-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Work_202506/RobotArm/STM32N6/build/Debug/FSBL/src/STM32N6_FSBL-stamp${cfgdir}") # cfgdir has leading slash
endif()
