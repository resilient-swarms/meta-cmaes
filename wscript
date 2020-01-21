#! /usr/bin/env python
import sys
sys.path.insert(0, sys.path[0]+'/waf_tools')

import os
import sferes
import rhex_dart



def options(opt):
    opt.load('rhex_dart')

def configure(conf):
    #conf.get_env()['BUILD_GRAPHIC'] = False

    conf.load('rhex_dart')
    conf.check_rhex_dart()
  


			# sferes.create_variants(bld,
		    #     source = 'rhex_metaCMAES.cpp',
		    #     use = 'sferes2',
		    #     uselib = 'DART_GRAPHIC ' + libs,
		    #     target = 'rhex_metaCMAES_graphic_envir_'+c,
		    #     cxxflags = cxxflags + ['-g', '-march=native',"-DGRAPHIC","-DEVAL_ENVIR","-DEXPERIMENT_TYPE="+str(i)],   # -march=native
		    #             variants = ['BINARY'])
			# sferes.create_variants(bld,
		    #                source = 'rhex_metaCMAES.cpp',
		    #                use = 'sferes2',
		    #                uselib = 'DART_GRAPHIC ' + libs,
		    #                target = 'rhex_metaCMAES_graphic_damage_'+c,
		    #                cxxflags = cxxflags + ['-g', '-march=native',"-DGRAPHIC","-DEXPERIMENT_TYPE="+str(i)],   # -march=native
		    #                variants = ['BINARY'])

def build(bld):
    libs = 'RHEX_DART RHEX_CONTROLLER DART EIGEN BOOST_DART BOOST TBB '

    cxxflags = bld.get_env()['CXXFLAGS']
    tag=""
    conditions=["meta","random","duty","bo","lv"]
    if os.environ.get('BUILD_ENVIR',False) == "True":
        for i,c in enumerate(conditions):
			sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = libs,
		                   target = 'rhex_metaCMAES_envir_'+c,
		                   cxxflags = cxxflags + ["-DGRAPHIC", '-g', '-march=native',"-DEVAL_ENVIR","-DEXPERIMENT_TYPE="+str(i)],   # -march=native
		                   variants = ['BINARY'])
    elif os.environ.get('BUILD_DAMAGE',False) == "True":
        for i,c in enumerate(conditions):
			sferes.create_variants(bld,
							source = 'rhex_metaCMAES.cpp',
							use = 'sferes2',
							uselib = libs,
							target = 'rhex_metaCMAES_damage_'+c,
							cxxflags = cxxflags + ['-g', '-march=native',"-DEXPERIMENT_TYPE="+str(i)],   # -march=native
							variants = ['BINARY'])
    # else:
	#     if os.environ.get('BUILD_GRAPHIC',False) == "True":
	#     	cxxflags+=["-DGRAPHIC"]
	# 	tag+="_graphic"
	# 	libs = 'DART_GRAPHIC ' + libs
	#     if os.environ.get('BUILD_EVAL_ENVIR',False)  == "True":
	#     	cxxflags+=["-DEVAL_ENVIR"]
	# 	tag+="_envir"
	#     else:
	# 	tag+="_damage"
	#     sferes.create_variants(bld,
	# 	                   source = 'rhex_metaCMAES.cpp',
	# 	                   use = 'sferes2',
	# 	                   uselib = libs,
	# 	                   target = 'rhex_metaCMAES'+tag,
	# 	                   cxxflags = cxxflags + ['-g', '-march=native'],   # -march=native
	# 	                   variants = ['BINARY'])

    # if bld.get_env()['BUILD_GRAPHIC'] == True:
    #   sferes.create_variants(bld,
    #                          source = 'hexa_duty.cpp',
    #                          use = 'sferes2',
    #                          uselib = graphic_libs,
    #                          cxxflags = cxxflags + ['-march=native'],
    #                          target = 'hexa_duty',
    #                          variants = ['GRAPHIC'])
