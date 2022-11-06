#!/bin/bash

#All steps required for FCNC statanalysis

echo "Starting the run ..."

#sh ./analyse.sh start   #probably don't need this, since you have done SM analysis first, have you?
#sh ./analyse.sh qcd

sh ./analyse.sh hists fcnc theta low_level
sh ./analyse.sh fcnc def theta low_level

sh ./analyse.sh hists fcnc theta super
sh ./analyse.sh fcnc def theta super

#sh ./analyse.sh hists2d sm theta low_level
#sh ./analyse.sh sm2d def theta low_level

echo "Run completed"
