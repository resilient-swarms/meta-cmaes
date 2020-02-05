#!/bin/bash

cd $SFERES_DIR
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
