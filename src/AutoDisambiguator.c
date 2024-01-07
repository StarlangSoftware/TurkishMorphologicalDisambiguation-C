//
// Created by Olcay Taner YILDIZ on 30.12.2023.
//

#include <FsmParse.h>
#include <CounterHashMap.h>
#include <Memory/Memory.h>
#include <string.h>
#include "AutoDisambiguator.h"

bool is_any_word_second_person(int index, Array_list_ptr correct_parses) {
    int count = 0;
    for (int i = index - 1; i >= 0; i--) {
        Fsm_parse_ptr fsm_parse = array_list_get(correct_parses, i);
        if (fsm_parse_contains_tag(fsm_parse, A2SG) || fsm_parse_contains_tag(fsm_parse, P2SG)) {
            count++;
        }
    }
    return count >= 1;
}

bool is_possessive_plural(int index, Array_list_ptr correct_parses) {
    for (int i = index - 1; i >= 0; i--) {
        Fsm_parse_ptr fsm_parse = array_list_get(correct_parses, i);
        if (is_fsm_parse_noun(fsm_parse)) {
            return is_fsm_parse_plural(fsm_parse);
        }
    }
    return false;
}

char *next_word_pos(Fsm_parse_list_ptr next_parse_list) {
    Counter_hash_map_ptr map = create_counter_hash_map((unsigned int (*)(const void *, int)) hash_function_string,
                                                       (int (*)(const void *, const void *)) compare_string);
    for (int i = 0; i < next_parse_list->fsm_parses->size; i++) {
        Fsm_parse_ptr fsm_parse = array_list_get(next_parse_list->fsm_parses, i);
        put_counter_hash_map(map, get_fsm_parse_pos(fsm_parse));
    }
    char* result = clone_string(max_counter_hash_map(map));
    free_counter_hash_map2(map, free_);
    return result;
}

bool is_before_last_word(int index, int length) {
    return index + 2 == length;
}

bool next_word_exists(int index, int length) {
    return index + 1 < length;
}

bool is_next_word_noun(int index, Fsm_parse_list_ptr* fsm_parses, int length) {
    char* pos = next_word_pos(fsm_parses[index + 1]);
    bool result = index + 1 < length && strcmp(pos, "NOUN") == 0;
    free_(pos);
    return result;
}

bool is_next_word_num(int index, Fsm_parse_list_ptr *fsm_parses, int length) {
    char* pos = next_word_pos(fsm_parses[index + 1]);
    bool result = index + 1 < length && strcmp(pos, "NUM") == 0;
    free_(pos);
    return result;
}

bool is_next_word_noun_or_adjective(int index, Fsm_parse_list_ptr *fsm_parses, int length) {
    char* pos = next_word_pos(fsm_parses[index + 1]);
    bool result = index + 1 < length && string_in_list(pos, (char*[]) {"NOUN", "ADJ", "DET"}, 3);
    free_(pos);
    return result;
}

bool is_first_word(int index) {
    return index == 0;
}

bool contains_two_ne_or_ya(Fsm_parse_list_ptr* fsm_parses, const char *word, int length) {
    int count = 0;
    for (int i = 0; i < length; i++) {
        Fsm_parse_ptr fsm_parse = array_list_get(fsm_parses[i]->fsm_parses, 0);
        char* surfaceForm = fsm_parse->form;
        if (strcmp(surfaceForm, word) == 0) {
            count++;
        }
    }
    return count == 2;
}

bool has_previous_word_tag(int index, Array_list_ptr correct_parses, Morphological_tag tag) {
    Fsm_parse_ptr fsm_parse = array_list_get(correct_parses, index - 1);
    return index > 0 && fsm_parse_contains_tag(fsm_parse, tag);
}

char *select_case_for_parse_string(const char *parse_string, 
                                   int index, 
                                   Fsm_parse_list_ptr *fsm_parses,
                                   Array_list_ptr correct_parses, 
                                   int length) {
    char* surfaceForm = ((Fsm_parse_ptr) array_list_get(fsm_parses[index]->fsm_parses, 0))->form;
    char* root = ((Fsm_parse_ptr) array_list_get(fsm_parses[index]->fsm_parses, 0))->root->name;
    char* lastWord = ((Fsm_parse_ptr) array_list_get(fsm_parses[length - 1]->fsm_parses, 0))->form;
    if (strcmp(parse_string, "P2SG$P3SG") == 0) {
        /* kısmını, duracağını, grubunun */
        if (is_any_word_second_person(index, correct_parses)) {
            return "P2SG";
        }
        return "P3SG";
    } else {
        if (strcmp(parse_string, "A2SG+P2SG$A3SG+P3SG") == 0) {
            if (is_any_word_second_person(index, correct_parses)) {
                return "A2SG+P2SG";
            }
            return "A3SG+P3SG";
        } else {
            /* BİR */
            if (strcmp(parse_string, "ADJ$ADV$DET$NUM+CARD") == 0) {
                return "DET";
            } else {
                /* tahminleri, işleri, hisseleri */
                if (strcmp(parse_string, "A3PL+P3PL+NOM$A3PL+P3SG+NOM$A3PL+PNON+ACC$A3SG+P3PL+NOM") == 0) {
                    if (is_possessive_plural(index, correct_parses)) {
                        return "A3SG+P3PL+NOM";
                    }
                    return "A3PL+P3SG+NOM";
                } else {
                    /* Ocak, Cuma, ABD */
                    if (strcmp(parse_string, "A3SG$PROP+A3SG") == 0) {
                        if (index > 0) {
                            return "PROP+A3SG";
                        }
                    } else {
                        /* şirketin, seçimlerin, borsacıların, kitapların */
                        if (strcmp(parse_string, "P2SG+NOM$PNON+GEN") == 0) {
                            if (is_any_word_second_person(index, correct_parses)) {
                                return "P2SG+NOM";
                            }
                            return "PNON+GEN";
                        } else {
                            /* ÇOK */ /* FAZLA */
                            if (strcmp(parse_string, "ADJ$ADV$DET$POSTP+PCABL") == 0 || strcmp(parse_string, "ADJ$ADV$POSTP+PCABL") == 0) {
                                if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
                                    return "POSTP+PCABL";
                                }
                                if (index + 1 < length) {
                                    char* next_pos = next_word_pos(fsm_parses[index + 1]);
                                    if (strcmp(next_pos, "NOUN") == 0){
                                        free_(next_pos);
                                        return "ADJ";
                                    } else {
                                        if (string_in_list(next_pos, (char*[]) {"ADJ", "ADV", "VERB"}, 3)){
                                            free_(next_pos);
                                            return "ADV";
                                        }
                                    }
                                    free_(next_pos);
                                }
                            } else {
                                if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM") == 0) {
                                    if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                        return "ADJ";
                                    }
                                    return "NOUN+A3SG+PNON+NOM";
                                } else {
                                    /* fanatiklerini, senetlerini, olduklarını */
                                    if (strcmp(parse_string, "A3PL+P2SG$A3PL+P3PL$A3PL+P3SG$A3SG+P3PL") == 0) {
                                        if (is_any_word_second_person(index, correct_parses)) {
                                            return "A3PL+P2SG";
                                        }
                                        if (is_possessive_plural(index, correct_parses)) {
                                            return "A3SG+P3PL";
                                        } else {
                                            return "A3PL+P3SG";
                                        }
                                    } else {
                                        if (strcmp(parse_string, "ADJ$NOUN+PROP+A3SG+PNON+NOM") == 0) {
                                            if (index > 0) {
                                                return "NOUN+PROP+A3SG+PNON+NOM";
                                            }
                                        } else {
                                            /* BU, ŞU */
                                            if (strcmp(parse_string, "DET$PRON+DEMONSP+A3SG+PNON+NOM") == 0) {
                                                if (is_next_word_noun(index, fsm_parses, length)) {
                                                    return "DET";
                                                }
                                                return "PRON+DEMONSP+A3SG+PNON+NOM";
                                            } else {
                                                /* gelebilir */
                                                if (strcmp(parse_string, "AOR+A3SG$AOR^DB+ADJ+ZERO") == 0) {
                                                    if (is_before_last_word(index, length)) {
                                                        return "AOR+A3SG";
                                                    } else if (is_first_word(index)) {
                                                        return "AOR^DB+ADJ+ZERO";
                                                    } else {
                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                            return "AOR^DB+ADJ+ZERO";
                                                        } else {
                                                            return "AOR+A3SG";
                                                        }
                                                    }
                                                } else {
                                                    if (strcmp(parse_string, "ADV$NOUN+A3SG+PNON+NOM") == 0) {
                                                        return "ADV";
                                                    } else {
                                                        if (strcmp(parse_string, "ADJ$ADV") == 0) {
                                                            if (is_next_word_noun(index, fsm_parses, length)) {
                                                                return "ADJ";
                                                            }
                                                            return "ADV";
                                                        } else {
                                                            if (strcmp(parse_string, "P2SG$PNON") == 0) {
                                                                if (is_any_word_second_person(index, correct_parses)) {
                                                                    return "P2SG";
                                                                }
                                                                return "PNON";
                                                            } else {
                                                                /* etti, kırdı */
                                                                if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO$VERB+POS") == 0) {
                                                                    if (is_before_last_word(index, length)) {
                                                                        return "VERB+POS";
                                                                    }
                                                                } else {
                                                                    /* İLE */
                                                                    if (strcmp(parse_string, "CONJ$POSTP+PCNOM") == 0) {
                                                                        return "POSTP+PCNOM";
                                                                    } else {
                                                                        /* gelecek */
                                                                        if (strcmp(parse_string, "POS+FUT+A3SG$POS^DB+ADJ+FUTPART+PNON") == 0) {
                                                                            if (is_before_last_word(index, length)) {
                                                                                return "POS+FUT+A3SG";
                                                                            }
                                                                            return "POS^DB+ADJ+FUTPART+PNON";
                                                                        } else {
                                                                            if (strcmp(parse_string, "ADJ^DB$NOUN+A3SG+PNON+NOM^DB") == 0) {
                                                                                if (string_in_list(root, (char*[]) {"yok", "düşük", "eksik", "rahat", "orta", "vasat"}, 6)) {
                                                                                    return "ADJ^DB";
                                                                                }
                                                                                return "NOUN+A3SG+PNON+NOM^DB";
                                                                            } else {
                                                                                /* yaptık, şüphelendik */
                                                                                if (strcmp(parse_string, "POS+PAST+A1PL$POS^DB+ADJ+PASTPART+PNON$POS^DB+NOUN+PASTPART+A3SG+PNON+NOM") == 0) {
                                                                                    return "POS+PAST+A1PL";
                                                                                } else {
                                                                                    /* ederim, yaparım */
                                                                                    if (strcmp(parse_string, "AOR+A1SG$AOR^DB+ADJ+ZERO^DB+NOUN+ZERO+A3SG+P1SG+NOM") == 0) {
                                                                                        return "AOR+A1SG";
                                                                                    } else {
                                                                                        /* geçti, vardı, aldı */
                                                                                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO$VERB+POS") == 0) {
                                                                                            if (strcmp(root, "var") == 0 && !is_possessive_plural(index, correct_parses)) {
                                                                                                return "ADJ^DB+VERB+ZERO";
                                                                                            }
                                                                                            return "VERB+POS";
                                                                                        } else {
                                                                                            /* ancak */
                                                                                            if (strcmp(parse_string, "ADV$CONJ") == 0) {
                                                                                                return "CONJ";
                                                                                            } else {
                                                                                                /* yaptığı, ettiği */
                                                                                                if (strcmp(parse_string, "ADJ+PASTPART+P3SG$NOUN+PASTPART+A3SG+P3SG+NOM") == 0) {
                                                                                                    if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                                                                        return "ADJ+PASTPART+P3SG";
                                                                                                    }
                                                                                                    return "NOUN+PASTPART+A3SG+P3SG+NOM";
                                                                                                } else {
                                                                                                    /* ÖNCE, SONRA */
                                                                                                    if (strcmp(parse_string, "ADV$NOUN+A3SG+PNON+NOM$POSTP+PCABL") == 0) {
                                                                                                        if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
                                                                                                            return "POSTP+PCABL";
                                                                                                        }
                                                                                                        return "ADV";
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (strcmp(parse_string, "NARR+A3SG$NARR^DB+ADJ+ZERO") == 0) {
        if (is_before_last_word(index, length)) {
            return "NARR+A3SG";
        }
        return "NARR^DB+ADJ+ZERO";
    } else {
        if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM$NOUN+PROP+A3SG+PNON+NOM") == 0) {
            if (index > 0) {
                return "NOUN+PROP+A3SG+PNON+NOM";
            } else {
                if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                    return "ADJ";
                }
                return "NOUN+A3SG+PNON+NOM";
            }
        } else {
            /* ödediğim */
            if (strcmp(parse_string, "ADJ+PASTPART+P1SG$NOUN+PASTPART+A3SG+P1SG+NOM") == 0) {
                if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                    return "ADJ+PASTPART+P1SG";
                }
                return "NOUN+PASTPART+A3SG+P1SG+NOM";
            } else {
                /* O */
                if (strcmp(parse_string, "DET$PRON+DEMONSP+A3SG+PNON+NOM$PRON+PERS+A3SG+PNON+NOM") == 0) {
                    if (is_next_word_noun(index, fsm_parses, length)) {
                        return "DET";
                    }
                    return "PRON+PERS+A3SG+PNON+NOM";
                } else {
                    /* BAZI */
                    if (strcmp(parse_string, "ADJ$DET$PRON+QUANTP+A3SG+P3SG+NOM") == 0) {
                        return "DET";
                    } else {
                        /* ONUN, ONA, ONDAN, ONUNLA, OYDU, ONUNKİ */
                        if (strcmp(parse_string, "DEMONSP$PERS") == 0) {
                            return "PERS";
                        } else {
                            if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                    return "ADJ";
                                }
                                return "NOUN+A3SG+PNON+NOM";
                            } else {
                                /* hazineler, kıymetler */
                                if (strcmp(parse_string, "A3PL+PNON+NOM$A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL$PROP+A3PL+PNON+NOM") == 0) {
                                    if (index > 0) {
                                        if (is_fsm_parse_capital_word(array_list_get(fsm_parses[index]->fsm_parses, 0))) {
                                            return "PROP+A3PL+PNON+NOM";
                                        }
                                        return "A3PL+PNON+NOM";
                                    }
                                } else {
                                    /* ARTIK, GERİ */
                                    if (strcmp(parse_string, "ADJ$ADV$NOUN+A3SG+PNON+NOM") == 0) {
                                        if (strcmp(root, "artık") == 0) {
                                            return "ADV";
                                        } else if (is_next_word_noun(index, fsm_parses, length)) {
                                            return "ADJ";
                                        }
                                        return "ADV";
                                    } else {
                                        if (strcmp(parse_string, "P1SG+NOM$PNON+NOM^DB+VERB+ZERO+PRES+A1SG") == 0) {
                                            if (is_before_last_word(index, length) || strcmp(root, "değil") == 0) {
                                                return "PNON+NOM^DB+VERB+ZERO+PRES+A1SG";
                                            }
                                            return "P1SG+NOM";
                                        } else {
                                            /* görülmektedir */
                                            if (strcmp(parse_string, "POS+PROG2$POS^DB+NOUN+INF+A3SG+PNON+LOC^DB+VERB+ZERO+PRES") == 0) {
                                                return "POS+PROG2";
                                            } else {
                                                /* NE */
                                                if (strcmp(parse_string, "ADJ$ADV$CONJ$PRON+QUESP+A3SG+PNON+NOM") == 0) {
                                                    if (strcmp(lastWord, "?") == 0) {
                                                        return "PRON+QUESP+A3SG+PNON+NOM";
                                                    }
                                                    if (contains_two_ne_or_ya(fsm_parses, "ne", length)) {
                                                        return "CONJ";
                                                    }
                                                    if (is_next_word_noun(index, fsm_parses, length)) {
                                                        return "ADJ";
                                                    }
                                                    return "ADV";
                                                } else {
                                                    /* TÜM */
                                                    if (strcmp(parse_string, "DET$NOUN+A3SG+PNON+NOM") == 0) {
                                                        return "DET";
                                                    } else {
                                                        /* AZ */
                                                        if (strcmp(parse_string, "ADJ$ADV$POSTP+PCABL$VERB+POS+IMP+A2SG") == 0) {
                                                            if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
                                                                return "POSTP+PCABL";
                                                            }
                                                            if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                                return "ADJ";
                                                            }
                                                            return "ADV";
                                                        } else {
                                                            /* görülmedik */
                                                            if (strcmp(parse_string, "NEG+PAST+A1PL$NEG^DB+ADJ+PASTPART+PNON$NEG^DB+NOUN+PASTPART+A3SG+PNON+NOM") == 0) {
                                                                if (strcmp(surfaceForm, "alışılmadık") == 0) {
                                                                    return "NEG^DB+ADJ+PASTPART+PNON";
                                                                }
                                                                return "NEG+PAST+A1PL";
                                                            } else {
                                                                if (strcmp(parse_string, "DATE$NUM+FRACTION") == 0) {
                                                                    return "NUM+FRACTION";
                                                                } else {
                                                                    /* giriş, satış, öpüş, vuruş */
                                                                    if (strcmp(parse_string, "POS^DB+NOUN+INF3+A3SG+PNON+NOM$RECIP+POS+IMP+A2SG") == 0) {
                                                                        return "POS^DB+NOUN+INF3+A3SG+PNON+NOM";
                                                                    } else {
                                                                        /* başka, yukarı */
                                                                        if (strcmp(parse_string, "ADJ$POSTP+PCABL") == 0) {
                                                                            if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
                                                                                return "POSTP+PCABL";
                                                                            }
                                                                            return "ADJ";
                                                                        } else {
                                                                            /* KARŞI */
                                                                            if (strcmp(parse_string, "ADJ$ADV$NOUN+A3SG+PNON+NOM$POSTP+PCDAT") == 0) {
                                                                                if (has_previous_word_tag(index, correct_parses, DATIVE)) {
                                                                                    return "POSTP+PCDAT";
                                                                                }
                                                                                if (is_next_word_noun(index, fsm_parses, length)) {
                                                                                    return "ADJ";
                                                                                }
                                                                                return "ADV";
                                                                            } else {
                                                                                /* BEN */
                                                                                if (strcmp(parse_string, "NOUN+A3SG$NOUN+PROP+A3SG$PRON+PERS+A1SG") == 0) {
                                                                                    return "PRON+PERS+A1SG";
                                                                                } else {
                                                                                    /* yapıcı, verici */
                                                                                    if (strcmp(parse_string, "ADJ+AGT$NOUN+AGT+A3SG+PNON+NOM") == 0) {
                                                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                                                            return "ADJ+AGT";
                                                                                        }
                                                                                        return "NOUN+AGT+A3SG+PNON+NOM";
                                                                                    } else {
                                                                                        /* BİLE */
                                                                                        if (strcmp(parse_string, "ADV$VERB+POS+IMP+A2SG") == 0) {
                                                                                            return "ADV";
                                                                                        } else {
                                                                                            /* ortalamalar, uzaylılar, demokratlar */
                                                                                            if (strcmp(parse_string, "NOUN+ZERO+A3PL+PNON+NOM$VERB+ZERO+PRES+A3PL") == 0) {
                                                                                                return "NOUN+ZERO+A3PL+PNON+NOM";
                                                                                            } else {
                                                                                                /* yasa, diye, yıla */
                                                                                                if (strcmp(parse_string, "NOUN+A3SG+PNON+DAT$VERB+POS+OPT+A3SG") == 0) {
                                                                                                    return "NOUN+A3SG+PNON+DAT";
                                                                                                } else {
                                                                                                    /* BİZ, BİZE */
                                                                                                    if (strcmp(parse_string, "NOUN+A3SG$PRON+PERS+A1PL") == 0) {
                                                                                                        return "PRON+PERS+A1PL";
                                                                                                    } else {
                                                                                                        /* AZDI */
                                                                                                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO$POSTP+PCABL^DB+VERB+ZERO$VERB+POS") == 0) {
                                                                                                            return "ADJ^DB+VERB+ZERO";
                                                                                                        } else {
                                                                                                            /* BİRİNCİ, İKİNCİ, ÜÇÜNCÜ, DÖRDÜNCÜ, BEŞİNCİ */
                                                                                                            if (strcmp(parse_string, "ADJ$NUM+ORD") == 0) {
                                                                                                                return "ADJ";
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* AY */
    if (strcmp(parse_string, "INTERJ$NOUN+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
        return "NOUN+A3SG+PNON+NOM";
    } else {
        /* konuşmam, savunmam, etmem */
        if (strcmp(parse_string, "NEG+AOR+A1SG$POS^DB+NOUN+INF2+A3SG+P1SG+NOM") == 0) {
            return "NEG+AOR+A1SG";
        } else {
            /* YA */
            if (strcmp(parse_string, "CONJ$INTERJ") == 0) {
                if (contains_two_ne_or_ya(fsm_parses, "ya", length)) {
                    return "CONJ";
                }
                if (next_word_exists(index, length) && strcmp(((Fsm_parse_ptr) array_list_get(fsm_parses[index + 1]->fsm_parses, 0))->form, "da") == 0) {
                    return "CONJ";
                }
                return "INTERJ";
            } else {
                if (strcmp(parse_string, "A3PL+P3PL$A3PL+P3SG$A3SG+P3PL") == 0) {
                    if (is_possessive_plural(index, correct_parses)) {
                        return "A3SG+P3PL";
                    }
                    return "A3PL+P3SG";
                } else {
                    /* YÜZDE, YÜZLÜ */
                    if (strcmp(parse_string, "NOUN$NUM+CARD^DB+NOUN+ZERO") == 0) {
                        return "NOUN";
                    } else {
                        /* almanlar, uzmanlar, elmaslar, katiller */
                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PRES+A3PL$NOUN+A3PL+PNON+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL") == 0) {
                            return "NOUN+A3PL+PNON+NOM";
                        } else {
                            /* fazlası, yetkilisi */
                            if (strcmp(parse_string, "ADJ+JUSTLIKE$NOUN+ZERO+A3SG+P3SG+NOM") == 0) {
                                return "NOUN+ZERO+A3SG+P3SG+NOM";
                            } else {
                                /* HERKES, HERKESTEN, HERKESLE, HERKES */
                                if (strcmp(parse_string, "NOUN+A3SG+PNON$PRON+QUANTP+A3PL+P3PL") == 0) {
                                    return "PRON+QUANTP+A3PL+P3PL";
                                } else {
                                    /* BEN, BENDEN, BENCE, BANA, BENDE */
                                    if (strcmp(parse_string, "NOUN+A3SG$PRON+PERS+A1SG") == 0) {
                                        return "PRON+PERS+A1SG";
                                    } else {
                                        /* karşısından, geriye, geride */
                                        if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO$NOUN") == 0) {
                                            return "ADJ^DB+NOUN+ZERO";
                                        } else {
                                            /* gideceği, kalacağı */
                                            if (strcmp(parse_string, "ADJ+FUTPART+P3SG$NOUN+FUTPART+A3SG+P3SG+NOM") == 0) {
                                                if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                    return "ADJ+FUTPART+P3SG";
                                                }
                                                return "NOUN+FUTPART+A3SG+P3SG+NOM";
                                            } else {
                                                /* bildiğimiz, geçtiğimiz, yaşadığımız */
                                                if (strcmp(parse_string, "ADJ+PASTPART+P1PL$NOUN+PASTPART+A3SG+P1PL+NOM") == 0) {
                                                    return "ADJ+PASTPART+P1PL";
                                                } else {
                                                    /* eminim, memnunum, açım */
                                                    if (strcmp(parse_string, "NOUN+ZERO+A3SG+P1SG+NOM$VERB+ZERO+PRES+A1SG") == 0) {
                                                        return "VERB+ZERO+PRES+A1SG";
                                                    } else {
                                                        /* yaparlar, olabilirler, değiştirirler */
                                                        if (strcmp(parse_string, "AOR+A3PL$AOR^DB+ADJ+ZERO^DB+NOUN+ZERO+A3PL+PNON+NOM") == 0) {
                                                            return "AOR+A3PL";
                                                        } else {
                                                            /* san, yasa */
                                                            if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$NOUN+PROP+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                                if (index > 0) {
                                                                    return "NOUN+PROP+A3SG+PNON+NOM";
                                                                }
                                                            } else {
                                                                /* etmeyecek, yapmayacak, koşmayacak */
                                                                if (strcmp(parse_string, "NEG+FUT+A3SG$NEG^DB+ADJ+FUTPART+PNON") == 0) {
                                                                    return "NEG+FUT+A3SG";
                                                                } else {
                                                                    /* etmeli, olmalı */
                                                                    if (strcmp(parse_string, "POS+NECES+A3SG$POS^DB+NOUN+INF2+A3SG+PNON+NOM^DB+ADJ+WITH") == 0) {
                                                                        if (is_before_last_word(index, length)) {
                                                                            return "POS+NECES+A3SG";
                                                                        }
                                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                                            return "POS^DB+NOUN+INF2+A3SG+PNON+NOM^DB+ADJ+WITH";
                                                                        }
                                                                        return "POS+NECES+A3SG";
                                                                    } else {
                                                                        /* DE */
                                                                        if (strcmp(parse_string, "CONJ$NOUN+PROP+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                                            if (index > 0) {
                                                                                return "NOUN+PROP+A3SG+PNON+NOM";
                                                                            }
                                                                        } else {
                                                                            /* GEÇ, SIK */
                                                                            if (strcmp(parse_string, "ADJ$ADV$VERB+POS+IMP+A2SG") == 0) {
                                                                                if (strcmp(surfaceForm, "sık") == 0) {
                                                                                    char* previousWord;
                                                                                    char* nextWord;
                                                                                    if (index - 1 > -1) {
                                                                                        previousWord = ((Fsm_parse_ptr) array_list_get(fsm_parses[index - 1]->fsm_parses, 0))->form;
                                                                                    }
                                                                                    if (index + 1 < length) {
                                                                                        nextWord = ((Fsm_parse_ptr) array_list_get(fsm_parses[index + 1]->fsm_parses, 0))->form;
                                                                                    }
                                                                                    if (strcmp(previousWord, "sık") == 0 || strcmp(nextWord, "sık") == 0) {
                                                                                        return "ADV";
                                                                                    }
                                                                                }
                                                                                if (is_next_word_noun(index, fsm_parses, length)) {
                                                                                    return "ADJ";
                                                                                }
                                                                                return "ADV";
                                                                            } else {
                                                                                /* BİRLİKTE */
                                                                                if (strcmp(parse_string, "ADV$POSTP+PCINS") == 0) {
                                                                                    if (has_previous_word_tag(index, correct_parses, INSTRUMENTAL)) {
                                                                                        return "POSTP+PCINS";
                                                                                    }
                                                                                    return "ADV";
                                                                                } else {
                                                                                    /* yavaşça, dürüstçe, fazlaca */
                                                                                    if (strcmp(parse_string, "ADJ+ASIF$ADV+LY$NOUN+ZERO+A3SG+PNON+EQU") == 0) {
                                                                                        return "ADV+LY";
                                                                                    } else {
                                                                                        /* FAZLADIR, FAZLAYDI, ÇOKTU, ÇOKTUR */
                                                                                        if (strcmp(parse_string, "ADJ^DB$POSTP+PCABL^DB") == 0) {
                                                                                            if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
                                                                                                return "POSTP+PCABL^DB";
                                                                                            }
                                                                                            return "ADJ^DB";
                                                                                        } else {
                                                                                            /* kaybettikleri, umdukları, gösterdikleri */
                                                                                            if (strcmp(parse_string, "ADJ+PASTPART+P3PL$NOUN+PASTPART+A3PL+P3PL+NOM$NOUN+PASTPART+A3PL+P3SG+NOM$NOUN+PASTPART+A3SG+P3PL+NOM") == 0) {
                                                                                                if (is_next_word_noun_or_adjective(index, fsm_parses, length)) {
                                                                                                    return "ADJ+PASTPART+P3PL";
                                                                                                }
                                                                                                if (is_possessive_plural(index, correct_parses)) {
                                                                                                    return "NOUN+PASTPART+A3SG+P3PL+NOM";
                                                                                                }
                                                                                                return "NOUN+PASTPART+A3PL+P3SG+NOM";
                                                                                            } else {
                                                                                                /* yılın, yolun */
                                                                                                if (strcmp(parse_string, "NOUN+A3SG+P2SG+NOM$NOUN+A3SG+PNON+GEN$VERB+POS+IMP+A2PL$VERB^DB+VERB+PASS+POS+IMP+A2SG") == 0) {
                                                                                                    if (is_any_word_second_person(index, correct_parses)) {
                                                                                                        return "NOUN+A3SG+P2SG+NOM";
                                                                                                    }
                                                                                                    return "NOUN+A3SG+PNON+GEN";
                                                                                                } else {
                                                                                                    /* sürmekte, beklenmekte, değişmekte */
                                                                                                    if (strcmp(parse_string, "POS+PROG2+A3SG$POS^DB+NOUN+INF+A3SG+PNON+LOC") == 0) {
                                                                                                        return "POS+PROG2+A3SG";
                                                                                                    } else {
                                                                                                        /* KİMSE, KİMSEDE, KİMSEYE */
                                                                                                        if (strcmp(parse_string, "NOUN+A3SG+PNON$PRON+QUANTP+A3SG+P3SG") == 0) {
                                                                                                            return "PRON+QUANTP+A3SG+P3SG";
                                                                                                        } else {
                                                                                                            /* DOĞRU */
                                                                                                            if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM$POSTP+PCDAT") == 0) {
                                                                                                                if (has_previous_word_tag(index, correct_parses, DATIVE)) {
                                                                                                                    return "POSTP+PCDAT";
                                                                                                                }
                                                                                                                return "ADJ";
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* ikisini, ikisine, fazlasına */
    if (strcmp(parse_string, "ADJ+JUSTLIKE^DB+NOUN+ZERO+A3SG+P2SG$NOUN+ZERO+A3SG+P3SG") == 0) {
        return "NOUN+ZERO+A3SG+P3SG";
    } else {
        /* kişilerdir, aylardır, yıllardır */
        if (strcmp(parse_string, "A3PL+PNON+NOM^DB+ADV+SINCE$A3PL+PNON+NOM^DB+VERB+ZERO+PRES+COP+A3SG$A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL+COP") == 0) {
            if (string_in_list(root, (char*[]) {"yıl", "süre", "zaman", "ay"}, 4)) {
                return "A3PL+PNON+NOM^DB+ADV+SINCE";
            } else {
                return "A3PL+PNON+NOM^DB+VERB+ZERO+PRES+COP+A3SG";
            }
        } else {
            /* HEP */
            if (strcmp(parse_string, "ADV$PRON+QUANTP+A3SG+P3SG+NOM") == 0) {
                return "ADV";
            } else {
                /* O */
                if (strcmp(parse_string, "DET$NOUN+PROP+A3SG+PNON+NOM$PRON+DEMONSP+A3SG+PNON+NOM$PRON+PERS+A3SG+PNON+NOM") == 0) {
                    if (is_next_word_noun(index, fsm_parses, length)){
                        return "DET";
                    } else {
                        return "PRON+PERS+A3SG+PNON+NOM";
                    }
                } else {
                    /* yapmalıyız, etmeliyiz, alınmalıdır */
                    if (strcmp(parse_string, "POS+NECES$POS^DB+NOUN+INF2+A3SG+PNON+NOM^DB+ADJ+WITH^DB+VERB+ZERO+PRES") == 0) {
                        return "POS+NECES";
                    } else {
                        /* kızdı, çekti, bozdu */
                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO$VERB+POS") == 0) {
                            return "VERB+POS";
                        } else {
                            /* BİZİMLE */
                            if (strcmp(parse_string, "NOUN+A3SG+P1SG$PRON+PERS+A1PL+PNON") == 0) {
                                return "PRON+PERS+A1PL+PNON";
                            } else {
                                /* VARDIR */
                                if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PRES+COP+A3SG$VERB^DB+VERB+CAUS+POS+IMP+A2SG") == 0) {
                                    return "ADJ^DB+VERB+ZERO+PRES+COP+A3SG";
                                } else {
                                    /* Mİ */
                                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$QUES+PRES+A3SG") == 0) {
                                        return "QUES+PRES+A3SG";
                                    } else {
                                        /* BENİM */
                                        if (strcmp(parse_string, "NOUN+A3SG+P1SG+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A1SG$PRON+PERS+A1SG+PNON+GEN$PRON+PERS+A1SG+PNON+NOM^DB+VERB+ZERO+PRES+A1SG") == 0) {
                                            return "PRON+PERS+A1SG+PNON+GEN";
                                        } else {
                                            /* SUN */
                                            if (strcmp(parse_string, "NOUN+PROP+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                return "NOUN+PROP+A3SG+PNON+NOM";
                                            } else {
                                                if (strcmp(parse_string, "ADJ+JUSTLIKE$NOUN+ZERO+A3SG+P3SG+NOM$NOUN+ZERO^DB+ADJ+ALMOST") == 0) {
                                                    return "NOUN+ZERO+A3SG+P3SG+NOM";
                                                } else {
                                                    /* düşündük, ettik, kazandık */
                                                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PAST+A1PL$VERB+POS+PAST+A1PL$VERB+POS^DB+ADJ+PASTPART+PNON$VERB+POS^DB+NOUN+PASTPART+A3SG+PNON+NOM") == 0) {
                                                        return "VERB+POS+PAST+A1PL";
                                                    } else {
                                                        /* komiktir, eksiktir, mevcuttur, yoktur */
                                                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PRES+COP+A3SG$NOUN+A3SG+PNON+NOM^DB+ADV+SINCE$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+COP+A3SG") == 0) {
                                                            return "ADJ^DB+VERB+ZERO+PRES+COP+A3SG";
                                                        } else {
                                                            /* edeceğim, ekeceğim, koşacağım, gideceğim, savaşacağım, olacağım  */
                                                            if (strcmp(parse_string, "POS+FUT+A1SG$POS^DB+ADJ+FUTPART+P1SG$POS^DB+NOUN+FUTPART+A3SG+P1SG+NOM") == 0) {
                                                                return "POS+FUT+A1SG";
                                                            } else {
                                                                /* A */
                                                                if (strcmp(parse_string, "ADJ$INTERJ$NOUN+PROP+A3SG+PNON+NOM") == 0) {
                                                                    return "NOUN+PROP+A3SG+PNON+NOM";
                                                                } else {
                                                                    /* BİZİ */
                                                                    if (strcmp(parse_string, "NOUN+A3SG+P3SG+NOM$NOUN+A3SG+PNON+ACC$PRON+PERS+A1PL+PNON+ACC") == 0) {
                                                                        return "PRON+PERS+A1PL+PNON+ACC";
                                                                    } else {
                                                                        /* BİZİM */
                                                                        if (strcmp(parse_string, "NOUN+A3SG+P1SG+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A1SG$PRON+PERS+A1PL+PNON+GEN$PRON+PERS+A1PL+PNON+NOM^DB+VERB+ZERO+PRES+A1SG") == 0) {
                                                                            return "PRON+PERS+A1PL+PNON+GEN";
                                                                        } else {
                                                                            /* erkekler, kadınlar, madenler, uzmanlar*/
                                                                            if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PRES+A3PL$NOUN+A3PL+PNON+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL$NOUN+PROP+A3PL+PNON+NOM") == 0) {
                                                                                return "NOUN+A3PL+PNON+NOM";
                                                                            } else {
                                                                                /* TABİ */
                                                                                if (strcmp(parse_string, "ADJ$INTERJ") == 0) {
                                                                                    return "ADJ";
                                                                                } else {
                                                                                    if (strcmp(parse_string, "AOR+A2PL$AOR^DB+ADJ+ZERO^DB+ADJ+JUSTLIKE^DB+NOUN+ZERO+A3SG+P2PL+NOM") == 0) {
                                                                                        return "AOR+A2PL";
                                                                                    } else {
                                                                                        /* ayın, düşünün*/
                                                                                        if (strcmp(parse_string, "NOUN+A3SG+P2SG+NOM$NOUN+A3SG+PNON+GEN$VERB+POS+IMP+A2PL") == 0) {
                                                                                            if (is_before_last_word(index, length)){
                                                                                                return "VERB+POS+IMP+A2PL";
                                                                                            }
                                                                                            return "NOUN+A3SG+PNON+GEN";
                                                                                        } else {
                                                                                            /* ödeyecekler, olacaklar */
                                                                                            if (strcmp(parse_string, "POS+FUT+A3PL$POS^DB+NOUN+FUTPART+A3PL+PNON+NOM") == 0) {
                                                                                                return "POS+FUT+A3PL";
                                                                                            } else {
                                                                                                /* 9:30'daki */
                                                                                                if (strcmp(parse_string, "P3SG$PNON") == 0) {
                                                                                                    return "PNON";
                                                                                                } else {
                                                                                                    /* olabilecek, yapabilecek */
                                                                                                    if (strcmp(parse_string, "ABLE+FUT+A3SG$ABLE^DB+ADJ+FUTPART+PNON") == 0) {
                                                                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                                                                            return "ABLE^DB+ADJ+FUTPART+PNON";
                                                                                                        }
                                                                                                        return "ABLE+FUT+A3SG";
                                                                                                    } else {
                                                                                                        /* düşmüş duymuş artmış */
                                                                                                        if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+NARR+A3SG$VERB+POS+NARR+A3SG$VERB+POS+NARR^DB+ADJ+ZERO") == 0) {
                                                                                                            if (is_before_last_word(index, length)){
                                                                                                                return "VERB+POS+NARR+A3SG";
                                                                                                            }
                                                                                                            return "VERB+POS+NARR^DB+ADJ+ZERO";
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* BERİ, DIŞARI, AŞAĞI */
    if (strcmp(parse_string, "ADJ$ADV$NOUN+A3SG+PNON+NOM$POSTP+PCABL") == 0) {
        if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
            return "POSTP+PCABL";
        }
        return "ADV";
    } else {
        /* TV, CD */
        if (strcmp(parse_string, "A3SG+PNON+ACC$PROP+A3SG+PNON+NOM") == 0) {
            return "A3SG+PNON+ACC";
        } else {
            /* değinmeyeceğim, vermeyeceğim */
            if (strcmp(parse_string, "NEG+FUT+A1SG$NEG^DB+ADJ+FUTPART+P1SG$NEG^DB+NOUN+FUTPART+A3SG+P1SG+NOM") == 0) {
                return "NEG+FUT+A1SG";
            } else {
                /* görünüşe, satışa, duruşa */
                if (strcmp(parse_string, "POS^DB+NOUN+INF3+A3SG+PNON+DAT$RECIP+POS+OPT+A3SG") == 0) {
                    return "POS^DB+NOUN+INF3+A3SG+PNON+DAT";
                } else {
                    /* YILDIR, AYDIR, YOLDUR */
                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+ADV+SINCE$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+COP+A3SG$VERB^DB+VERB+CAUS+POS+IMP+A2SG") == 0) {
                        if (string_in_list(root, (char*[]) {"yıl", "ay"}, 2)) {
                            return "NOUN+A3SG+PNON+NOM^DB+ADV+SINCE";
                        } else {
                            return "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+COP+A3SG";
                        }
                    } else {
                        /* BENİ */
                        if (strcmp(parse_string, "NOUN+A3SG+P3SG+NOM$NOUN+A3SG+PNON+ACC$PRON+PERS+A1SG+PNON+ACC") == 0) {
                            return "PRON+PERS+A1SG+PNON+ACC";
                        } else {
                            /* edemezsin, kanıtlarsın, yapamazsın */
                            if (strcmp(parse_string, "AOR+A2SG$AOR^DB+ADJ+ZERO^DB+ADJ+JUSTLIKE^DB+NOUN+ZERO+A3SG+P2SG+NOM") == 0) {
                                return "AOR+A2SG";
                            } else {
                                /* BÜYÜME, ATAMA, KARIMA, KORUMA, TANIMA, ÜREME */
                                if (strcmp(parse_string, "NOUN+A3SG+P1SG+DAT$VERB+NEG+IMP+A2SG$VERB+POS^DB+NOUN+INF2+A3SG+PNON+NOM") == 0) {
                                    if (strcmp(root, "karı") == 0){
                                        return "NOUN+A3SG+P1SG+DAT";
                                    }
                                    return "VERB+POS^DB+NOUN+INF2+A3SG+PNON+NOM";
                                } else {
                                    /* HANGİ */
                                    if (strcmp(parse_string, "ADJ$PRON+QUESP+A3SG+PNON+NOM") == 0) {
                                        if (strcmp(lastWord, "?") == 0) {
                                            return "PRON+QUESP+A3SG+PNON+NOM";
                                        }
                                        return "ADJ";
                                    } else {
                                        /* GÜCÜNÜ, GÜCÜNÜN, ESASINDA */
                                        if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3SG+P2SG$ADJ^DB+NOUN+ZERO+A3SG+P3SG$NOUN+A3SG+P2SG$NOUN+A3SG+P3SG") == 0) {
                                            return "NOUN+A3SG+P3SG";
                                        } else {
                                            /* YILININ, YOLUNUN, DİLİNİN */
                                            if (strcmp(parse_string, "NOUN+A3SG+P2SG+GEN$NOUN+A3SG+P3SG+GEN$VERB^DB+VERB+PASS+POS+IMP+A2PL") == 0) {
                                                return "NOUN+A3SG+P3SG+GEN";
                                            } else {
                                                /* ÇIKARDI */
                                                if (strcmp(parse_string, "VERB+POS+AOR$VERB^DB+VERB+CAUS+POS") == 0) {
                                                    return "VERB+POS+AOR";
                                                } else {
                                                    /* sunucularımız, rakiplerimiz, yayınlarımız */
                                                    if (strcmp(parse_string, "P1PL+NOM$P1SG+NOM^DB+VERB+ZERO+PRES+A1PL") == 0) {
                                                        return "P1PL+NOM";
                                                    } else {
                                                        /* etmiştir, artmıştır, düşünmüştür, alınmıştır */
                                                        if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+NARR+A3SG+COP$VERB+POS+NARR+COP+A3SG") == 0) {
                                                            return "VERB+POS+NARR+COP+A3SG";
                                                        } else {
                                                            /* hazırlandı, yuvarlandı, temizlendi */
                                                            if (strcmp(parse_string, "VERB+REFLEX$VERB^DB+VERB+PASS") == 0) {
                                                                return "VERB^DB+VERB+PASS";
                                                            } else {
                                                                /* KARA, ÇEK, SOL, KOCA */
                                                                if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM$NOUN+PROP+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                                    if (index > 0) {
                                                                        if (is_fsm_parse_capital_word(array_list_get(fsm_parses[index]->fsm_parses, 0))) {
                                                                            return "NOUN+PROP+A3SG+PNON+NOM";
                                                                        }
                                                                        return "ADJ";
                                                                    }
                                                                } else {
                                                                    /* YÜZ */
                                                                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$NUM+CARD$VERB+POS+IMP+A2SG") == 0) {
                                                                        if (is_next_word_num(index, fsm_parses, length)){
                                                                            return "NUM+CARD";
                                                                        }
                                                                        return "NOUN+A3SG+PNON+NOM";
                                                                    } else {
                                                                        if (strcmp(parse_string, "ADJ+AGT^DB+ADJ+JUSTLIKE$NOUN+AGT+A3SG+P3SG+NOM$NOUN+AGT^DB+ADJ+ALMOST") == 0) {
                                                                            return "NOUN+AGT+A3SG+P3SG+NOM";
                                                                        } else {
                                                                            /* artışın, düşüşün, yükselişin*/
                                                                            if (strcmp(parse_string, "POS^DB+NOUN+INF3+A3SG+P2SG+NOM$POS^DB+NOUN+INF3+A3SG+PNON+GEN$RECIP+POS+IMP+A2PL") == 0) {
                                                                                if (is_any_word_second_person(index, correct_parses)){
                                                                                    return "POS^DB+NOUN+INF3+A3SG+P2SG+NOM";
                                                                                }
                                                                                return "POS^DB+NOUN+INF3+A3SG+PNON+GEN";
                                                                            } else {
                                                                                /* VARSA */
                                                                                if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+COND$VERB+POS+DESR") == 0) {
                                                                                    return "ADJ^DB+VERB+ZERO+COND";
                                                                                } else {
                                                                                    /* DEK */
                                                                                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$POSTP+PCDAT") == 0) {
                                                                                        return "POSTP+PCDAT";
                                                                                    } else {
                                                                                        /* ALDIK */
                                                                                        if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PAST+A1PL$VERB+POS+PAST+A1PL$VERB+POS^DB+ADJ+PASTPART+PNON$VERB+POS^DB+NOUN+PASTPART+A3SG+PNON+NOM") == 0) {
                                                                                            return "VERB+POS+PAST+A1PL";
                                                                                        } else {
                                                                                            /* BİRİNİN, BİRİNE, BİRİNİ, BİRİNDEN */
                                                                                            if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3SG+P2SG$ADJ^DB+NOUN+ZERO+A3SG+P3SG$NUM+CARD^DB+NOUN+ZERO+A3SG+P2SG$NUM+CARD^DB+NOUN+ZERO+A3SG+P3SG") == 0) {
                                                                                                return "NUM+CARD^DB+NOUN+ZERO+A3SG+P3SG";
                                                                                            } else {
                                                                                                /* ARTIK */
                                                                                                if (strcmp(parse_string, "ADJ$ADV$NOUN+A3SG+PNON+NOM$NOUN+PROP+A3SG+PNON+NOM") == 0) {
                                                                                                    return "ADV";
                                                                                                } else {
                                                                                                    /* BİRİ */
                                                                                                    if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3SG+P3SG+NOM$ADJ^DB+NOUN+ZERO+A3SG+PNON+ACC$NUM+CARD^DB+NOUN+ZERO+A3SG+P3SG+NOM$NUM+CARD^DB+NOUN+ZERO+A3SG+PNON+ACC") == 0) {
                                                                                                        return "NUM+CARD^DB+NOUN+ZERO+A3SG+P3SG+NOM";
                                                                                                    } else {
                                                                                                        /* DOĞRU */
                                                                                                        if (strcmp(parse_string, "ADJ$NOUN+A3SG+PNON+NOM$NOUN+PROP+A3SG+PNON+NOM$POSTP+PCDAT") == 0) {
                                                                                                            if (has_previous_word_tag(index, correct_parses, DATIVE)) {
                                                                                                                return "POSTP+PCDAT";
                                                                                                            }
                                                                                                            return "ADJ";
                                                                                                        } else {
                                                                                                            /* demiryolları, havayolları, milletvekilleri */
                                                                                                            if (strcmp(parse_string, "P3PL+NOM$P3SG+NOM$PNON+ACC") == 0) {
                                                                                                                if (is_possessive_plural(index, correct_parses)){
                                                                                                                    return "P3PL+NOM";
                                                                                                                }
                                                                                                                return "P3SG+NOM";
                                                                                                            } else {
                                                                                                                /* GEREK */
                                                                                                                if (strcmp(parse_string, "CONJ$NOUN+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                                                                                    if (contains_two_ne_or_ya(fsm_parses, "gerek", length)){
                                                                                                                        return "CONJ";
                                                                                                                    }
                                                                                                                    return "NOUN+A3SG+PNON+NOM";
                                                                                                                }
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* bilmediğiniz, sevdiğiniz, kazandığınız */
    if (strcmp(parse_string, "ADJ+PASTPART+P2PL$NOUN+PASTPART+A3SG+P2PL+NOM$NOUN+PASTPART+A3SG+PNON+GEN^DB+VERB+ZERO+PRES+A1PL") == 0) {
        if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
            return "ADJ+PASTPART+P2PL";
        }
        return "NOUN+PASTPART+A3SG+P2PL+NOM";
    } else {
        /* yapabilecekleri, edebilecekleri, sunabilecekleri */
        if (strcmp(parse_string, "ADJ+FUTPART+P3PL$NOUN+FUTPART+A3PL+P3PL+NOM$NOUN+FUTPART+A3PL+P3SG+NOM$NOUN+FUTPART+A3PL+PNON+ACC$NOUN+FUTPART+A3SG+P3PL+NOM") == 0) {
            if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                return "ADJ+FUTPART+P3PL";
            }
            if (is_possessive_plural(index, correct_parses)){
                return "NOUN+FUTPART+A3SG+P3PL+NOM";
            }
            return "NOUN+FUTPART+A3PL+P3SG+NOM";
        } else {
            /* KİM */
            if (strcmp(parse_string, "NOUN+PROP$PRON+QUESP") == 0) {
                if (strcmp(lastWord, "?") == 0) {
                    return "PRON+QUESP";
                }
                return "NOUN+PROP";
            } else {
                /* ALINDI */
                if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3SG+P2SG+NOM^DB+VERB+ZERO$ADJ^DB+NOUN+ZERO+A3SG+PNON+GEN^DB+VERB+ZERO$VERB^DB+VERB+PASS+POS") == 0) {
                    return "VERB^DB+VERB+PASS+POS";
                } else {
                    /* KIZIM */
                    if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+PRES+A1SG$NOUN+A3SG+P1SG+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A1SG") == 0) {
                        return "NOUN+A3SG+P1SG+NOM";
                    } else {
                        /* etmeliydi, yaratmalıydı */
                        if (strcmp(parse_string, "POS+NECES$POS^DB+NOUN+INF2+A3SG+PNON+NOM^DB+ADJ+WITH^DB+VERB+ZERO") == 0) {
                            return "POS+NECES";
                        } else {
                            /* HERKESİN */
                            if (strcmp(parse_string, "NOUN+A3SG+P2SG+NOM$NOUN+A3SG+PNON+GEN$PRON+QUANTP+A3PL+P3PL+GEN") == 0) {
                                return "PRON+QUANTP+A3PL+P3PL+GEN";
                            } else {
                                if (strcmp(parse_string, "ADJ+JUSTLIKE^DB+NOUN+ZERO+A3SG+P2SG$ADJ+JUSTLIKE^DB+NOUN+ZERO+A3SG+PNON$NOUN+ZERO+A3SG+P3SG") == 0) {
                                    return "NOUN+ZERO+A3SG+P3SG";
                                } else {
                                    /* milyarlık, milyonluk, beşlik, ikilik */
                                    if (strcmp(parse_string, "NESS+A3SG+PNON+NOM$ZERO+A3SG+PNON+NOM^DB+ADJ+FITFOR") == 0) {
                                        return "ZERO+A3SG+PNON+NOM^DB+ADJ+FITFOR";
                                    } else {
                                        /* alınmamaktadır, koymamaktadır */
                                        if (strcmp(parse_string, "NEG+PROG2$NEG^DB+NOUN+INF+A3SG+PNON+LOC^DB+VERB+ZERO+PRES") == 0) {
                                            return "NEG+PROG2";
                                        } else {
                                            /* HEPİMİZ */
                                            if (strcmp(parse_string, "A1PL+P1PL+NOM$A3SG+P3SG+GEN^DB+VERB+ZERO+PRES+A1PL") == 0) {
                                                return "A1PL+P1PL+NOM";
                                            } else {
                                                /* KİMSENİN */
                                                if (strcmp(parse_string, "NOUN+A3SG+P2SG$NOUN+A3SG+PNON$PRON+QUANTP+A3SG+P3SG") == 0) {
                                                    return "PRON+QUANTP+A3SG+P3SG";
                                                } else {
                                                    /* GEÇMİŞ, ALMIŞ, VARMIŞ */
                                                    if (strcmp(parse_string, "ADJ^DB+VERB+ZERO+NARR+A3SG$VERB+POS+NARR+A3SG$VERB+POS+NARR^DB+ADJ+ZERO") == 0) {
                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                            return "VERB+POS+NARR^DB+ADJ+ZERO";
                                                        }
                                                        return "VERB+POS+NARR+A3SG";
                                                    } else {
                                                        /* yapacağınız, konuşabileceğiniz, olacağınız */
                                                        if (strcmp(parse_string, "ADJ+FUTPART+P2PL$NOUN+FUTPART+A3SG+P2PL+NOM$NOUN+FUTPART+A3SG+PNON+GEN^DB+VERB+ZERO+PRES+A1PL") == 0) {
                                                            if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                                return "ADJ+FUTPART+P2PL";
                                                            }
                                                            return "NOUN+FUTPART+A3SG+P2PL+NOM";
                                                        } else {
                                                            /* YILINA, DİLİNE, YOLUNA */
                                                            if (strcmp(parse_string, "NOUN+A3SG+P2SG+DAT$NOUN+A3SG+P3SG+DAT$VERB^DB+VERB+PASS+POS+OPT+A3SG") == 0) {
                                                                if (is_any_word_second_person(index, correct_parses)){
                                                                    return "NOUN+A3SG+P2SG+DAT";
                                                                }
                                                                return "NOUN+A3SG+P3SG+DAT";
                                                            } else {
                                                                /* MİSİN, MİYDİ, MİSİNİZ */
                                                                if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM^DB+VERB+ZERO$QUES") == 0) {
                                                                    return "QUES";
                                                                } else {
                                                                    /* ATAKLAR, GÜÇLER, ESASLAR */
                                                                    if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3PL+PNON+NOM$ADJ^DB+VERB+ZERO+PRES+A3PL$NOUN+A3PL+PNON+NOM$NOUN+A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL") == 0) {
                                                                        return "NOUN+A3PL+PNON+NOM";
                                                                    } else {
                                                                        if (strcmp(parse_string, "A3PL+P3SG$A3SG+P3PL$PROP+A3PL+P3PL") == 0) {
                                                                            return "PROP+A3PL+P3PL";
                                                                        } else {
                                                                            /* pilotunuz, suçunuz, haberiniz */
                                                                            if (strcmp(parse_string, "P2PL+NOM$PNON+GEN^DB+VERB+ZERO+PRES+A1PL") == 0) {
                                                                                return "P2PL+NOM";
                                                                            } else {
                                                                                /* yıllarca, aylarca, düşmanca */
                                                                                if (strcmp(parse_string, "ADJ+ASIF$ADV+LY") == 0) {
                                                                                    if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                                                        return "ADJ+ASIF";
                                                                                    }
                                                                                    return "ADV+LY";
                                                                                } else {
                                                                                    /* gerçekçi, alıcı */
                                                                                    if (strcmp(parse_string, "ADJ^DB+NOUN+AGT+A3SG+PNON+NOM$NOUN+A3SG+PNON+NOM^DB+ADJ+AGT") == 0) {
                                                                                        if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                                                            return "NOUN+A3SG+PNON+NOM^DB+ADJ+AGT";
                                                                                        }
                                                                                        return "ADJ^DB+NOUN+AGT+A3SG+PNON+NOM";
                                                                                    } else {
                                                                                        /* havayollarına, gözyaşlarına */
                                                                                        if (strcmp(parse_string, "P2SG$P3PL$P3SG") == 0) {
                                                                                            if (is_any_word_second_person(index, correct_parses)){
                                                                                                return "P2SG";
                                                                                            }
                                                                                            if (is_possessive_plural(index, correct_parses)){
                                                                                                return "P3PL";
                                                                                            }
                                                                                            return "P3SG";
                                                                                        } else {
                                                                                            /* olun, kurtulun, gelin */
                                                                                            if (strcmp(parse_string, "VERB+POS+IMP+A2PL$VERB^DB+VERB+PASS+POS+IMP+A2SG") == 0) {
                                                                                                return "VERB+POS+IMP+A2PL";
                                                                                            } else {
                                                                                                if (strcmp(parse_string, "ADJ+JUSTLIKE^DB$NOUN+ZERO+A3SG+P3SG+NOM^DB") == 0) {
                                                                                                    return "NOUN+ZERO+A3SG+P3SG+NOM^DB";
                                                                                                } else {
                                                                                                    /* oluşmaktaydı, gerekemekteydi */
                                                                                                    if (strcmp(parse_string, "POS+PROG2$POS^DB+NOUN+INF+A3SG+PNON+LOC^DB+VERB+ZERO") == 0) {
                                                                                                        return "POS+PROG2";
                                                                                                    } else {
                                                                                                        /* BERABER */
                                                                                                        if (strcmp(parse_string, "ADJ$ADV$POSTP+PCINS") == 0) {
                                                                                                            if (has_previous_word_tag(index, correct_parses, INSTRUMENTAL)) {
                                                                                                                return "POSTP+PCINS";
                                                                                                            }
                                                                                                            if (is_next_word_noun_or_adjective(index, fsm_parses, length)){
                                                                                                                return "ADJ";
                                                                                                            }
                                                                                                            return "ADV";
                                                                                                        } else {
                                                                                                            /* BİN, KIRK */
                                                                                                            if (strcmp(parse_string, "NUM+CARD$VERB+POS+IMP+A2SG") == 0) {
                                                                                                                return "NUM+CARD";
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* ÖTE */
    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$POSTP+PCABL") == 0) {
        if (has_previous_word_tag(index, correct_parses, ABLATIVE)) {
            return "POSTP+PCABL";
        }
        return "NOUN+A3SG+PNON+NOM";
    } else {
        /* BENİMLE */
        if (strcmp(parse_string, "NOUN+A3SG+P1SG$PRON+PERS+A1SG+PNON") == 0) {
            return "PRON+PERS+A1SG+PNON";
        } else {
            /* Accusative and Ablative Cases*/
            if (strcmp(parse_string, "ADV+WITHOUTHAVINGDONESO$NOUN+INF2+A3SG+PNON+ABL") == 0) {
                return "ADV+WITHOUTHAVINGDONESO";
            } else {
                if (strcmp(parse_string, "ADJ^DB+NOUN+ZERO+A3SG+P3SG+NOM$ADJ^DB+NOUN+ZERO+A3SG+PNON+ACC$NOUN+A3SG+P3SG+NOM$NOUN+A3SG+PNON+ACC") == 0) {
                    return "ADJ^DB+NOUN+ZERO+A3SG+P3SG+NOM";
                } else {
                    if (strcmp(parse_string, "P3SG+NOM$PNON+ACC") == 0) {
                        if (strcmp(((Fsm_parse_ptr)array_list_get(fsm_parses[index]->fsm_parses, 0))->pos, "PROP") == 0) {
                            return "PNON+ACC";
                        } else {
                            return "P3SG+NOM";
                        }
                    } else {
                        if (strcmp(parse_string, "A3PL+PNON+NOM$A3SG+PNON+NOM^DB+VERB+ZERO+PRES+A3PL") == 0) {
                            return "A3PL+PNON+NOM";
                        } else {
                            if (strcmp(parse_string, "ADV+SINCE$VERB+ZERO+PRES+COP+A3SG") == 0) {
                                if (string_in_list(root, (char*[]) {"yıl", "süre", "zaman", "ay"}, 4)) {
                                    return "ADV+SINCE";
                                } else {
                                    return "VERB+ZERO+PRES+COP+A3SG";
                                }
                            } else {
                                if (strcmp(parse_string, "CONJ$VERB+POS+IMP+A2SG") == 0) {
                                    return "CONJ";
                                } else {
                                    if (strcmp(parse_string, "NEG+IMP+A2SG$POS^DB+NOUN+INF2+A3SG+PNON+NOM") == 0) {
                                        return "POS^DB+NOUN+INF2+A3SG+PNON+NOM";
                                    } else {
                                        if (strcmp(parse_string, "NEG+OPT+A3SG$POS^DB+NOUN+INF2+A3SG+PNON+DAT") == 0) {
                                            return "POS^DB+NOUN+INF2+A3SG+PNON+DAT";
                                        } else {
                                            if (strcmp(parse_string, "NOUN+A3SG+P3SG+NOM$NOUN^DB+ADJ+ALMOST") == 0) {
                                                return "NOUN+A3SG+P3SG+NOM";
                                            } else {
                                                if (strcmp(parse_string, "ADJ$VERB+POS+IMP+A2SG") == 0) {
                                                    return "ADJ";
                                                } else {
                                                    if (strcmp(parse_string, "NOUN+A3SG+PNON+NOM$VERB+POS+IMP+A2SG") == 0) {
                                                        return "NOUN+A3SG+PNON+NOM";
                                                    } else {
                                                        if (strcmp(parse_string, "INF2+A3SG+P3SG+NOM$INF2^DB+ADJ+ALMOST$") == 0) {
                                                            return "INF2+A3SG+P3SG+NOM";
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

Fsm_parse_ptr case_disambiguator(int index, 
                                 Fsm_parse_list_ptr *fsm_parses, 
                                 Array_list_ptr correct_parses, 
                                 int length) {
    Fsm_parse_list_ptr fsmParseList = fsm_parses[index];
    char* caseString = parses_without_prefix_and_suffix(fsm_parses[index]);
    char* defaultCase = select_case_for_parse_string(caseString, index, fsm_parses, correct_parses, length);
    if (defaultCase != NULL) {
        for (int i = 0; i < fsmParseList->fsm_parses->size; i++) {
            Fsm_parse_ptr fsm_parse = array_list_get(fsmParseList->fsm_parses, i);
            char* list = transition_list(fsm_parse);
            if (str_contains(list, defaultCase)) {
                free_(list);
                return fsm_parse;
            }
            free_(list);
        }
    }
    return array_list_get(fsmParseList->fsm_parses, 0);
}
