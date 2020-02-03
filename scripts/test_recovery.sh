#!/bin/bash

DEST=$1 # destination folder

for method in meta; do
  for test_type in envir; do
    
    
    if [[ "${method}" == "meta" ]]; then
      if [[ "${test_type}" == "envir" ]]; then
        last_gen=270  # leads to 2.8M evals
      else
        last_gen=260  # leads to 2.8M evals
      fi
      method_tag="${test_type}_${method}"
      binary_tag="${test_type}_${method}"
    else
      method_tag="${method}"
      binary_tag="${test_type}_control"
      last_gen=7000  # leads to 2.8M evals
    fi
    for replicate in 1 2 3; do
      echo "start doing ${method}, ${test_type}, run ${replicate}"
      Outfolder="${DEST}/${method_tag}/exp${replicate}"
      echo "will write to ${Outfolder}"

      ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/test_${binary_tag}_binary --load ${Outfolder}/gen_${last_gen} --d ${Outfolder} -o ${Outfolder}/${test_type}_performance &
    done

  done

done
