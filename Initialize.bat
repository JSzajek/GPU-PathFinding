@echo off

call git init
call git submodule add https://github.com/JSzajek/OpenCL-Lib-fork.git "vendor/opencl_lib/"
call git submodule add https://github.com/JSzajek/OpenCV-Lib-fork.git "vendor/opencv_lib/"
PAUSE