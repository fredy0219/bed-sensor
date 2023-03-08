# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ff/esp4.42/esp-idf/components/bootloader/subproject"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix/tmp"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix/src"
  "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ff/Desktop/BedSensor/bed-sensor/bed-sensor-firmware/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
