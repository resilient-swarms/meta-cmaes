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
  


def build(bld):
    libs = 'RHEX_DART RHEX_CONTROLLER DART EIGEN BOOST_DART BOOST TBB '

    cxxflags = bld.get_env()['CXXFLAGS']
    tag=""
    if os.environ.get('BUILD_ALL',False) == "True":
			    sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = 'DART_GRAPHIC ' + libs,
		                   target = 'rhex_metaCMAES_graphic_envir',
		                   cxxflags = cxxflags + ['-g', '-march=native',"-DGRAPHIC","-DEVAL_ENVIR"],   # -march=native
		                   variants = ['TEXT'])

			    sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = 'DART_GRAPHIC ' + libs,
		                   target = 'rhex_metaCMAES_graphic_damage',
		                   cxxflags = cxxflags + ['-g', '-march=native',"-DGRAPHIC"],   # -march=native
		                   variants = ['TEXT'])

	                    sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = libs,
		                   target = 'rhex_metaCMAES_envir',
		                   cxxflags = cxxflags + ['-g', '-march=native',"-DEVAL_ENVIR"],   # -march=native
		                   variants = ['TEXT'])

			    sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = libs,
		                   target = 'rhex_metaCMAES_damage',
		                   cxxflags = cxxflags + ['-g', '-march=native'],   # -march=native
		                   variants = ['TEXT'])
    else:
	    if os.environ.get('BUILD_GRAPHIC',False) == "True":
	    	cxxflags+=["-DGRAPHIC"]
		tag+="_graphic"
		libs = 'DART_GRAPHIC ' + libs
	    if os.environ.get('BUILD_EVAL_ENVIR',False)  == "True":
	    	cxxflags+=["-DEVAL_ENVIR"]
		tag+="_envir"
	    else:
		tag+="_damage"
	    sferes.create_variants(bld,
		                   source = 'rhex_metaCMAES.cpp',
		                   use = 'sferes2',
		                   uselib = libs,
		                   target = 'rhex_metaCMAES'+tag,
		                   cxxflags = cxxflags + ['-g', '-march=native'],   # -march=native
		                   variants = ['TEXT'])

    # if bld.get_env()['BUILD_GRAPHIC'] == True:
    #   sferes.create_variants(bld,
    #                          source = 'hexa_duty.cpp',
    #                          use = 'sferes2',
    #                          uselib = graphic_libs,
    #                          cxxflags = cxxflags + ['-march=native'],
    #                          target = 'hexa_duty',
    #                          variants = ['GRAPHIC'])
