//
// Created by Olcay Taner YILDIZ on 30.12.2023.
//

#ifndef MORPHOLOGICALDISAMBIGUATION_AUTODISAMBIGUATOR_H
#define MORPHOLOGICALDISAMBIGUATION_AUTODISAMBIGUATOR_H

#include <stdbool.h>
#include <ArrayList.h>
#include <FsmParseList.h>

bool is_any_word_second_person(int index, Array_list_ptr correct_parses);

bool is_possessive_plural(int index, Array_list_ptr correct_parses);

char* next_word_pos(Fsm_parse_list_ptr next_parse_list);

bool is_before_last_word(int index, int length);

bool next_word_exists(int index, int length);

bool is_next_word_noun(int index, Fsm_parse_list_ptr* fsm_parses, int length);

bool is_next_word_num(int index, Fsm_parse_list_ptr* fsm_parses, int length);

bool is_next_word_noun_or_adjective(int index, Fsm_parse_list_ptr* fsm_parses, int length);

bool is_first_word(int index);

bool contains_two_ne_or_ya(Fsm_parse_list_ptr* fsm_parses, const char* word, int length);

bool has_previous_word_tag(int index, Array_list_ptr correct_parses, Morphological_tag tag);

char* select_case_for_parse_string(const char* parse_string, int index, Fsm_parse_list_ptr* fsmParses, Array_list_ptr correct_parses, int length);

Fsm_parse_ptr case_disambiguator(int index, Fsm_parse_list_ptr* fsm_parses, Array_list_ptr correct_parses, int length);

#endif //MORPHOLOGICALDISAMBIGUATION_AUTODISAMBIGUATOR_H
