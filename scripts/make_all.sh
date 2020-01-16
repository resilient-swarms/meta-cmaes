#!/bin/bash

export BUILD_ALL="True"
cd $SFERES_DIR
./waf distclean
./waf configure 
./waf
./waf configure --exp MAP-Elites-Rhex 
./waf --exp MAP-Elites-Rhex 
