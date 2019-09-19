#!/bin/sh
#cd ${0%/*} || exit 1    # run from this directory

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/CleanFunctions
source $CFDEM_PROJECT_DIR/etc/functions.sh

#- define variables
casePath="$(dirname "$(readlink -f ${BASH_SOURCE[0]})")"
cd $casePath/CFD
cleanCase
# rm $casePath/CFD/rmass*.dat
#rm $casePath/CFD/*.dat
#rm $casePath/CFD/*.txt
rm $casePath/log*
#rm $casePath/run_error.log
#cp -R 0/Org/p 0/p
#cp -R 0/Org/U 0/U
#cp -R 0/Org/Us 0/Us
#cp -R 0/Org/phiIB 0/phiIB
#cp -R 0/Org/voidfraction 0/voidfraction

rm -R $casePath/DEM/post
mkdir $casePath/DEM/post
mkdir $casePath/DEM/post/restart
#cd $casePath/DEM/post/restart 
#touch liggghts.restart

## if postproc is activated (in fix_chem_shrink) 
#rm $casePath/changeOfCO2
#rm $casePath/changeOfO2
#rm $casePath/rhogas_
#rm $casePath/pmass_

# ----------------------------------------------------------------- end-of-file
