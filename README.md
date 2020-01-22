# meta-cmaes
learning behaviour-performance map with meta-cmaes


installation instructions:

1. Go to https://github.com/AranBSmith/MAP-Elites-Rhex and follow the instructions.

2. Set the correct values for the environment variables ${RESIBOTS_DIR} and ${SFERES_DIR} in your .bashrc file

3. Go to the meta-cmaes folder, and copy the files in 'copy_to_exp' to the your sferes experiment folder:

cd ${RESIBOTS_DIR}/include/meta-cmaes
cp copy_to_exp/* ${SFERES_DIR}/exp/MAP-Elites-Rhex

4. A. Compiling the META-learning conditions:

    export BUILD_META=True

   B. Compiling the control conditions does not require this export, or you can be explicit and write:

    export BUILD_META=False 


5. Set the number of cores you are using, e.h.:

    export NUM_CORES=16

6. To compile, do:

    bash scripts/make.sh


   This will generate either 2 binaries (the two meta-learning conditions) or 4 binaries (the control-conditions), depending on BUILD_META.


7. To run, use the binary path obtained from the compilation, and add the replicate_number and the folder where you want to store the results.

   Please use the following template for the different conditions:

    i) DAMAGE_META:
        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_damage_meta_binary ${replicate_number} --d ${RESULTS_DIR}/damage_meta/exp${replicate_number}

    ii) ENVIR_META:

        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_damage_envir_binary ${replicate_number} --d ${RESULTS_DIR}/envir_meta/exp${replicate_number}
    

    iii) RANDOM:

        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_random_binary ${replicate_number} --d ${RESULTS_DIR}/random/exp${replicate_number}

    iv) DUTY_CYCLE:

        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_duty_binary ${replicate_number} --d ${RESULTS_DIR}/envir_meta/exp${replicate_number}

    
    v) BODY_ORIENTATION:

        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_bo_binary ${replicate_number} --d ${RESULTS_DIR}/envir_meta/exp${replicate_number}

        
    vi) LINEAR_VELOCITY:

        ${SFERES_DIR}/build/exp/MAP-Elites-Rhex/rhex_metaCMAES_lv_binary ${replicate_number} --d ${RESULTS_DIR}/envir_meta/exp${replicate_number}