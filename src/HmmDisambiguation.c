//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#include <Memory/Memory.h>
#include <DisambiguatedWord.h>
#include <LaplaceSmoothing.h>
#include <math.h>
#include <limits.h>
#include "HmmDisambiguation.h"

/**
 * The train method gets sentences from given DisambiguationCorpus and both word and the next word of that sentence at each iteration.
 * Then, adds these words together with their part of speech tags to word unigram and bigram models. It also adds the last inflectional group of
 * word to the ig unigram and bigram models.
 * <p>
 * At the end, it calculates the NGram probabilities of both word and ig unigram models by using LaplaceSmoothing, and
 * both word and ig bigram models by using InterpolatedSmoothing.
 *
 * @param corpus DisambiguationCorpus to train.
 */
Hmm_model_ptr train_hmm(Corpus_ptr corpus) {
    Hmm_model_ptr model = malloc_(sizeof(Hmm_model), "train_hmm");
    int i, j;
    Sentence_ptr sentence;
    Disambiguated_word_ptr word, word2;
    Array_list_ptr words = create_array_list();
    Array_list_ptr igs = create_array_list();
    array_list_add(words, clone_string(" "));
    array_list_add(words, clone_string(" "));
    array_list_add(igs, clone_string(" "));
    array_list_add(igs, clone_string(" "));
    model->word_uni_gram_model = create_n_gram2(1, (unsigned int (*)(const void *, int)) hash_function_string,
                                                (int (*)(const void *, const void *)) compare_string);
    model->word_bi_gram_model = create_n_gram2(2, (unsigned int (*)(const void *, int)) hash_function_string,
                                               (int (*)(const void *, const void *)) compare_string);
    model->ig_uni_gram_model = create_n_gram2(1, (unsigned int (*)(const void *, int)) hash_function_string,
                                              (int (*)(const void *, const void *)) compare_string);
    model->ig_bi_gram_model = create_n_gram2(2, (unsigned int (*)(const void *, int)) hash_function_string,
                                             (int (*)(const void *, const void *)) compare_string);
    for (i = 0; i < corpus->sentences->size; i++) {
        sentence = array_list_get(corpus->sentences, i);
        for (j = 0; j < sentence->words->size - 1; j++) {
            word = array_list_get(sentence->words, j);
            word2 = array_list_get(sentence->words, j + 1);
            array_list_replace(words, 0, get_word_with_pos(word->parse), free_);
            array_list_replace(words, 1, get_word_with_pos(word2->parse), free_);
            add_n_gram(model->word_uni_gram_model, words, 1);
            add_n_gram(model->word_bi_gram_model, words, 2);
            for (int k = 0; k < word2->parse->inflectional_groups->size; k++){
                array_list_replace(igs, 0, inflectional_group_to_string(get_last_inflectional_group(word->parse)), free_);
                array_list_replace(igs, 1, inflectional_group_to_string(get_inflectional_group(word2->parse, k)), free_);
                add_n_gram(model->ig_bi_gram_model, igs, 2);
                array_list_swap(igs, 0, 1);
                add_n_gram(model->ig_uni_gram_model, igs, 1);
            }
        }
    }
    free_array_list(words, free_);
    free_array_list(igs, free_);
    double delta[] = {1.0};
    set_probabilities_simple(model->word_uni_gram_model, delta, set_probabilities_with_level_laplace_smoothing);
    set_probabilities_simple(model->word_bi_gram_model, delta, set_probabilities_with_level_laplace_smoothing);
    set_probabilities_simple(model->ig_uni_gram_model, delta, set_probabilities_with_level_laplace_smoothing);
    set_probabilities_simple(model->ig_bi_gram_model, delta, set_probabilities_with_level_laplace_smoothing);
    return model;
}

/**
 * The disambiguate method takes FsmParseList as an input and gets one word with its part of speech tags, then gets its probability
 * from word unigram model. It also gets ig and its probability. Then, hold the logarithmic value of  the product of these probabilities in an array.
 * Also by taking into consideration the parses of these word it recalculates the probabilities and returns these parses.
 *
 * @param fsm_parses FsmParseList to disambiguate.
 * @return ArrayList of fsm_parses.
 */
Array_list_ptr disambiguate_hmm(Hmm_model_ptr model, 
                                Fsm_parse_list_ptr *fsm_parses, 
                                int size) {
    int j, k, t, bestIndex;
    double probability, bestProbability;
    char* w1, *w2, *ig1, *ig2;
    for (int i = 0; i < size; i++) {
        if (fsm_parses[i]->fsm_parses->size == 0) {
            return create_array_list();
        }
    }
    Array_list_ptr correct_fsm_parses = create_array_list();
    double** probabilities = malloc_(size * sizeof(double *), "disambiguate_hmm_1");
    int** best = malloc_(size * sizeof(int*), "disambiguate_hmm_2");
    for (int i = 0; i < size; i++) {
        probabilities[i] = malloc_(fsm_parses[i]->fsm_parses->size * sizeof(double), "disambiguate_hmm_3");
        best[i] = malloc_(fsm_parses[i]->fsm_parses->size * sizeof(int), "disambiguate_hmm_4");
    }
    for (int i = 0; i < fsm_parses[0]->fsm_parses->size; i++) {
        Fsm_parse_ptr currentParse = array_list_get(fsm_parses[0]->fsm_parses, i);
        w1 = get_word_with_pos2(currentParse);
        probability = get_probability(model->word_uni_gram_model, 1, w1);
        free_(w1);
        for (j = 0; j < currentParse->inflectional_groups->size; j++) {
            ig1 = inflectional_group_to_string(array_list_get(currentParse->inflectional_groups, j));
            probability *= get_probability(model->ig_uni_gram_model, 1, ig1);
            free_(ig1);
        }
        probabilities[0][i] = log(probability);
    }
    for (int i = 1; i < size; i++) {
        for (j = 0; j < fsm_parses[i]->fsm_parses->size; j++) {
            bestProbability = -INT_MAX;
            bestIndex = -1;
            Fsm_parse_ptr currentParse = array_list_get(fsm_parses[i]->fsm_parses, j);
            for (k = 0; k < fsm_parses[i - 1]->fsm_parses->size; k++) {
                Fsm_parse_ptr previousParse = array_list_get(fsm_parses[i - 1]->fsm_parses, k);
                w1 = get_word_with_pos2(previousParse);
                w2 = get_word_with_pos2(currentParse);
                probability = probabilities[i - 1][k] + log(get_probability(model->word_bi_gram_model, 2, w1, w2));
                free_(w1);
                free_(w2);
                for (t = 0; t < ((Fsm_parse_ptr) array_list_get(fsm_parses[i]->fsm_parses, j))->inflectional_groups->size; t++) {
                    ig1 = inflectional_group_to_string(array_list_get(previousParse->inflectional_groups, previousParse->inflectional_groups->size - 1));
                    ig2 = inflectional_group_to_string(array_list_get(currentParse->inflectional_groups, t));
                    probability += log(get_probability(model->ig_bi_gram_model, 2, ig1, ig2));
                    free_(ig1);
                    free_(ig2);
                }
                if (probability > bestProbability) {
                    bestIndex = k;
                    bestProbability = probability;
                }
            }
            probabilities[i][j] = bestProbability;
            best[i][j] = bestIndex;
        }
    }
    bestProbability = -INT_MAX;
    bestIndex = -1;
    for (int i = 0; i < fsm_parses[size - 1]->fsm_parses->size; i++) {
        if (probabilities[size - 1][i] > bestProbability) {
            bestProbability = probabilities[size - 1][i];
            bestIndex = i;
        }
    }
    if (bestIndex == -1) {
        return create_array_list();
    }
    array_list_add(correct_fsm_parses, clone_fsm_parse(array_list_get(fsm_parses[size - 1]->fsm_parses, bestIndex)));
    for (int i = size - 2; i >= 0; i--) {
        bestIndex = best[i + 1][bestIndex];
        if (bestIndex == -1) {
            return create_array_list();
        }
        array_list_insert(correct_fsm_parses, 0, array_list_get(fsm_parses[i]->fsm_parses, bestIndex));
    }
    for (int i = 0; i < size; i++){
        free_(probabilities[i]);
        free_(best[i]);
    }
    free_(probabilities);
    free_(best);
    return correct_fsm_parses;
}
