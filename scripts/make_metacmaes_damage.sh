#!/bin/bash

export BUILD_GRAPHIC="False"
export BUILD_EVAL_ENVIR="False"
cd $SFERES_DIR
./waf distclean
./waf configure 
./waf
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
