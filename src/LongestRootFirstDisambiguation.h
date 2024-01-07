//
// Created by Olcay Taner YILDIZ on 7.01.2024.
//

#ifndef MORPHOLOGICALDISAMBIGUATION_LONGESTROOTFIRSTDISAMBIGUATION_H
#define MORPHOLOGICALDISAMBIGUATION_LONGESTROOTFIRSTDISAMBIGUATION_H

#include <FsmParseList.h>
#include <ArrayList.h>

Array_list_ptr disambiguate_longest_root_first(Hash_map_ptr root_list, Fsm_parse_list_ptr* fsm_parses, int size);;

Hash_map_ptr train_longest_root_first();

#endif //MORPHOLOGICALDISAMBIGUATION_LONGESTROOTFIRSTDISAMBIGUATION_H
