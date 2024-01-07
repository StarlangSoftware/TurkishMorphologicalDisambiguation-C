//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#include <FileUtils.h>
#include <string.h>
#include "LongestRootFirstDisambiguation.h"
#include "AutoDisambiguator.h"

Hash_map_ptr train_longest_root_first() {
    return read_hash_map("rootlist.txt");
}

/**
 * The disambiguate method gets an array of fsmParses. Then loops through that parses and finds the longest root
 * word. At the end, gets the parse with longest word among the fsmParses and adds it to the correctFsmParses
 * ArrayList.
 *
 * @param fsm_parses FsmParseList to disambiguate.
 * @return correctFsmParses ArrayList which holds the parses with longest root words.
 */
Array_list_ptr disambiguate_longest_root_first(Hash_map_ptr root_list, Fsm_parse_list_ptr *fsm_parses, int size) {
    Array_list_ptr correct_fsm_parses = create_array_list();
    Fsm_parse_ptr best_parse;
    for (int i = 0; i < size; i++) {
        Fsm_parse_list_ptr fsmParseList = fsm_parses[i];
        char* best_root;
        char* surface_form = ((Fsm_parse_ptr) array_list_get(fsmParseList->fsm_parses, 0))->form;
        if (hash_map_contains(root_list, surface_form)){
            best_root = hash_map_get(root_list, surface_form);
        }
        bool root_found = false;
        for (int j = 0; j < fsmParseList->fsm_parses->size; j++) {
            Fsm_parse_ptr fsm_parse = array_list_get(fsmParseList->fsm_parses, j);
            if (strcmp(fsm_parse->root->name, best_root) == 0) {
                root_found = true;
                break;
            }
        }
        if (!root_found){
            best_parse = get_parse_with_longest_root_word(fsmParseList);
            reduce_to_parses_with_same_root(fsm_parses[i], best_parse->root->name);
        } else {
            reduce_to_parses_with_same_root(fsm_parses[i], best_root);
        }
        Fsm_parse_ptr new_best_parse = case_disambiguator(i, fsm_parses, correct_fsm_parses, size);
        array_list_add(correct_fsm_parses, clone_fsm_parse(new_best_parse));
    }
    return correct_fsm_parses;
}
