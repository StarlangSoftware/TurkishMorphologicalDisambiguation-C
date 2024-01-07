//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#ifndef MORPHOLOGICALDISAMBIGUATION_HMMDISAMBIGUATION_H
#define MORPHOLOGICALDISAMBIGUATION_HMMDISAMBIGUATION_H

#include <Corpus.h>
#include "RootFirstDisambiguation.h"

Hmm_model_ptr train_hmm(Corpus_ptr corpus);

Array_list_ptr disambiguate_hmm(Hmm_model_ptr model,
                                Fsm_parse_list_ptr* fsm_parses,
                                int size);

#endif //MORPHOLOGICALDISAMBIGUATION_HMMDISAMBIGUATION_H
