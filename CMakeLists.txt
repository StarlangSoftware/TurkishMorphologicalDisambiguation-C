cmake_minimum_required(VERSION 3.23)
project(MorphologicalDisambiguation)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-O3")

find_package(morphological_analysis_c REQUIRED)
find_package(hmm_c REQUIRED)
find_package(n_gram_c REQUIRED)

add_library(MorphologicalDisambiguation src/AutoDisambiguator.c src/AutoDisambiguator.h
        src/DummyDisambiguation.c
        src/DummyDisambiguation.h
        src/RootFirstDisambiguation.c
        src/RootFirstDisambiguation.h
        src/LongestRootFirstDisambiguation.c
        src/LongestRootFirstDisambiguation.h
        src/HmmDisambiguation.c
        src/HmmDisambiguation.h)
target_link_libraries(MorphologicalDisambiguation morphological_analysis_c::morphological_analysis_c hmm_c::hmm_c n_gram_c::n_gram_c)
add_executable(DummyDisambiguationTest src/AutoDisambiguator.c src/AutoDisambiguator.h
        src/DummyDisambiguation.c
        src/DummyDisambiguation.h
        src/RootFirstDisambiguation.c
        src/RootFirstDisambiguation.h
        src/LongestRootFirstDisambiguation.c
        src/LongestRootFirstDisambiguation.h
        src/HmmDisambiguation.c
        src/HmmDisambiguation.h
        Test/DummyDisambiguationTest.c)
target_link_libraries(DummyDisambiguationTest morphological_analysis_c::morphological_analysis_c hmm_c::hmm_c n_gram_c::n_gram_c)
add_executable(LongestRootFirstDisambiguationTest src/AutoDisambiguator.c src/AutoDisambiguator.h
        src/DummyDisambiguation.c
        src/DummyDisambiguation.h
        src/RootFirstDisambiguation.c
        src/RootFirstDisambiguation.h
        src/LongestRootFirstDisambiguation.c
        src/LongestRootFirstDisambiguation.h
        src/HmmDisambiguation.c
        src/HmmDisambiguation.h
        Test/LongestRootFirstDisambiguationTest.c)
target_link_libraries(LongestRootFirstDisambiguationTest morphological_analysis_c::morphological_analysis_c hmm_c::hmm_c n_gram_c::n_gram_c)
