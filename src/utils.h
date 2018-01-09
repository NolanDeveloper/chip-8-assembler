extern void *emalloc(size_t size);
extern char *loadFile(const char *filepath);
extern void die_(const char *source, const char *format, ...);
extern uintmax_t stringHash(const char *str);
