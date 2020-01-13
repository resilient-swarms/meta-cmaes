#!/bin/bash


cd 
cd rhex_common/rhex_models
./waf distclean
./waf configure --prefix=$RESIBOTS_DIR
./waf install
cd ../rhex_controller
./waf distclean
./waf configure --prefix=$RESIBOTS_DIR
./waf
./waf install
cd
cd rhex_simu/rhex_dart
./waf distclean
./waf configure --prefix=$RESIBOTS_DIR
./waf
./waf install
cd
cd sferes2
./waf distclean
./waf configure
./waf
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
