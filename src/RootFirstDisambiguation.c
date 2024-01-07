//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#include <FsmParse.h>
#include <Memory/Memory.h>
#include <limits.h>
#include "RootFirstDisambiguation.h"
#include <Corpus.h>
#include <DisambiguatedWord.h>
#include <LaplaceSmoothing.h>

/**
 * The getWordProbability method returns the probability of a word by using word bigram or unigram model.
 *
 * @param word             Word to find the probability.
 * @param correct_fsm_parses FsmParse of given word which will be used for getting part of speech tags.
 * @param index            Index of FsmParse of which part of speech tag will be used to get the probability.
 * @return The probability of the given word.
 */
double get_word_probability(Hmm_model_ptr model,
                            char *word,
                            Array_list_ptr correct_fsm_parses,
                            int index) {
    if (index != 0 && correct_fsm_parses->size == index) {
        Fsm_parse_ptr fsm_parse = array_list_get(correct_fsm_parses, index - 1);
        char* word_with_pos = get_word_with_pos2(fsm_parse);
        double p =  get_probability(model->word_bi_gram_model, 2, word_with_pos, word);
        free_(word_with_pos);
        return p;
    } else {
        return get_probability(model->word_uni_gram_model, 1, word);
    }
}

/**
 * The getIgProbability method returns the probability of a word by using ig bigram or unigram model.
 *
 * @param word             Word to find the probability.
 * @param correctFsmParses FsmParse of given word which will be used for getting transition list.
 * @param index            Index of FsmParse of which transition list will be used to get the probability.
 * @return The probability of the given word.
 */
double get_ig_probability(Hmm_model_ptr model,
                          char *word,
                          Array_list_ptr correct_fsm_parses,
                          int index) {
    if (index != 0 && correct_fsm_parses->size == index) {
        Fsm_parse_ptr fsm_parse = array_list_get(correct_fsm_parses, index - 1);
        char* list = transition_list(fsm_parse);
        double p =  get_probability(model->ig_bi_gram_model, 2, list, word);
        free_(list);
        return p;
    } else {
        return get_probability(model->ig_uni_gram_model, 1, word);
    }
}

/**
 * The getParseWithBestIgProbability gets each FsmParse's transition list as a Word ig. Then, finds the corresponding
 * probabilitt. At the end returns the parse with the highest ig probability.
 *
 * @param parse_list        FsmParseList is used to get the FsmParse.
 * @param correct_fsm_parses FsmParse is used to get the transition lists.
 * @param index            Index of FsmParse of which transition list will be used to get the probability.
 * @return The parse with the highest probability.
 */
Fsm_parse_ptr get_parse_with_best_ig_probability(Hmm_model_ptr model,
                                                 Fsm_parse_list_ptr parse_list,
                                                 Array_list_ptr correct_fsm_parses,
                                                 int index) {
    double best_probability, probability;
    Fsm_parse_ptr best_parse;
    best_probability = -INT_MAX;
    for (int j = 0; j < parse_list->fsm_parses->size; j++) {
        char* ig = transition_list(array_list_get(parse_list->fsm_parses, j));
        probability = get_ig_probability(model, ig, correct_fsm_parses, index);
        if (probability > best_probability) {
            best_parse = array_list_get(parse_list->fsm_parses, j);
            best_probability = probability;
        }
        free_(ig);
    }
    return best_parse;
}

/**
 * The getBestRootWord method takes a FsmParseList as an input and loops through the list. It gets each word with its
 * part of speech tags as a new Word word and its transition list as a Word ig. Then, finds their corresponding
 * probabilities. At the end returns the word with the highest probability.
 *
 * @param fsmParseList FsmParseList is used to get the part of speech tags and transition lists of words.
 * @return The word with the highest probability.
 */
char *get_best_root_word(Hmm_model_ptr model, Fsm_parse_list_ptr parse_list) {
    double best_probability, probability;
    char* best_word = NULL;
    best_probability = -INT_MAX;
    for (int j = 0; j < parse_list->fsm_parses->size; j++) {
        char* word = get_word_with_pos2(array_list_get(parse_list->fsm_parses, j));
        char* ig = transition_list(array_list_get(parse_list->fsm_parses, j));
        double wordProbability = get_probability(model->word_uni_gram_model, 1, word);
        double igProbability = get_probability(model->ig_uni_gram_model, 1, ig);
        free_(ig);
        probability = wordProbability * igProbability;
        if (probability > best_probability) {
            best_word = word;
            best_probability = probability;
        } else {
            free_(word);
        }
    }
    return best_word;
}

void free_hmm_model(Hmm_model_ptr model) {
    free_(model);
    free_n_gram(model->ig_uni_gram_model);
    free_n_gram(model->ig_bi_gram_model);
    free_n_gram(model->word_uni_gram_model);
    free_n_gram(model->word_bi_gram_model);
}

Array_list_ptr disambiguate_root_first(Hmm_model_ptr model,
                                       Fsm_parse_list_ptr *fsm_parses,
                                       int size) {
    char* best_word;
    Fsm_parse_ptr best_parse;
    Array_list_ptr correct_fsm_parses = create_array_list();
    for (int i = 0; i < size; i++) {
        best_word = get_best_root_word(model, fsm_parses[i]);
        reduce_to_parses_with_same_root_and_pos(fsm_parses[i], best_word);
        best_parse = get_parse_with_best_ig_probability(model, fsm_parses[i], correct_fsm_parses, i);
        if (best_parse->inflectional_groups->size > 0) {
            array_list_add(correct_fsm_parses, clone_fsm_parse(best_parse));
        }
    }
    return correct_fsm_parses;
}

/**
 * The train method initially creates new NGrams; wordUniGramModel, wordBiGramModel, igUniGramModel, and igBiGramModel. It gets the
 * sentences from given corpus and gets each word as a DisambiguatedWord. Then, adds the word together with its part of speech
 * tags to the wordUniGramModel. It also gets the transition list of that word and adds it to the igUniGramModel.
 * <p>
 * If there exists a next word in the sentence, it adds the current and next DisambiguatedWord to the wordBiGramModel with
 * their part of speech tags. It also adds them to the igBiGramModel with their transition lists.
 * <p>
 * At the end, it calculates the NGram probabilities of both word and ig unigram models by using LaplaceSmoothing, and
 * both word and ig bigram models by using InterpolatedSmoothing.
 *
 * @param corpus DisambiguationCorpus to train.
 */
Hmm_model_ptr train_root_first(Corpus_ptr corpus) {
    Hmm_model_ptr model = malloc_(sizeof(Hmm_model), "train_root_first");
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
        for (j = 0; j < sentence->words->size; j++) {
            word = array_list_get(sentence->words, j);
            array_list_replace(words, 0, get_word_with_pos(word->parse), free_);
            add_n_gram(model->word_uni_gram_model, words, 1);
            array_list_replace(igs, 0, get_transition_list(word->parse), free_);
            add_n_gram(model->ig_uni_gram_model, igs, 1);
            if (j + 1 < sentence->words->size) {
                word2 = array_list_get(sentence->words, j + 1);
                array_list_replace(words, 1, get_word_with_pos(word2->parse), free_);
                add_n_gram(model->word_bi_gram_model, words, 2);
                array_list_replace(igs, 1, get_transition_list(word2->parse), free_);
                add_n_gram(model->ig_bi_gram_model, igs, 2);
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
