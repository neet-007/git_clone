#include "git_index.h"

void read_big_endian_20_bytes(unsigned char *bytes, char *result) {
    uint64_t part1 = 0;
    uint64_t part2 = 0;
    uint32_t part3 = 0;

    for (int i = 0; i < 8; i++) {
        part1 = (part1 << 8) | bytes[i];
    }

    for (int i = 8; i < 16; i++) {
        part2 = (part2 << 8) | bytes[i];
    }

    for (int i = 16; i < 20; i++) {
        part3 = (part3 << 8) | bytes[i];
    }

    sprintf(result, "%016llx%016llx%08x", part1, part2, part3);
}

GitIndex *read_index(GitRepository *repo){
#define BIG_ENDIAN_INT_4_BYTES(index, bytes, res) \
    (res) = (((bytes)[(index)] << 24) | \
             ((bytes)[(index) + 1] << 16) | \
             ((bytes)[(index) + 2] << 8) | \
             ((bytes)[(index) + 3]))

#define BIG_ENDIAN_INT_2_BYTES(index, bytes, res) \
    (res) = (((bytes)[(index)] << 8) | \
             ((bytes)[(index) + 1]))

    char *index_file = repo_file(repo, false, 1, "index");
    if (index_file == NULL){
        fprintf(stderr, "unable to find index file in read_index\n");
        return NULL;
    }

    if (!PATH_EXISTS(index_file)){
        fprintf(stderr, "index file does not exist in read_index %s\n", index_file);
        free(index_file);
        return NULL;
    }

    FILE *f = fopen(index_file, "rb");
    if (f == NULL){
        fprintf(stderr, "unable to opne index file in read_index %s\n", index_file);
        free(index_file);
        return NULL;
    }

    size_t i = 0;
    size_t j = 0;
    char header[12];
    fread(header, sizeof(char), 4, f);
    i += 4;
    header[i] = '\0';
    if (strcmp(header, "DIRC") != 0){
        fprintf(stderr, "DIRC was not found in header in read_index %s\n", index_file);
        free(index_file);
        fclose(f);
        return NULL;
    }

    fread(header + i, sizeof(char), 4, f);
    int version = 2;
    BIG_ENDIAN_INT_4_BYTES(i, header, version);
    i += 4;

    fread(header + i, sizeof(char), 4, f);
    size_t entries_count = 2;
    BIG_ENDIAN_INT_4_BYTES(i, header, entries_count);
    i += 4;

    size_t curr_entry = 0;
    unsigned char buf_20_byte[20];
    unsigned char buf_4_byte[4];
    unsigned char buf_2_byte[2];
    int unused = -1;

    int ctime_sec;
    int ctime_nsec;
    int mtime_sec;
    int mtime_nsec;
    int dev;
    int ino;
    int mode;
    int mode_type;
    int mode_perms;
    int uid;
    int gid;
    int fsize;
    unsigned char sha[GIT_SHA_SIZE];
    int flags;
    int flag_assume_valid;
    int flag_extended;
    int flag_stage;

    size_t name_size = 0;
    size_t name_length = 0;
    char *name = NULL;

    GitIndexEntry *entry = NULL;
    GitIndexEntry **entries = malloc(sizeof(GitIndexEntry *) * entries_count);
    if (entries == NULL){
        fprintf(stderr, "uanble to allocate memory for entries in read_index %s\n", index_file);
        free(index_file);
        fclose(f);
        return NULL;
    }

    i = 0;
    size_t new_i;
    for (curr_entry = 0; curr_entry < entries_count; curr_entry++){
        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, ctime_sec);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, ctime_nsec);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, mtime_sec);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, mtime_nsec);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, dev);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, ino);
        i += 4;

        fread(buf_2_byte, sizeof(char), 2, f);
        BIG_ENDIAN_INT_2_BYTES(0, buf_2_byte, unused);
        i += 2;
        if (unused != 0){
            fprintf(stderr, "unused must be 0 not %d in read_index %s\n", unused, index_file);
            free(index_file);
            free(entries);
            fclose(f);
            return NULL;
        }

        fread(buf_2_byte, sizeof(char), 2, f);
        BIG_ENDIAN_INT_2_BYTES(0, buf_2_byte, mode);
        i += 2;
        mode_type = mode >> 12;
        if (mode_type != 0b1000 && mode_type != 0b1010 && mode_type != 0b1110){
            fprintf(stderr, "wrong mode type %d in read_index %s\n", mode_type, index_file);
            free(index_file);
            free(entries);
            fclose(f);
            return NULL;
        }
        mode_perms = mode & 0b0000000111111111;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, uid);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, gid);
        i += 4;

        fread(buf_4_byte, sizeof(char), 4, f);
        BIG_ENDIAN_INT_4_BYTES(0, buf_4_byte, fsize);
        i += 4;

        fread(buf_20_byte, sizeof(char), 20, f);
        read_big_endian_20_bytes(buf_20_byte, sha);
        sha[GIT_SHA_SIZE - 1] = '\0';
        i += 20;

        fread(buf_2_byte, sizeof(char), 2, f);
        BIG_ENDIAN_INT_2_BYTES(0, buf_2_byte, flags);
        i += 2;
        flag_assume_valid = (flags & 0b1000000000000000) != 0;
        flag_extended = (flags & 0b0100000000000000) != 0;
        if (flag_extended){
            fprintf(stderr, "git_clone does not support flag extenstion in read_index %s\n", index_file);
            free(index_file);
            free(entries);
            fclose(f);
            return NULL;
        }
        flag_stage = flags & 0b0011000000000000;

        name_length = flags & 0b0000111111111111;

        if (name_length < 0xFFF){
            if (name_length >= name_size){
                char *temp= realloc(name, sizeof(char) * ((name_length * 2) + 1));
                if (temp== NULL){
                    fprintf(stderr, "unable to allocate memory for name in read_index %s\n", index_file);
                    free(index_file);
                    free(entries);
                    free(name);
                    fclose(f);
                    return NULL;
                }
                name = temp;
            }
            fread(name, sizeof(char), name_length, f);
            name[name_length] = '\0';
            i += (name_length + 1);
            fseek(f, 1, SEEK_CUR);
        }else{
            //TODO: make the other case
            fprintf(stderr, "unimplaented in read_index %s\n", index_file);
            free(index_file);
            free(entries);
            free(name);
            fclose(f);
            return NULL;
        }

        new_i = (i + 7) & ~7;
        fseek(f, new_i - i, SEEK_CUR);
        i = new_i;

        entry = malloc(sizeof(GitIndexEntry));
        if (entry == NULL){
            fprintf(stderr, "unable to allocate memory for entry in read_index %s\n", index_file);
            free(index_file);
            free(entries);
            free(name);
            fclose(f);
            return NULL;
        }
        entry->ctime_sec = ctime_sec;
        entry->ctime_nsec = ctime_nsec;
        entry->mtime_sec = mtime_sec;
        entry->mtime_nsec = mtime_nsec;
        entry->dev = dev;
        entry->ino = ino;
        entry->uid = uid;
        entry->gid = gid;
        entry->mode = mode;
        entry->mode_type = mode_type;
        entry->mode_perms = mode_perms;
        entry->flags = flags;
        entry->flag_assume_valid = flag_assume_valid;
        entry->flag_extended = flag_extended;
        entry->flag_stage = flag_stage;
        entry->fsize = fsize;
        entry->name = strdup(name);
        memcpy(entry->sha, sha, sizeof(char) * (GIT_SHA_SIZE));

        entries[curr_entry] = entry;
    }

    GitIndex *index = malloc(sizeof(GitIndex));
    if (index == NULL){
        fprintf(stderr, "unable to allocate memory for index in read_index %s\n", index_file);
        free(index_file);
        free(entries);
        free(name);
        fclose(f);
        return NULL;
    }
    index->entries = entries;
    index->entries_count = entries_count;
    index->version = version;


    free(index_file);
    free(name);
    fclose(f);

    return index;

#undef BIG_ENDIAN_INT_4_BYTES
#undef BIG_ENDIAN_INT_2_BYTES
}

//TODO: a way like this migth be more c like
void read_git_index(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Error getting file size");
        close(fd);
        return;
    }

    size_t file_size = st.st_size;
    void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return;
    }

    GitIndex *index = (GitIndex *)data;

    // Validate the index signature
    if (memcmp(&index->signature, "DIRC", 4) != 0) {
        printf("Invalid Git index signature.\n");
        munmap(data, file_size);
        close(fd);
        return;
    }

    printf("Git Index Version: %u\n", index->version);
    printf("Number of Entries: %u\n", index->entries_count);

    // Read each entry
    void *entry_ptr = (uint8_t *)index + sizeof(GitIndex);
    for (uint32_t i = 0; i < index->entries_count; i++) {
        GitIndexEntry *entry = (GitIndexEntry *)entry_ptr;

        printf("Entry %u:\n", i + 1);
        printf("  ctime: %u.%u\n", entry->ctime_sec, entry->ctime_nsec);
        printf("  mtime: %u.%u\n", entry->mtime_sec, entry->mtime_nsec);
        printf("  dev: %u ino: %u\n", entry->dev, entry->ino);
        printf("  mode: %o uid: %u gid: %u\n", entry->mode, entry->uid, entry->gid);
        printf("  file size: %u\n", entry->fsize);
        printf("  SHA: ");
        for (int j = 0; j < GIT_SHA_SIZE; j++) {
            printf("%02x", entry->sha[j]);
        }
        printf("\n");

        printf("  flags: 0x%04x\n", entry->flags);
        printf("  name: %s\n", entry->name);

        // Advance pointer by size of entry and variable-length name (rounded up to 8 bytes)
        size_t entry_size = sizeof(GitIndexEntry) + strlen(entry->name) + 1;
        entry_size = (entry_size + 7) & ~7;  // Align to 8 bytes
        entry_ptr = (uint8_t *)entry_ptr + entry_size;
    }

    munmap(data, file_size);
    close(fd);
}
