//
// Created by Olcay Taner YILDIZ on 29.01.2024.
//

#include <FsmMorphologicalAnalyzer.h>
#include <Corpus.h>
#include <DisambiguationCorpus.h>
#include <DisambiguatedWord.h>
#include <Memory/Memory.h>
#include <string.h>
#include "../src/DummyDisambiguation.h"

int main(){
    Fsm_morphological_analyzer_ptr fsm = create_fsm_morphological_analyzer3();
    Corpus_ptr corpus = create_disambiguation_corpus("penntreebank.txt");
    int correct_parse = 0, total_parse = 0;
    for (int i = 0; i < 10; i++){
        Sentence_ptr sentence = array_list_get(corpus->sentences, i);
        Sentence_ptr new_sentence = create_sentence();
        for (int j = 0; j < sentence->words->size; j++){
            Disambiguated_word_ptr word1 = array_list_get(sentence->words, j);
            sentence_add_word(new_sentence, clone_string(word1->name));
        }
        Fsm_parse_list_ptr* fsm_parses = robust_morphological_analysis2(fsm, new_sentence);
        free_sentence(new_sentence);
        Array_list_ptr correct_parses =  disambiguate_dummy(fsm_parses, sentence->words->size);;
        for (int j = 0; j < sentence->words->size; j++){
            Disambiguated_word_ptr word1 = array_list_get(sentence->words, j);
            Fsm_parse_ptr fsm_parse = array_list_get(correct_parses, j);
            char* list1 = transition_list(fsm_parse);
            char* list2 = morphological_parse_to_string(word1->parse);
            if (strcmp(list1, list2) == 0){
                correct_parse++;
            }
            total_parse++;
            free_(list1);
            free_(list2);
            free_fsm_parse_list(fsm_parses[j]);
        }
        free_(fsm_parses);
        free_array_list(correct_parses, (void (*)(void *)) free_fsm_parse);
    }
    free_disambiguation_corpus(corpus);
    free_fsm_morphological_analyzer(fsm);
}