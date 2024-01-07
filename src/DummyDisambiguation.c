//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#include <stdlib.h>
#include "DummyDisambiguation.h"

/**
 * Overridden disambiguate method takes an array of FsmParseList and loops through its items, if the current FsmParseList's
 * size is greater than 0, it adds a random parse of this list to the correctFsmParses vector.
 *
 * @param fsm_parses FsmParseList to disambiguate.
 * @return correctFsmParses vector.
 */
Array_list_ptr disambiguate_dummy(Fsm_parse_list_ptr *fsm_parses, int size) {
    Array_list_ptr correct_fsm_parses = create_array_list();
    for (int i = 0; i < size; i++) {
        Fsm_parse_list_ptr fsmParseList = fsm_parses[i];
        if (fsmParseList->fsm_parses->size > 0) {
            array_list_add(correct_fsm_parses, clone_fsm_parse(array_list_get(fsmParseList->fsm_parses, random() % fsmParseList->fsm_parses->size)));
        }
    }
    return correct_fsm_parses;
}
