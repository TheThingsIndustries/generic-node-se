# app_template

## Description

This application provides a bloat-free template for starting with application development.

Developers are expected to customize this application, add their source files and build configurations according to their needs.

## Configuration

This application does not require any additional configurations to function.
Any application specific configurations should be added to [`conf/app_conf.h`](./conf/app_conf.h).

## Setup

This application does not require any additional setup as it acts as a template.
You can add more functionality to this application by following the steps below:

1. Add your source files in [`app_template`](.)
2. Add your library paths to [`CMakeLists.txt`](./CMakeLists.txt) and STM32CubeIDE [`.cproject`](./.cproject),[`.project`](./.project) similar to`GNSE_BSP`library

> **Note:** Keep in mind that you can also see how this is done in the other example applications located in [`app`](./..) folder.

If you are satisfied with the application behavior, you can rename it and add the new app name to [`CMakeLists.txt`](./../../CMakeLists.txt) under `APP_LIST`.

> Please remember to adjust the paths of the source files according to your new application name.

## Observation

Since this application acts as a template, it simply blinks the RGB LEDs.
