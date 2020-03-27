#!/bin/bash

cd $SFERES_DIR
./waf configure
./waf
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
