#!/bin/bash


cd
cd sferes2
./waf distclean
./waf configure
./waf
./waf configure --exp MAP-Elites-Rhex
./waf --exp MAP-Elites-Rhex
