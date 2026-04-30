# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "D:\\Work_202506\\RobotArm\\STM32N6\\Appli\\build"
  "D:\\Work_202506\\RobotArm\\STM32N6\\FSBL\\build"
  )
endif()
