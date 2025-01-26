#include "utils.h"

char *join_path_(char *path, unsigned int count, va_list va_a){
    if (path == NULL){
        return NULL;
    }

    size_t path_len = strlen(path);
    size_t curr_len = 0;
    size_t new_path_len = path_len;
    size_t curr_new_path_len = 0;
    unsigned int add_dir_sep = 0;
    unsigned int i = 0;
    const char *curr;
    va_list va;
    va_list vb;
    va_copy(va, va_a);

    if (path[path_len - 1] != '/'){
        add_dir_sep |= (1 << 0);
        new_path_len++;
    }

    for (i = 0; i < count; i++){
        curr = va_arg(va, char *);
        curr_len = strlen(curr);

        if (curr[curr_len - 1] != '/' && i != count - 1){
            new_path_len += (curr_len + 1);
            add_dir_sep |= (1 << (i + 1));
        }else{
            new_path_len += curr_len;
        }
    };

    va_end(va);

    char *new_path = malloc(sizeof(char) * (new_path_len + 1));
    if (new_path == NULL){
        return NULL;
    }
    new_path[0] = '\0';

    strcat(new_path + curr_new_path_len, path);
    curr_new_path_len += path_len;
    if (add_dir_sep & (1 << 0)){
        strcat(new_path + curr_new_path_len, "/");
        curr_new_path_len++;
    }

    va_copy(vb, va_a);
    for (i = 0; i < count; i++){
       curr = va_arg(vb, char*);
       strcat(new_path + curr_new_path_len, curr);
       curr_new_path_len += strlen(curr);

       if (add_dir_sep & (1 << (i + 1))){
           strcat(new_path + curr_new_path_len, "/");
           curr_new_path_len++;
       }
    }

    va_end(vb);

    return new_path;
}

char *join_path(char *path, unsigned int count, ...){
    va_list va;
    va_start(va, count);
    char *res = join_path_(path, count, va);
    va_end(va);
    return res;
}

int make_directories(char *path) {
    char temp[1024];
    size_t len = strlen(path);
    struct stat st;

    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    for (size_t i = 1; i <= len; i++) {
        if (temp[i] == '/' || temp[i] == '\0') {
            char saved_char = temp[i];
            temp[i] = '\0';

            if (stat(temp, &st) != 0) {
                if (mkdir(temp, 0755) != 0) {
                    perror("mkdir");
                    return -1;
                }
            } else if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "Error: %s exists and is not a directory\n", temp);
                return -1;
            }

            temp[i] = saved_char;
        }
    }

    return 1;
}

bool is_dir(char *path){
    struct stat sb;

    return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
}

int is_dir_empty(char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        count++;
        break;
    }

    closedir(dir);
    return count == 0;
}

int count_digits(int num) {
    if (num == 0){
        return 1;
    }
    int count = 0;
    if (num < 0){
        num = -num;
    }

    while (num != 0) {
        num /= 10;
        count++;
    }
    return count;
}
