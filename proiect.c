// Copyright (C) 2017 Andrei Dinu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void alloc_arena(unsigned char **arena, int arena_dim) {
    *arena = (unsigned char *)calloc(arena_dim, sizeof(char));
}

void free_arena(unsigned char **arena) {
    free(*arena);
}

void dump(unsigned char *arena, int arena_dim) {
    int i;
    for (i = 0; i < arena_dim; i++) {
        if (i % 16 == 0) {
            printf("%08X", i);
            printf("\t");
        }
        printf("%02X ", arena[i]);
        if (i % 16 == 7) {  // Adaug un spatiu intre al 8-lea si al 9-lea byte
            printf(" ");
        } else if (i % 16 == 15) {
            printf("\n");
        }
        if (i == arena_dim - 1) {
            printf("\n%08X\n", arena_dim);
        }
    }
}

void fill(unsigned char **arena, int index, int size, int value,
          int arena_dim) {
    int i;
    for (i = 0; i < size; i++) {
        if (index + i == arena_dim) {
            break;
        } else {
            (*arena)[index + i] = value;
        }
    }
}

int alloc_block(unsigned char **arena, int arena_dim, int size) {
    unsigned char *index_crt = *arena;
    int total_size = 12 + size;
    int crt_block;
    int next_block; // Retine indexul urmatorului bloc
    int aux_next_block; // Indexul de final al zonei de date a blocului curent
    int final_index;

    if (arena_dim - 4 >= total_size && *(int *)arena == 0) {
        *(int *)(*arena) = 4;
        *(int *)((*arena) + 4) = 0;
        *(int *)((*arena) + 8) = 0;
        *(int *)((*arena) + 12) = 12 + size;

        return 16;
    } else {
        aux_next_block = 4;
        next_block = *(int *)(*arena);
        crt_block = 0;

        // Parcurg arena din bloc in bloc si caut spatiu spatiu intre 2 blocuri
        while (next_block != 0) {
            // Verific daca exista spatiu liber intre blocul curent si
            // urmatorul bloc.
            if (next_block - aux_next_block >= total_size) {
                // Configurez sectiunea de gestiune a blocului
                *(int *)((*arena) + aux_next_block) = next_block;
                *(int *)((*arena) + aux_next_block + 4) = crt_block;
                *(int *)((*arena) + aux_next_block + 8) = 12 + size;
                *(int *)((*arena) + next_block + 4) = aux_next_block;
                *(int *)index_crt = aux_next_block;
                final_index = aux_next_block + 12;

                return final_index;
            }
            index_crt = *arena + *(int *)index_crt;  // Trec la blocul urmator
            crt_block = next_block;
            next_block = *(int *)index_crt;
            aux_next_block = crt_block + *(int *)(index_crt + 8);
        }

        // Daca am ajuns la acest if inseamna ca nu am gasit spatiu intre
        // 2 blocuri.
        // Incerc sa aloc sptaiu pentru noul bloc la sfarsitul arenei
        if (arena_dim - aux_next_block >= total_size) {
            *(int *)index_crt = aux_next_block;
            *(int *)((*arena) + aux_next_block) = 0;
            *(int *)((*arena) + aux_next_block + 4) = crt_block;
            *(int *)((*arena) + aux_next_block + 8) = total_size;
            final_index = aux_next_block + 12;

            return final_index;
        }
    }

    return 0;
}

void free_block(unsigned char **arena, int free_index) {
    int aux_next;
    int aux_prev;
    int aux;

    // Daca urmatorul bloc este 0 inseamna ca eliberez ultimul bloc din memorie
    // Aplic schimbari doar in gestiunea blocului precedent
    // In caz contrat aplic schimbari si in gestiunea blocului din fata si in
    // cea a blocului din spate
    if (*(int *)((*arena) + free_index - 12) == 0) {
        aux = *(int *)((*arena) + free_index - 8);
        *(int *)((*arena) + aux) = 0;
    } else {
        aux_prev = *(int *)((*arena) + free_index - 12);
        aux = *(int *)((*arena) + free_index - 8);
        *(int *)((*arena) + aux) = aux_prev;

        aux_next = *(int *)((*arena) + free_index - 8);
        aux = *(int *)((*arena) + free_index - 12);
        *(int *)((*arena) + aux + 4) = aux_next;
    }
}

void show_free(unsigned char *arena, int arena_dim) {
    unsigned char *index_crt = arena;
    int blocks = 0;
    int bytes = 0;
    int busy = 0;
    int aux;

    // Verific daca indexul de start e lipit de primul bloc
    if ((*(int *)(arena) == 4)) {
        index_crt = arena + 4;
        aux = 4;
        busy += 4;
    } else if (*(int *)arena != 4 && *(int *)arena != 0) {
        blocks++;
        busy += 4;
        aux = *(int *)arena;
        index_crt = arena + aux;
    } else {
        busy += 4;
        aux = *(int *)arena;
        index_crt = arena + aux;
    }

    while (*(int *)index_crt != 0) {
        // Verific daca blocul este lipit de cel din dreapta lui
        if (aux + *(int *)(index_crt + 8) == *(int *)(index_crt)) {
              busy += *(int *)(index_crt + 8);
              aux = *(int *)index_crt;
              index_crt = arena + *(int *)index_crt;
        } else {
            aux = *(int *)index_crt;
            busy += *(int *)(index_crt + 8);
            index_crt = arena + *(int *)index_crt;
            blocks++;
        }
    }

    if (arena_dim - (aux + *(int *)(index_crt + 8)) != 0) {
        blocks++;
    }
    busy += *(int *)(index_crt + 8);
    bytes = arena_dim - busy;
    printf("%d blocks (%d bytes) free\n", blocks, bytes);
}

void show_usage(unsigned char *arena, int arena_dim) {
    unsigned char *index_crt = arena;
    int occupied_bytes = 0;
    int blocks = 0;
    int used_bytes = 0;
    int reserved_bytes = 0;
    int free_blocks = 0;
    int aux;

    // Verific daca arena este complet goala
    if (*(int *)arena == 0) {
        printf("0 blocks (0 bytes) used\n");
        printf("0%% efficiency\n");
        printf("0%% fragmentation\n");
        return;
    } else {
        // Calculez nr de blocuri ce contin date si cati octeti ocupa
        index_crt = arena + *(int *)arena;
        while (*(int *)index_crt != 0) {
            occupied_bytes += *(int *)(index_crt + 8) - 12;
            index_crt = arena + *(int *)(index_crt);
            blocks++;
        }
        occupied_bytes += *(int *)(index_crt + 8) - 12;
        blocks++;
        printf("%d blocks (%d bytes) used\n", blocks, occupied_bytes);

        index_crt = arena + *(int *)arena;
        reserved_bytes += 4;

        // Numar octetii ocupati/rezervati si calculez eficienta
        while (*(int *)index_crt != 0) {
            reserved_bytes += *(int *)(index_crt + 8);
            used_bytes += *(int *)(index_crt + 8) - 12;
            index_crt = arena + *(int *)(index_crt);
        }
        reserved_bytes += *(int *)(index_crt + 8);
        used_bytes += *(int *)(index_crt + 8) - 12;
        printf("%d%% efficiency\n", 100 * used_bytes / reserved_bytes);

        // Nr. cate blocuri goale sunt si calculez fragmentarea
        index_crt = arena + *(int *)arena;
        aux = *(int *)arena;
        if (aux != 4) {
            free_blocks++;
        }
        while (*(int *)index_crt != 0) {
            // Verific daca blocul este lipit de cel din dreapta lui
            if (aux + *(int *)(index_crt + 8) != *(int *)(index_crt)) {
                free_blocks++;
            }

            aux = *(int *)index_crt;
            index_crt = arena + *(int *)index_crt;
        }
        if (arena_dim - aux - *(int *)(index_crt + 8) != 0) {
            free_blocks++;
        }
        printf("%d%% fragmentation\n", 100 * (free_blocks - 1) / blocks);
    }
}

void show_allocations(unsigned char *arena, int arena_dim) {
    unsigned char *index_crt = arena;
    int aux;

    // Intotdeana voi avea 4 octeti ocupati datorita indicelui de start
    printf("OCCUPIED 4 bytes\n");

    if (*(int *)(arena) == 4) {
        aux = 4;
        index_crt = arena + 4;
    } else if (*(int *)arena != 4 && *(int *)arena != 0) {
        aux = *(int *)index_crt;
        printf("FREE %d bytes\n", (*(int *)(index_crt)) - 4);
        index_crt = arena + *(int *)arena;
    } else {
        printf("FREE %d bytes\n", arena_dim - 4);
        return;
    }

    // Parcurg blocurile pana la intalnirea ultimului
    while (*(int *)index_crt != 0) {
        if (aux + *(int *)(index_crt + 8) == *(int *)(index_crt)) {
            printf("OCCUPIED %d bytes\n", *(int *)(index_crt + 8));
            aux = *(int *)index_crt;
            index_crt = arena + *(int *)index_crt;

        } else {
            printf("OCCUPIED %d bytes\n", *(int *)(index_crt + 8));
            printf("FREE %d bytes\n", *(int *)(index_crt) -
                                      *(int *)(index_crt + 8) - aux);
            aux = *(int *)index_crt;
            index_crt = arena + *(int *)index_crt;
        }
    }
    printf("OCCUPIED %d bytes\n", *(int *)(index_crt + 8));

    aux += *(int *)(index_crt + 8);
    // Afisez spatiul gol dintre ultimul bloc si capatul arenei, daca exista
    if (arena_dim - aux != 0) {
        printf("FREE %d bytes\n", arena_dim - aux);
    }
}

int main() {
    char cmd[20], cmd_2[20];
    unsigned char *arena;
    int arena_dim;
    int index, size, value;
    int block_dim;
    int free_index;

    while (1) {
        scanf("%s", cmd);

        if (!strcmp(cmd, "INITIALIZE")) {
            scanf("%d", &arena_dim);
            alloc_arena(&arena, arena_dim);
            if (!arena) {
                exit(1);
            }
        } else if (!strcmp(cmd, "FINALIZE")) {
            free_arena(&arena);
            if (arena) {
                exit(2);
            }

            return 0;
        } else if (!strcmp(cmd, "DUMP")) {
            dump(arena, arena_dim);
        } else if (!strcmp(cmd, "FILL")) {
            scanf("%d %d %d", &index, &size, &value);
            fill(&arena, index, size, value, arena_dim);
        } else if (!strcmp(cmd, "ALLOC")) {
            scanf("%d", &block_dim);
            printf("%d\n", alloc_block(&arena, arena_dim, block_dim));
        } else if (!strcmp(cmd, "FREE")) {
            scanf("%d", &free_index);
            free_block(&arena, free_index);
        } else if (!strcmp(cmd, "SHOW")) {
            scanf("%s", cmd_2);

            if (!strcmp(cmd_2, "FREE")) {
                show_free(arena, arena_dim);
            } else if (!strcmp(cmd_2, "USAGE")) {
                show_usage(arena, arena_dim);
            } else if (!strcmp(cmd_2, "ALLOCATIONS")) {
                show_allocations(arena, arena_dim);
            }
        }
    }

    return 0;
}

