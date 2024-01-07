cmake_minimum_required(VERSION 3.23)
project(MorphologicalDisambiguation)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-O3")

find_package(morphological_analysis_c REQUIRED)
find_package(hmm_c REQUIRED)
find_package(n_gram_c REQUIRED)

add_library(MorphologicalDisambiguation AutoDisambiguator.c AutoDisambiguator.h
        DummyDisambiguation.c
        DummyDisambiguation.h
        RootFirstDisambiguation.c
        RootFirstDisambiguation.h
        LongestRootFirstDisambiguation.c
        LongestRootFirstDisambiguation.h
        HmmDisambiguation.c
        HmmDisambiguation.h)
target_link_libraries(MorphologicalDisambiguation morphological_analysis_c::morphological_analysis_c hmm_c::hmm_c n_gram_c::n_gram_c)
