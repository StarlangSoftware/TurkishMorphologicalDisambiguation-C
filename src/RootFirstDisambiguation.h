//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#ifndef MORPHOLOGICALDISAMBIGUATION_ROOTFIRSTDISAMBIGUATION_H
#define MORPHOLOGICALDISAMBIGUATION_ROOTFIRSTDISAMBIGUATION_H

#include <NGram.h>
#include <FsmParseList.h>
#include <Corpus.h>

struct hmm_model{
    N_gram_ptr word_bi_gram_model;
    N_gram_ptr word_uni_gram_model;
    N_gram_ptr ig_bi_gram_model;
    N_gram_ptr ig_uni_gram_model;
};

typedef struct hmm_model Hmm_model;

typedef Hmm_model *Hmm_model_ptr;

void free_hmm_model(Hmm_model_ptr model);

double get_word_probability(Hmm_model_ptr model,
                            char* word,
                            Array_list_ptr correct_fsm_parses,
                            int index);

double get_ig_probability(Hmm_model_ptr model,
                          char* word,
                          Array_list_ptr correct_fsm_parses,
                          int index);

Fsm_parse_ptr get_parse_with_best_ig_probability(Hmm_model_ptr model,
                                                 Fsm_parse_list_ptr parse_list,
                                                 Array_list_ptr correct_fsm_parses,
                                                 int index);

char* get_best_root_word(Hmm_model_ptr model, Fsm_parse_list_ptr parse_list);

Array_list_ptr disambiguate_root_first(Hmm_model_ptr model,
                                       Fsm_parse_list_ptr* fsm_parses,
                                       int size);

Hmm_model_ptr train_root_first(Corpus_ptr corpus);

#endif //MORPHOLOGICALDISAMBIGUATION_ROOTFIRSTDISAMBIGUATION_H
