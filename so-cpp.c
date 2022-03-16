#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct hashmap_entry_node {
    char *symbol;
    char *mapping;
    int symbol_len;
    int mapping_len;
    struct hashmap_entry_node *next;
} hashmap_entry;

typedef struct {
    hashmap_entry **entries;
    int size;
} hashmap;

hashmap *create_map(int size) {
    hashmap *map = malloc(sizeof(hashmap));
    map->size = size;
    map->entries = malloc(size * sizeof(hashmap_entry*));
    for (int i = 0; i < size; i++) {
        map->entries[i] = NULL;
    }
    return map;
}

void print_table(hashmap *map) {
    for (int i = 0; i < map->size; i++) {
        hashmap_entry *it = map->entries[i];
        printf("%d. ", i);
        while (it) {
            printf("(%s, %s)", it->symbol, it->mapping);
            it = it->next;
        }
        printf("\n");
    }
}

int hash(char *str) {
	int hash = 5381, c, i = 0;

	c = *str;
	while (c != '\0') {
		i++;
		hash = ((hash << 5) + hash) + c;
		c = str[i];
	}
	if (hash < 0)
		hash = -hash;
	return hash;
}

void add_hashmap_entry(char *symbol, char *mapping, hashmap *map, int size) {
    hashmap_entry *new_entry = malloc(sizeof(hashmap_entry));
    new_entry->symbol = symbol;
    new_entry->mapping = mapping;
    new_entry->next = NULL;
    new_entry->symbol_len = strlen(symbol);
    new_entry->mapping_len = strlen(mapping);

    hashmap_entry *entry = map->entries[hash(symbol) % size];
    if (entry) {
        while (entry->next) {
            entry = entry->next;
        }
        entry->next = new_entry;
    } else {
        map->entries[hash(symbol) % size] = new_entry;
    }
}

void delete_hashmap_entry(char *symbol, hashmap *map, int size) {
    hashmap_entry *it = map->entries[hash(symbol) % size];
    hashmap_entry *prev_it = NULL;

    while (it) {
        if (strcmp(it->symbol, symbol) == 0) {
            free(it->symbol);
            free(it->mapping);
            if (prev_it == NULL) {
                map->entries[hash(symbol) % size] = it->next;
                free(it);
            } else {
                prev_it->next = it->next;
                free(it);                
            }
            return;
        }
        prev_it = it;
        it = it->next;
    }
}

int add_to_hashmap(char **argv, hashmap *map, int index, int size) {
    int current_index = index;
    int beginning = 0;
    int ret_value = 0;
    if (strlen(argv[index]) > 2) {
        beginning = 2;
    } else {
        current_index++;
        ret_value = 1;
    } 

    if (argv[index][1] == 'D') {
        char *symbol, *mapping;
        int count = 0;

        for (int i = beginning; i < strlen(argv[current_index]); i++) {
            if (argv[current_index][i] == '=')
                break;
            count++;
        }
        symbol = malloc((count + 1) * sizeof(char));
        strncpy(symbol, argv[current_index] + beginning, count);
        symbol[count] = '\0';

        if (strlen(argv[current_index]) > beginning + count) {
            int mapping_length = strlen(argv[current_index]) - beginning - count - 1;
            mapping = malloc((mapping_length + 1) * sizeof(char));
            strncpy(mapping, argv[current_index] + beginning + 1 + count,
                mapping_length);
            mapping[mapping_length] = '\0';
        } else {
            mapping = malloc(sizeof(char));
            mapping[0] = '\0';
        }

        add_hashmap_entry(symbol, mapping, map, size);
    }

    return ret_value;
}

void free_map(hashmap *map) {
    hashmap_entry *it;
    for (int i = 0; i < map->size; i++) {
        while (map->entries[i]) {
            it = map->entries[i];
            map->entries[i] = it->next;
            free(it->symbol);
            free(it->mapping);
            free(it);
        }
    }
    free(map->entries);
    free(map);
}

int check_if_undefine(char* buffer, hashmap* map, int size) {
    char *define_keyword = "#undef";
    char *symbol;

    const char separator[2] = " ";
    char *auxiliary = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(auxiliary, buffer);
    char *tok = strtok(auxiliary, separator);

    if (tok != NULL && strcmp(tok, define_keyword) == 0) {
        tok = strtok(NULL, separator);
        if (tok != NULL) {
            symbol = malloc((strlen(tok) + 1) * sizeof(char));
            strcpy(symbol, tok);
            symbol[strlen(tok) - 1] = '\0';
        }

        if (symbol) {
            delete_hashmap_entry(symbol, map, size);
            free(symbol);
        }
        free(auxiliary);
        return 1;
    }
    free(auxiliary);
    return 0;
}

int check_if_define(char* buffer, hashmap* map, int size) {
    char *define_keyword = "#define";
    char *symbol;
    char *mapping;

    const char separator[2] = " ";
    char *auxiliary = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(auxiliary, buffer);
    char *tok = strtok(auxiliary, separator);

    if (tok != NULL && strcmp(tok, define_keyword) == 0) {
        tok = strtok(NULL, separator);
        if (tok != NULL) {
            symbol = malloc((strlen(tok) + 1) * sizeof(char));
            strcpy(symbol, tok);
            symbol[strlen(tok)] = '\0';
        }
        tok = strtok(NULL, separator);
        tok[strlen(tok) - 1] = '\0';
        if (tok != NULL) {
            mapping = malloc((strlen(tok) + 1) * sizeof(char));
            strcpy(mapping, tok);
            mapping[strlen(tok)] = '\0';
        }

        if (symbol && mapping) {
            add_hashmap_entry(symbol, mapping, map, size);
        }
        free(auxiliary);
        return 1;
    }
    free(auxiliary);
    return 0;
}

char* find_mapping(hashmap *map, char *symbol) {
    int hashcode = hash(symbol);
    hashmap_entry *it;

    if (map->entries[hashcode % map->size]) {
        it = map->entries[hashcode % map->size];
        while (it) {
            if (strcmp(it->symbol, symbol) == 0)
                return it->mapping;
            it = it->next;
        }
    }
    return NULL;
}

void resolve_defines(char *buffer, hashmap *map) {
    const char separators[8] = " )(;,{}";
    char *auxiliary = malloc((strlen(buffer) + 1) * sizeof(char));
    char symbol[200], *mapping;
    int buffer_length = 0;

    strcpy(auxiliary, buffer);
    strcpy(buffer, "");
    buffer[0] = '\0';

    for (int i = 0; i < strlen(auxiliary);) {
        if (strchr(separators, auxiliary[i]) || auxiliary[i] == '\n') {
            strncat(buffer, auxiliary + i, 1);
            i++;
            buffer_length++;
        } else {
            // reset symbol
            strcpy(symbol, "");
            symbol[0] = '\0';
            while (!strchr(separators, auxiliary[i]) || auxiliary[i] == '\n') {
                strncat(symbol, auxiliary + i, 1);
                i++;
            }
            mapping = find_mapping(map, symbol);
            if (mapping) {
                strcat(buffer, mapping);
                buffer_length += strlen(mapping);        
            } else {
                strcat(buffer, symbol);
                buffer_length += strlen(symbol);
            }
        }
    }
    free(auxiliary);
}

int take_input(int stdin_aux, int stdout_aux, char* directory,
    char* outfile, char* infile, hashmap* map, int size) {
    FILE *file_read = NULL;
    FILE *file_write = NULL;
        
    if (stdin_aux == 0)
        file_read = fopen(infile, "r");
    else
        file_read = stdin;
    
    if (file_read == NULL) {
        return -1;
    }

    if (stdout_aux == 0)
        file_write = fopen(outfile, "w");
    else
        file_write = stdout;

    if (file_write == NULL) {
        return -1;
    }

    int buffer_size = 200;
    char buffer[buffer_size];
    while (fgets(buffer, buffer_size, file_read)) {
        if (check_if_define(buffer, map, size)) {
        } else if (check_if_undefine(buffer, map, size)) {
        } else {
            resolve_defines(buffer, map);
            fprintf(file_write, "%s", buffer);
        }
    }

    if (stdin_aux == 0 && file_read)
        fclose(file_read);
    if (stdout_aux == 0 && file_write)
        fclose(file_write);
    return 0;
}

int main(int argc, char **argv) {
    
    int hashmap_size = 100;
    hashmap *map = create_map(hashmap_size);

    int stdin_aux = 1;
    int stdout_aux = 1;

    int not_option = 0;

    char *directory = NULL;
    char *outfile = NULL;
    char *infile = NULL;

    for (int i = 1; i < argc;) {
        if (argv[i][0] == '-' && argv[i][1] == 'D') {
            if(add_to_hashmap(argv, map, i, hashmap_size) == 1)
                i += 2;
            else
                i += 1;
        } else if (argv[i][0] == '-' && argv[i][1] == 'I') {
            if (strlen(argv[i]) <= 2) {
                directory = malloc((strlen(argv[i + 1]) + 1) * sizeof(char));
                strcpy(directory, argv[i + 1]);
                directory[strlen(argv[i + 1])] = '\0';
                i += 2;
            } else {
                directory = malloc((strlen(argv[i]) - 1) * sizeof(char));
                strcpy(directory, argv[i] + 2);
                directory[strlen(argv[i]) - 2] = '\0';
                i += 1;
            }
        } else if (argv[i][0] == '-' && argv[i][1] == 'o') {
            if (strlen(argv[i]) <= 2) {
                outfile = malloc((strlen(argv[i + 1]) + 1) * sizeof(char));
                strcpy(outfile, argv[i + 1]);
                outfile[strlen(argv[i + 1])] = '\0';
                i += 2;
            } else {
                outfile = malloc((strlen(argv[i]) - 1) * sizeof(char));
                strcpy(outfile, argv[i] + 2);
                outfile[strlen(argv[i]) - 2] = '\0';
                i += 1;
            }
            stdout_aux = 0;
        } else if (stdin_aux == 1) {
            infile = malloc((strlen(argv[i]) + 1) * sizeof(char));
            strcpy(infile, argv[i]);
            infile[strlen(argv[i])] = '\0';
            stdin_aux = 0;
            not_option++;
            i += 1;
        } else {
            not_option++;
            if (argc - 1 == i && stdout_aux == 1) {
                outfile = malloc((strlen(argv[i]) + 1) * sizeof(char));
                strcpy(outfile, argv[i]);
                outfile[strlen(argv[i])] = '\0';
                stdout_aux = 0;
            }
            i += 1;
        }
    }

    if (not_option > 2)
        return -1;

    int res = take_input(stdin_aux, stdout_aux, directory,
        outfile, infile, map, hashmap_size);
    if (res == -1)
        return res;

    // print_table(map);

    free_map(map);
    if (directory)
        free(directory);
    if (outfile)
        free(outfile);
    if (infile)
        free(infile);

    return 0;
}