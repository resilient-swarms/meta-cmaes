#!/bin/bash

export BUILD_GRAPHIC="False"
export BUILD_EVAL_ENVIR="True"
cd $SFERES_DIR
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
