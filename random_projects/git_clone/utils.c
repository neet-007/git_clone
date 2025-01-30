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

    if (strcmp(path, "") != 0 && path[path_len - 1] != '/'){
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

bool is_dir(char *path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, "not a dir %s\n", path);
        perror("stat failed");
        return false;
    }
    return S_ISDIR(sb.st_mode);
}

bool is_file(char *path){
    struct stat _st;
    return ((path) != NULL && stat((path), &_st) == 0 && S_ISREG(_st.st_mode));
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

char* compress_zlib(char *input, size_t input_len, size_t *output_size) {
    z_stream strm = {0};
    deflateInit(&strm, Z_BEST_COMPRESSION);

    size_t max_compressed_size = deflateBound(&strm, input_len);

    char *compressed_data = (char*)malloc(max_compressed_size);
    if (!compressed_data) {
        perror("Memory allocation failed");
        deflateEnd(&strm);
        return NULL;
    }

    strm.avail_in = input_len;
    strm.next_in = (unsigned char *)input;
    strm.avail_out = max_compressed_size;
    strm.next_out = (unsigned char *)compressed_data;

    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        perror("Compression failed");
        free(compressed_data);
        deflateEnd(&strm);
        return NULL;
    }

    *output_size = max_compressed_size - strm.avail_out;

    deflateEnd(&strm);
    return compressed_data;
}

unsigned char *decompress_zlib_from_data(const unsigned char *src, size_t src_len, size_t *dst_len) {
    z_stream strm = {0};
    strm.next_in = (unsigned char *)src;
    strm.avail_in = src_len;

    size_t buffer_size = 1024;
    unsigned char *dst = malloc(buffer_size);
    if (!dst) {
        perror("Failed to allocate memory");
        return NULL;
    }

    if (inflateInit(&strm) != Z_OK) {
        fprintf(stderr, "Failed to initialize decompression\n");
        free(dst);
        return NULL;
    }

    int ret;
    size_t total_out = 0;

    do {
        if (strm.total_out >= buffer_size) {
            buffer_size *= 2;
            unsigned char *new_dst = realloc(dst, buffer_size);
            if (!new_dst) {
                perror("Failed to reallocate memory");
                inflateEnd(&strm);
                free(dst);
                return NULL;
            }
            dst = new_dst;
        }

        strm.next_out = dst + strm.total_out;
        strm.avail_out = buffer_size - strm.total_out;

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            fprintf(stderr, "Decompression failed: %d\n", ret);
            inflateEnd(&strm);
            free(dst);
            return NULL;
        }
    } while (ret != Z_STREAM_END);

    total_out = strm.total_out;
    inflateEnd(&strm);

    unsigned char *final_dst = realloc(dst, total_out);
    if (!final_dst) {
        perror("Failed to reallocate memory to final size");
        free(dst);
        return NULL;
    }

    *dst_len = total_out;
    return final_dst;
}

unsigned char *decompress_zlib_from_file(FILE *file, size_t *dst_len) {
    z_stream strm = {0};
    if (inflateInit(&strm) != Z_OK) {
        fprintf(stderr, "Failed to initialize decompression\n");
        return NULL;
    }

    size_t buffer_size = 1024;
    unsigned char *dst = malloc(buffer_size);
    if (!dst) {
        perror("Failed to allocate memory");
        inflateEnd(&strm);
        return NULL;
    }

    unsigned char in[1024];
    int ret;
    size_t total_out = 0;

    do {
        strm.avail_in = fread(in, 1, sizeof(in), file);
        if (ferror(file)) {
            perror("Failed to read file");
            free(dst);
            inflateEnd(&strm);
            return NULL;
        }

        if (strm.avail_in == 0)
            break;

        strm.next_in = in;

        do {
            if (strm.total_out >= buffer_size) {
                buffer_size *= 2; // Double the buffer size
                unsigned char *new_dst = realloc(dst, buffer_size);
                if (!new_dst) {
                    perror("Failed to reallocate memory");
                    free(dst);
                    inflateEnd(&strm);
                    return NULL;
                }
                dst = new_dst;
            }

            strm.next_out = dst + strm.total_out;
            strm.avail_out = buffer_size - strm.total_out;

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                fprintf(stderr, "Decompression failed: %d\n", ret);
                free(dst);
                inflateEnd(&strm);
                return NULL;
            }
        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    total_out = strm.total_out;
    inflateEnd(&strm);

    unsigned char *final_dst = realloc(dst, total_out);
    if (!final_dst) {
        perror("Failed to reallocate memory to final size");
        free(dst);
        return NULL;
    }

    *dst_len = total_out;
    return final_dst;
}

void print_raw_data_as_chars(char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
            printf("%c", data[i]);
    }
}

void sha1_hexdigest(unsigned char *data, size_t len, char *output) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(data, len, hash);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA_DIGEST_LENGTH * 2] = '\0';
}

char *file_read_all(FILE *f, size_t *data_size){
    *data_size = 0;

    while(fgetc(f) != EOF){
        (*data_size)++;
    }

    fseek(f, -((long)(*data_size)), SEEK_END);

    char *data = malloc(sizeof(char) * (*data_size));
    if (data == NULL){
        fprintf(stderr, "unable to allocate memory for data in file_read_all\n");
        return NULL;
    }

    size_t curr = 0;
    while(curr < *data_size){
        data[curr++] = fgetc(f);
    }

    return data;
}

int find_char(char *raw, char c, int start, size_t raw_size) {
    if (!raw || start < 0 || start >= raw_size) {
        return -1;
    }

    size_t pos = start;
    bool found = false;
    while (pos < raw_size){
        if (raw[pos] == c){
            found = true;
            break;
        }
        pos++;
    }

    return found ? pos : -1;
}
