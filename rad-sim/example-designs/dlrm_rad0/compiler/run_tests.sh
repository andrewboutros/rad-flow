#!/bin/sh

RUN="mvm16-flitw$FLITW-vcbuf$VCBUF-$MODEL"
ROOT_DIR="/home/andrew/repos/rad-flow-dev/"

cd $ROOT_DIR
for FLITW in 131
do
  for VCBUF in 8
  do

    if [ $FLITW -eq 131 ]
    then
      sed -i 's/noc_payload_width = \[[0-9]*\]/noc_payload_width = \[82\]/g' rad-flow.config 
    elif [ $FLITW -eq 195 ]
    then
      sed -i 's/noc_payload_width = \[[0-9]*\]/noc_payload_width = \[146\]/g' rad-flow.config 
    else
      sed -i 's/noc_payload_width = \[[0-9]*\]/noc_payload_width = \[274\]/g' rad-flow.config 
    fi

    sed -i 's/noc_vc_buffer_size = \[[0-9]*\]/noc_vc_buffer_size = \['$VCBUF'\]/g' rad-flow.config 
    cd scripts
    python config.py

    for MVM in 16 32
    do
      for MODEL in small
      do
        for INPUTS in 1 256
        do
          RUN="mvm$MVM-flitw$FLITW-vcbuf$VCBUF-$MODEL-$INPUTS"
          echo "$RUN"

          cd $ROOT_DIR/rad-sim/example-designs/dlrm/compiler
          python dlrm.py -l $MVM -n $INPUTS -m ab_$MODEL.csv

          cd $ROOT_DIR/rad-sim/build
          make >> make.log
          ./sim/build/system

          cd $ROOT_DIR/rad-sim/logs
          mkdir $ROOT_DIR/rad-sim/example-designs/dlrm/compiler/reports/dramsim_logs/$RUN
          cp *.txt $ROOT_DIR/rad-sim/example-designs/dlrm/compiler/reports/dramsim_logs/$RUN/

          cd $ROOT_DIR/rad-sim/example-designs/dlrm/compiler
          python plot.py $ROOT_DIR/rad-sim/example-designs/dlrm/compiler/reports/plots/$RUN
        done
      done
    done
  done
done