#!/bin/bash

#---------- 0. Please Set Parameters

nbins=20
niters=500000 #500000
release="2021_UL17_summer20_25vars" # "2020_novenber_NoIsoCut"
burn_in_frac=0.1
nchains=3


mode=$1
#  possible modes:
#    start  - just clean and create folder
#    qcd    - find qcd normalization
#    hists  - produce all related root files with hists
#    sm     - sm analyse
#    fcnc   - both fcnc 1d analyse
#    unmarg - unmarg error
#    full   - full chain

package=$2
#  supported packages:
#    theta
#    cl
#    all

#---------- 1. Setup
myname=`basename "$0"`
echo "$myname, setup ... "
#source /cvmfs/cms.cern.ch/slc7_amd64_gcc530/cms/cmssw/CMSSW_8_1_0/external/slc7_amd64_gcc530/bin/root

source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.06.08/x86_64-slc6-gcc49-opt/root/bin/thisroot.sh
source /cvmfs/sft.cern.ch/lcg/contrib/gcc/4.9/x86_64-slc6-gcc49-opt/setup.sh

#export PATH=/cvmfs/cms.cern.ch/slc7_amd64_gcc530/cms/cmssw/CMSSW_8_1_0/external/slc7_amd64_gcc530/bin:$PATH
#export LD_LIBRARY_PATH=/cvmfs/cms.cern.ch/slc7_amd64_gcc530/cms/cmssw/CMSSW_8_1_0/external/slc7_amd64_gcc530/lib:$LD_LIBRARY_PATH
export PATH=/cvmfs/sft.cern.ch/lcg/external/Python/2.7.4/x86_64-slc6-gcc48-opt/bin:$PATH
export LD_LIBRARY_PATH=/cvmfs/sft.cern.ch/lcg/external/Python/2.7.4/x86_64-slc6-gcc48-opt/lib:$LD_LIBRARY_PATH
#export PYTHONPATH=/cvmfs/sft.cern.ch/lcg/external/Python/2.7.4/x86_64-slc6-gcc48-opt/bin
which python
which root

srcdir=`pwd`/scripts
cfgdir=$(pwd)
workdir=$(pwd)/../$release

set +e
set -o xtrace
#echo "$PATH"
if [ "$mode" = "start" ] || [ "$mode" = "full" ]; then
  echo "$myname, recreate work directory $workdir"
  rm -rf $workdir
  mkdir -p $workdir
  if [ "$mode" = "start" ]; then exit; fi
else echo "$myname, skip recreating work directory $workdir"; fi

cd $workdir

#---------- SYS IMPACT
if [ "$mode" = "sys_impact" ] || [ "$mode" = "full" ]; then
  package="sm"
  mkdir -p "$workdir/sys_check/" && cd "$_"
  if [ "$package" = "sm" ]; then
    mkdir -p "$workdir/sys_check/$package" && cd "$_"
    python $cfgdir/create_card.py --fname="sys_impact" --nbins=$nbins --niters=$niters --input="$workdir/hists/hists_SM.root" --mode="theta" --nchains=$nchains

    #for sys_name in "colourFlipUp" "erdOnUp" "QCDbasedUp"; do
    #  cd "$workdir/sys_check/$package/$sys_name/" && cd "$_"
    #  cp "../expected_sm_"$sys_name"_theta.cfg" .
    #  $srcdir/run_theta.sh "expected_sm_"$sys_name"_theta.cfg"
    #done;
    #exit

    mkdir -p "$workdir/sys_check/$package/expected" && cd "$_"
    cp ../expected_sm_theta.cfg .
    $srcdir/run_theta.sh expected_sm_theta.cfg

    for f in $workdir/sys_check/$package/expected_*_theta.cfg; do
      if [[ $f =~ expected_sm_(.*)_theta.cfg ]]; then  
        sys_name=${BASH_REMATCH[1]} 
        mkdir -p "$workdir/sys_check/$package/$sys_name" && cd "$_"
        cp ../*$sys_name*.cfg .
        $srcdir/run_theta.sh $f
        #root -q -b -l "$srcdir/getTable.cpp(\"expected_"$sys_name"_theta.root\", \"expected_theta\", $burn_in_frac)"
        #pdflatex -interaction=batchmode getTable_expected_theta.tex
        #pdflatex -interaction=batchmode model_expected_$sys_name.tex
      fi
    done
#    root -q -b -l "$cfgdir/plotSysImpact.C(\"$release\")"
  fi
  exit
fi

if [ "$mode" = "plot_sys" ]; then
  root -q -b -l "$cfgdir/plotSysImpact.C(\"$release\")"
fi


#---------- 2. Find QCD normalization
if [ "$mode" = "qcd" ] || [ "$mode" = "full" ]; then
  rm -rf "$workdir/qcd" && mkdir "$_" && cd "$_"
  echo "$myname, find QCD normalization ... "
  root -q -b -l "$cfgdir/tree_to_hists.C(\"QCD\",\""$release" SIG\",\"hists_QCD.root\", $nbins)"
  root -q -b -l "$srcdir/histsPlot.cpp(\"QCD_before\", \"hists_QCD.root\")"

  python $cfgdir/create_card.py --fname="qcd" --nbins=$nbins --input="hists_QCD.root" --mode="theta"
  $srcdir/run_theta.sh qcd_theta.cfg

  root -q -b -l "$srcdir/getQuantiles.cpp(\"qcd_theta.root\", \"f_Other\")"
  IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat getQuantiles_temp.txt`"
  echo "$myname, Other norm factors = $QCD_low $QCD_norm $QCD_upp ..."


  root -q -b -l "$srcdir/getQuantiles.cpp(\"qcd_theta.root\", \"f_QCD\")"
  IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat getQuantiles_temp.txt`"
  echo "$myname, QCD norm factors = $QCD_low $QCD_norm $QCD_upp ..."

  root -q -b -l "$srcdir/histsPlot.cpp(\"QCD_after\",\"hists_QCD.root\","$QCD_norm")"
else echo "$myname, skip qcd normalization calcullations"; fi

IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat $workdir/qcd/getQuantiles_temp.txt`"
echo "$myname, QCD norm factors = $QCD_low $QCD_norm $QCD_upp ..."

#---------- 3. Create histogramms file
make_hists(){
  nbins_=$1
  QCD_norm_=$2
  qcd_cut_=$3
  mode=$4

  root -q -b -l "$cfgdir/tree_to_hists.C(\"$mode\", \""$release" SIG\", \"hists_"$mode".root\", $nbins_, $QCD_norm_, $qcd_cut_)"
  root -q -b -l "$srcdir/histsPlot.cpp(\""$mode"_before\",\"hists_"$mode".root\")"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"_\", \"\", 1)"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"DIFF_\", \"diff\", 1)"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"DIFFPERCENT_\", \"diff percent\", 1)"
  return
}

if [ "$mode" = "hists" ] || [ "$mode" = "full" ]; then
  echo "$myname, create histogramms file ... "
  submode=$2
  if [ "$submode" = "sm" ] || [ "$submode" = "sm_all" ] || [ "$submode" = "all" ]; then
    mkdir -p "$workdir/hists" && cd "$_"
    make_hists $nbins $QCD_norm 0.70 SM
  fi
  if [ "$submode" = "sm_qcd" ] || [ "$submode" = "sm_all" ]  || [ "$submode" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/hists/QCD_"$QCD_qut && cd "$_"
      make_hists $nbins $QCD_norm $QCD_qut  SM
    done
  fi
  if [ "$submode" = "sm_bins" ] || [ "$submode" = "sm_all" ]  || [ "$submode" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do #1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
      mkdir -p "$workdir/hists/bins_"$nbins_alt && cd "$_"
      make_hists $nbins_alt $QCD_norm 0.70 SM
    done
  fi

  if [ "$package" = "fcnc" ] || [ "$submode" = "fcnc_all" ] || [ "$package" = "all" ]; then
    mkdir -p "$workdir/hists_fcnc" && cd "$_"
    make_hists $nbins $QCD_norm 0.70 FCNC_tcg
    make_hists $nbins $QCD_norm 0.70 FCNC_tug
  fi

  if [ "$submode" = "fcnc_qcd" ] || [ "$submode" = "fcnc_all" ]  || [ "$submode" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/hists_fcnc/QCD_"$QCD_qut && cd "$_"
      make_hists $nbins $QCD_norm $QCD_qut FCNC_tcg &
      make_hists $nbins $QCD_norm $QCD_qut FCNC_tug &
    done
    wait
  fi
  if [ "$submode" = "fcnc_bins" ] || [ "$submode" = "fcnc_all" ]  || [ "$submode" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do
      mkdir -p "$workdir/hists_fcnc/bins_"$nbins_alt && cd "$_"
      make_hists $nbins_alt $QCD_norm 0.70 FCNC_tcg &
      make_hists $nbins_alt $QCD_norm 0.70 FCNC_tug &
    done
    wait
  fi

  if [ "$mode" = "hists" ]; then exit; fi
else echo "$myname, skip recreating histogramms files"; fi

#---------- 4. Run SM analyse
make_analyse_theta(){
  
  input_hists=$1 # "$workdir/hists/hists_SM.root"
  nbins_=$2
  niters_=$3
  hist_path=$4
  mode=$5 # sm FcncTugModel FcncTcgModel

  #python $cfgdir/create_card.py --fname=$mode --nbins=$nbins_ --niters=$niters_ --input=$input_hists --mode="latex cl mRoot" --nchains=$nchains
  #$srcdir/run_theta.sh $mode"_theta.cfg"
  #$srcdir/run_cl.sh "combine sm_cl.txt -M MarkovChainMC -i 5000000 --tries 3 --saveChain --noSlimChain --burnInSteps 0 --noDefaultPrior=0 --setParameterRanges sigma_t_ch=-0.5,0.5:sigma_s_ch=-2.85,3.0:sigma_tW_ch=-6.0,1.5:sigma_ttbar=-4.0,5.0:sigma_Diboson=-3.0,3.0:sigma_DY=-3.0,3.0:sigma_WQQ=-1.0,4.5:sigma_Wc=-2.5,2.0:sigma_Wb=-2.5,3.5:sigma_Wother=-3.5,1.5:sigma_Wlight=-3.5,1.5:sigma_QCD=-3.0,1.5:lumi=-2.5,2.5 --freezeParameters r --redefineSignalPOIs sigma_t_ch"
  #$srcdir/run_cl.sh "combine sm_cl.txt -M MarkovChainMC -i 500000 --tries 3 --saveChain --noSlimChain --burnInSteps 0 --noDefaultPrior=0 --freezeParameters r --redefineSignalPOIs sigma_t_ch"
  #$srcdir/run_cl.sh "combine -M AsymptoticLimits sm_cl.txt --freezeParameters r --redefineSignalPOIs sigma_t_ch"
  #$srcdir/run_cl.sh "combine --help"
  POI="sigma_t_ch"
  if [ "$mode" = "FcncTugModel" ]; then
    POI="KU"
  fi
  if [ "$mode" = "FcncTcgModel" ]; then
    POI="KC"
  fi
  
  #root -q -b -l "$srcdir/CL_workspace_to_tree.cpp(\"higgsCombineTest.MarkovChainMC.mH120.root\", \""$mode"_theta.root\", \"sigma_s_ch:0.1 sigma_tW_ch:0.15 sigma_ttbar:0.15 sigma_Diboson:0.2 sigma_DY:0.2 sigma_WQQ:0.3 sigma_Wc:0.3 sigma_Wb:0.3 sigma_Wother:0.3 sigma_Wlight:0.3 sigma_QCD:1.0 lumi:0.025 sigma_t_ch:unif\")"
  #root -q -b -l "$srcdir/burnInStudy.cpp(\""$mode"_theta.root\", \"$POI\", \"BurnInStudy"$mode"Theta\")"
  #root -q -b -l "$srcdir/getPostHists.cpp(\"$input_hists\", \""$mode"_mroot.txt\", \""$mode"_theta.root\")"
  #root -q -b -l "$srcdir/histsPlot.cpp(\"SM_after\",\"postfit_hists/posthists.root\")"
  #root -q -b -l "$srcdir/histsChecker.cpp(\"$input_hists\",\"./postfit_hists/posthists.root\", \"SM_comp_\")"
  
  root -q -b -l "$srcdir/getTable.cpp(\""$mode"_theta.root\", \"$mode\", $burn_in_frac, \"$hist_path\")"
  pdflatex -interaction=batchmode getTable_$mode.tex
  pdflatex -interaction=batchmode model_$mode.tex
}

if [ "$mode" = "sm" ] || [ "$mode" = "full" ]; then
  echo "$myname, SM ... "
  if [ "$package" = "def" ] || [ "$package" = "all" ]; then
    mkdir -p "$workdir/sm/def" && cd "$_"
    make_analyse_theta "$workdir/hists/hists_SM.root" $nbins $niters "$workdir/hists/" sm
    mv getTable_SM.pdf table_sm_theta_def.pdf
  fi

  if [ "$package" = "qcd" ] || [ "$package" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/sm/QCD_"$QCD_qut && cd "$_"
      make_analyse_theta "$workdir/hists/QCD_"$QCD_qut"/hists_SM.root" $nbins $niters "$workdir/hists/" sm
      mv "$workdir/sm/QCD_"$QCD_qut"/sm_theta.root" "$workdir/sm/QCD_"$QCD_qut".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm/\", \"QCD_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_QCD_cut\")"
    # root -q -b -l "theta_13tev_global/plotResultsForDifferentConditions.cpp(\"$workdir/sm/\", \"QCD_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_QCD_cut\")"
  fi
  
  if [ "$package" = "bins" ] || [ "$package" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do 
    #1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
      mkdir -p "$workdir/sm/bins_"$nbins_alt && cd "$_"
      make_analyse_theta "$workdir/hists/bins_"$nbins_alt"/hists_SM.root" $nbins_alt $niters "$workdir/hists/" sm
      mv "$workdir/sm/bins_"$nbins_alt"/sm_theta.root" "$workdir/sm/bins_"$nbins_alt".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm/\", \"bins_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_nbins\")"
  fi

  exit
  if [ "$package" = "iters" ] || [ "$package" = "all" ]; then
    for iters in 50000 100000 200000 300000 400000 500000 750000 1000000 1500000 2000000; do
      mkdir -p "$workdir/sm/iters_"$iters && cd "$_"
      make_analyse_theta "$workdir/hists/hists_SM.root" $nbins $iters "$workdir/hists/" sm
      mv "$workdir/sm/iters_"$iters"/sm_theta.root" "$workdir/sm/iters_"$iters".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm/\", \"iters_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_MCMC_iters\")"
  fi

  if [ "$mode" = "sm" ]; then exit; fi
else echo "$myname, skip sm analyse"; fi

#---------- 5. Run FCNC analyse
set -x
if [ "$mode" = "fcnc" ] || [ "$mode" = "full" ]; then
  echo "$myname, FCNC ... "
  if [ "$package" = "def" ] || [ "$package" = "all" ]; then
    mkdir -p "$workdir/fcnc/def" && cd "$_"
    make_analyse_theta "$workdir/hists_fcnc/hists_FCNC_tug.root" $nbins $niters "$workdir/hists_fcnc/" "FcncTugModel"
    mv getTable_FcncTugModel.pdf table_KU_def.pdf

    make_analyse_theta "$workdir/hists_fcnc/hists_FCNC_tcg.root" $nbins $niters "$workdir/hists_fcnc/" "FcncTcgModel"
    mv getTable_FcncTcgModel.pdf table_KC_def.pdf
  fi

  if [ "$package" = "bins" ] || [ "$package" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do
      mkdir -p "$workdir/fcnc/bins_"$nbins_alt && cd "$_"
      make_analyse_theta "$workdir/hists_fcnc/bins_"$nbins_alt"/hists_FCNC_tug.root" $nbins_alt $niters "$workdir/hists_fcnc/" "FcncTugModel"
      mv "$workdir/fcnc/bins_"$nbins_alt"/FcncTugModel_theta.root" "$workdir/fcnc/bins_FcncTugModel_"$nbins_alt".root"

      make_analyse_theta "$workdir/hists_fcnc/bins_"$nbins_alt"/hists_FCNC_tcg.root" $nbins_alt $niters "$workdir/hists_fcnc/" "FcncTcgModel"
      mv "$workdir/fcnc/bins_"$nbins_alt"/FcncTcgModel_theta.root" "$workdir/fcnc/bins_FcncTcgModel_"$nbins_alt".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/fcnc/\", \"bins_FcncTugModel.+\.root\", \"KU\", \"KU_vs_nbins\")"
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/fcnc/\", \"bins_FcncTcgModel.+\.root\", \"KC\", \"KC_vs_nbins\")"
  fi

  if [ "$package" = "qcd" ] || [ "$package" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/fcnc/QCD_"$QCD_qut && cd "$_"
      make_analyse_theta "$workdir/hists_fcnc/hists_FCNC_tug.root" $nbins $niters "$workdir/hists_fcnc/" "FcncTugModel"
      mv getTable_FcncTugModel.pdf ../table_KU_QCD$QCD_qut.pdf
      mv "FcncTugModel_theta.root" "../QCD_KU_"$QCD_qut".root"

      make_analyse_theta "$workdir/hists_fcnc/hists_FCNC_tcg.root" $nbins $niters "$workdir/hists_fcnc/" "FcncTcgModel"
      mv getTable_FcncTcgModel.pdf ../table_KC_QCD$QCD_qut.pdf
      mv "FcncTcgModel_theta.root" "../QCD_KC_"$QCD_qut".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/fcnc/\", \"QCD_KC_.+\.root\", \"KC\", \"KC_vs_QCD_cut\")"
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/fcnc/\", \"QCD_KU_.+\.root\", \"KU\", \"KU_vs_QCD_cut\")"
  fi
else echo "$myname, skip fcnc analyse"; fi

if [ "$mode" = "fcnc_var" ] || [ "$mode" = "full" ]; then
  mkdir -p "$workdir/fcnc_var" && cd "$_"
  python $cfgdir/create_card.py --fname="fcnc_tug_expected_variation" --nbins=$nbins --niters=$niters --input="$workdir/hists/hists_FCNC_tug.root" --mode="latex theta" --nchains=$nchains
  
  for name in fcnc_*cfg; do
    echo $name
    $srcdir/run_theta.sh $name

    filename="${name%.*}"
    root -q -b -l "$srcdir/getTable.cpp(\"$filename.root\", \"$filename\", $burn_in_frac)"
    pdflatex -interaction=batchmode getTable_$filename.tex
  done;
fi

#---------- 6. Run 2D SM analyse


#---------- 6a. Find QCD normalization
if [ "$mode" = "qcd2d" ] || [ "$mode" = "full" ]; then
  rm -rf "$workdir/qcd" && mkdir "$_" && cd "$_"
  echo "$myname, find QCD normalization ... "
  root -q -b -l "$cfgdir/tree_to_hists.C(\"QCD\",\""$release" SIG\",\"hists_QCD.root\",$((nbins*nbins)))"
  root -q -b -l "$srcdir/histsPlot.cpp(\"QCD_before\", \"hists_QCD.root\")"

  python $cfgdir/create_card.py --fname="qcd" --nbins=$((nbins*nbins)) --input="hists_QCD.root" --mode="theta"
  $srcdir/run_theta.sh qcd_theta.cfg

  root -q -b -l "$srcdir/getQuantiles.cpp(\"qcd_theta.root\", \"f_Other\")"
  IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat getQuantiles_temp.txt`"
  echo "$myname, Other norm factors = $QCD_low $QCD_norm $QCD_upp ..."


  root -q -b -l "$srcdir/getQuantiles.cpp(\"qcd_theta.root\", \"f_QCD\")"
  IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat getQuantiles_temp.txt`"
  echo "$myname, QCD norm factors = $QCD_low $QCD_norm $QCD_upp ..."

  root -q -b -l "$srcdir/histsPlot.cpp(\"QCD_after\",\"hists_QCD.root\","$QCD_norm")"
else echo "$myname, skip qcd normalization calcullations"; fi

IFS=" " read QCD_low QCD_norm QCD_upp <<< "`cat $workdir/qcd/getQuantiles_temp.txt`"
echo "$myname, QCD norm factors = $QCD_low $QCD_norm $QCD_upp ..."

#---------- 6b. Create histogramms file
make_hists(){
  nbins_=$1
  QCD_norm_=$2
  qcd_cut_=$3
  mode=$4

  root -q -b -l "$cfgdir/tree_to_hists.C(\"$mode\", \""$release" SIG\", \"hists_"$mode".root\", $nbins_, $QCD_norm_, $qcd_cut_)"
  root -q -b -l "$srcdir/histsPlot.cpp(\""$mode"_before\",\"hists_"$mode".root\")"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"_\", \"\", 1)"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"DIFF_\", \"diff\", 1)"
  root -q -b -l "$srcdir/histsChecker.cpp(\"hists_"$mode".root\",\""$mode"DIFFPERCENT_\", \"diff percent\", 1)"
  return
}

if [ "$mode" = "hists2d" ] || [ "$mode" = "full" ]; then
  echo "$myname, create histogramms file ... "
  submode=$2
  if [ "$submode" = "sm" ] || [ "$submode" = "sm_all" ] || [ "$submode" = "all" ]; then
    mkdir -p "$workdir/hists2d" && cd "$_"
    make_hists $nbins $QCD_norm 0.70 SM2D
  fi
  if [ "$submode" = "sm_qcd" ] || [ "$submode" = "sm_all" ]  || [ "$submode" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/hists2d/QCD_"$QCD_qut && cd "$_"
      make_hists $nbins $QCD_norm $QCD_qut  SM2D
    done
  fi
  if [ "$submode" = "sm_bins" ] || [ "$submode" = "sm_all" ]  || [ "$submode" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10; do
      mkdir -p "$workdir/hists2d/bins_"$nbins_alt && cd "$_"
      make_hists $nbins_alt $QCD_norm 0.70 SM2D
    done
  fi

  if [ "$mode" = "hists2d" ]; then exit; fi
else echo "$myname, skip recreating histogramms files"; fi

#---------- 6c. Run SM analyse
make_analyse_theta(){
  input_hists=$1 # "$workdir/hists/hists_SM2D.root"
  nbins_=$2
  niters_=$3
  hist_path=$4
  mode=$5 # sm

  python $cfgdir/create_card.py --fname=$mode --nbins=$nbins_ --niters=$niters_ --input=$input_hists --mode="latex theta mRoot" --nchains=$nchains
  $srcdir/run_theta.sh $mode"_theta.cfg"

  POI="sigma_t_ch"

  root -q -b -l "$srcdir/burnInStudy.cpp(\""$mode"_theta.root\", \"$POI\", \"BurnInStudy"$mode"Theta\")"
  root -q -b -l "$srcdir/getPostHists.cpp(\"$input_hists\", \""$mode"_mroot.txt\", \""$mode"_theta.root\")"
  root -q -b -l "$srcdir/histsPlot.cpp(\"SM_after\",\"postfit_hists/posthists.root\")"
  root -q -b -l "$srcdir/histsChecker.cpp(\"$input_hists\",\"./postfit_hists/posthists.root\", \"SM_comp_\")"

  root -q -b -l "$srcdir/getTable.cpp(\""$mode"_theta.root\", \"$mode\", $burn_in_frac, \"$hist_path\")"
  pdflatex -interaction=batchmode getTable_$mode.tex
  pdflatex -interaction=batchmode model_$mode.tex
}

if [ "$mode" = "sm2d" ] || [ "$mode" = "full" ]; then
  echo "$myname, SM2D  ... "
  if [ "$package" = "def" ] || [ "$package" = "all" ]; then
    mkdir -p "$workdir/sm2d/def" && cd "$_"
    make_analyse_theta "$workdir/hists2d/hists_SM2D.root" $((nbins*nbins)) $niters "$workdir/hists2d/" sm
    mv getTable_SM.pdf table_sm_theta_def.pdf
  fi

  if [ "$package" = "qcd" ] || [ "$package" = "all" ]; then
    for QCD_qut in "0.50" "0.55" "0.60" "0.65" "0.70" "0.75" "0.80" "0.85" "0.90" "0.95"; do
      mkdir -p "$workdir/sm2d/QCD_"$QCD_qut && cd "$_"
      make_analyse_theta "$workdir/hists2d/QCD_"$QCD_qut"/hists_SM2D.root" $((nbins*nbins)) $niters "$workdir/hists2d/" sm
      mv "$workdir/sm2d/QCD_"$QCD_qut"/sm_theta.root" "$workdir/sm2d/QCD_"$QCD_qut".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm2d/\", \"QCD_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_QCD_cut\")"
    # root -q -b -l "theta_13tev_global/plotResultsForDifferentConditions.cpp(\"$workdir/sm/\", \"QCD_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_QCD_cut\")"
  fi

  if [ "$package" = "bins" ] || [ "$package" = "all" ]; then
    for nbins_alt in 1 2 3 4 5 6 7 8 9 10; do
      mkdir -p "$workdir/sm2d/bins_"$nbins_alt && cd "$_"
      make_analyse_theta "$workdir/hists2d/bins_"$nbins_alt"/hists_SM2D.root" $((nbins_alt*nbins_alt)) $niters "$workdir/hists2d/" sm
      mv "$workdir/sm2d/bins_"$nbins_alt"/sm_theta.root" "$workdir/sm2d/bins_"$nbins_alt".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm2d/\", \"bins_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_nbins\")"
  fi

  exit
  if [ "$package" = "iters" ] || [ "$package" = "all" ]; then
    for iters in 50000 100000 200000 300000 400000 500000 750000 1000000 1500000 2000000; do
      mkdir -p "$workdir/sm2d/iters_"$iters && cd "$_"
      make_analyse_theta "$workdir/hists2d/hists_SM2D.root" $((nbins*nbins)) $iters "$workdir/hists2d/" sm
      mv "$workdir/sm2d/iters_"$iters"/sm_theta.root" "$workdir/sm2d/iters_"$iters".root"
    done
    root -q -b -l "$srcdir/plotResultsForDifferentConditions.cpp(\"$workdir/sm2d/\", \"iters_.+\.root\", \"sigma_t_ch\", \"t_ch_vs_MCMC_iters\")"
  fi

  if [ "$mode" = "sm2d" ]; then exit; fi
else echo "$myname, skip sm analyse"; fi

if [ "$mode" = "sys_impact2d" ] || [ "$mode" = "full" ]; then
  package="sm"
  mkdir -p "$workdir/sys_check/" && cd "$_"
  if [ "$package" = "sm" ]; then
    mkdir -p "$workdir/sys_check/$package" && cd "$_"
    python $cfgdir/create_card.py --fname="sys_impact2d" --nbins=$((nbins*nbins)) --niters=$niters --input="$workdir/hists2d/hists_SM2D.root" --mode="theta" --nchains=$nchains

    #for sys_name in "colourFlipUp" "erdOnUp" "QCDbasedUp"; do
    #  cd "$workdir/sys_check/$package/$sys_name/" && cd "$_"
    #  cp "../expected_sm_"$sys_name"_theta.cfg" .
    #  $srcdir/run_theta.sh "expected_sm_"$sys_name"_theta.cfg"
    #done;
    #exit

    mkdir -p "$workdir/sys_check/$package/expected" && cd "$_"
    cp ../expected_sm_theta.cfg .
    $srcdir/run_theta.sh expected_sm_theta.cfg

    for f in $workdir/sys_check/$package/expected_*_theta.cfg; do
      if [[ $f =~ expected_sm_(.*)_theta.cfg ]]; then  
        sys_name=${BASH_REMATCH[1]} 
        mkdir -p "$workdir/sys_check/$package/$sys_name" && cd "$_"
        cp ../*$sys_name*.cfg .
        $srcdir/run_theta.sh $f
        #root -q -b -l "$srcdir/getTable.cpp(\"expected_"$sys_name"_theta.root\", \"expected_theta\", $burn_in_frac)"
        #pdflatex -interaction=batchmode getTable_expected_theta.tex
        #pdflatex -interaction=batchmode model_expected_$sys_name.tex
      fi
    done
#    root -q -b -l "$cfgdir/plotSysImpact.C(\"$release\")"
  fi
  exit
fi
















