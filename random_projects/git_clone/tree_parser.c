#include "tree_parser.h"

// start=0
GitTreeLeaf *tree_parse_leaf(char *raw, size_t raw_size, size_t *start){
    GitTreeLeaf *leaf = malloc(sizeof(GitTreeLeaf));
    if (leaf == NULL){
        fprintf(stderr, "unable to allocate memory for git tree leaf in tree_parse_leaf\n");
        return NULL;
    }

    int x = find_char(raw, ' ', *start, raw_size);
    if (x - *start != 5 && x - *start != 6){
        fprintf(stderr, "mode size must be 5 or 6 not %ld\n", x - *start);
        free(leaf);
        return NULL;
    }
    size_t i = 0;
    if (x - *start == 5){
        leaf->mode[i] = '0';
        while (*start + i < x){
            leaf->mode[i + 1] = raw[*start + i];
            i++;
        }
    }else{
        while (*start + i < x){
            leaf->mode[i] = raw[*start + i];
            i++;
        }
    }

    int y = find_char(raw, '\0', x, raw_size);
    if (y < 0){
        fprintf(stderr, "unable to find null byte for path in tree_parse_leaf\n");
        free(leaf);
        return NULL;
    }

    leaf->path = malloc(sizeof(char) * (y - x));
    if (leaf->path == NULL){
        fprintf(stderr, "unable to allocate memory for path in tree_parse_leaf\n");
        free(leaf);
        return NULL;
    }

    memcpy(leaf->path, raw + (x + 1), sizeof(char) * y - x - 1);
    leaf->path[y - x - 1] = '\0';

    if (raw_size - y < 21){
        fprintf(stderr, "invalid size of data to read sha need 21 got %ld in tree_parse_leaf\n", raw_size - y);
        free(leaf);
        return NULL;
    }

    for (int i = 0; i < 20; i++) {
        snprintf(leaf->sha + i * 2, 3, "%02x", (unsigned char)raw[y + 1 + i]);
    }

    leaf->sha[40] = '\0';

    *start = y + 21;
    return leaf;
}

//WARNING: this is placeholder until i fix dynamic array
GitTreeLeaf **tree_parse(char *raw, size_t raw_size, size_t *leafs_len, size_t *leafs_size){
    size_t start = 0;
    *leafs_len = 0;
    *leafs_size = 8;
    GitTreeLeaf **leafs = malloc(sizeof(GitTreeLeaf *) * (*leafs_size));
    GitTreeLeaf *curr = NULL;
    start = 0;
    while(start < raw_size){
        curr = tree_parse_leaf(raw, raw_size, &start);
        if (curr == NULL){
            fprintf(stderr, "unable to parse a leaf in tree_parse\n");
            free(leafs);
            return NULL;
        }
        if (*leafs_len >= *leafs_size){
            (*leafs_size) *= 2;
            GitTreeLeaf **temp = realloc(leafs, sizeof(GitTreeLeaf *) * (*leafs_size));
            if (temp == NULL){
                fprintf(stderr, "unable allocate memory for leafs in tree_parse\n");
                free(curr);
                free(leafs);
                return NULL;
            }
            leafs = temp;
        }
        leafs[(*leafs_len)++] = curr;
    }

    return leafs;
}

char *tree_serialize(GitTree *tree, size_t *data_size){
    size_t i = 0;
    GitTreeLeaf *curr = NULL;
    //FIXME: sort this by path
    while (i < tree->items_len){
        if (tree->items[i]->mode[0] != '1' && tree->items[i]->mode[1] != '0'){
            tree->items[i]->path = realloc(tree->items[i]->path, sizeof(char) * (strlen(tree->items[i]->path) + 2));
            if (tree->items[i]->path == NULL){
                fprintf(stderr, "unable to allocate memory for path in tree_serialize\n");
                return NULL;
            }
            tree->items[i]->path[strlen(tree->items[i]->path)-1] = '/';
            tree->items[i]->path[strlen(tree->items[i]->path)] = '\0';
        }
    }

    *data_size = (GIT_TREE_MODE_SIZE + 1 + 1 + GIT_SHA_SIZE) * tree->items_len;
    char *ret = malloc(sizeof(char) * (*data_size));
    if (ret == NULL){
        fprintf(stderr, "unable to allocate memory for data in tree_serialize\n");
        return NULL;
    }

    i = 0;
    int y = 0;
    size_t curr_len = 0;
    unsigned char sha_bytes[20];
    while(i < tree->items_len){
        curr = tree->items[i];
        memcpy(ret + curr_len, curr->mode, sizeof(char) * GIT_TREE_MODE_SIZE);
        curr_len += GIT_TREE_MODE_SIZE;
        ret[curr_len] = ' ';
        memcpy(ret + curr_len, curr->path, sizeof(char) * strlen(curr->path));
        curr_len += strlen(curr->path);
        ret[curr_len] = '\0';
        for (y = 0; y < 20; y++) {
            sscanf(&curr->sha[y * 2], "%2hhx", &sha_bytes[y]);
        }
        memcpy(ret + curr_len, sha_bytes, 20);
        curr_len += 20;
        i++;
    }

    return ret;
}
