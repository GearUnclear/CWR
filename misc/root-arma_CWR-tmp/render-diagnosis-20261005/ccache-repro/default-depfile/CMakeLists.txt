cmake_minimum_required(VERSION 3.25)
project(DependencyProbe LANGUAGES C CXX)
add_library(probe OBJECT probe.cpp probe.c)
