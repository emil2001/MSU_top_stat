#!/bin/bash

#All steps required for SM statanalysis

echo "Starting the run ..."

sh ./analyse.sh start
sh ./analyse.sh qcd

sh ./analyse.sh hists sm theta low_level
sh ./analyse.sh sm def theta low_level

sh ./analyse.sh hists sm theta super
sh ./analyse.sh sm def theta super

#sh ./analyse.sh hists2d sm theta low_level
#sh ./analyse.sh sm2d def theta low_level

echo "Run completed"