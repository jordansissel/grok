
void substr_replace(char **strp, int *strp_len, int *strp_alloc_size,
                    const int start, const int end, 
                    const char *replace, const int replace_len);

#define ESCAPE_LIKE_C 0x0001
#define ESCAPE_UNICODE 0x0002
#define ESCAPE_HEX 0x0004
#define ESCAPE_NONPRINTABLE 0x0008

void string_escape(char **strp, int *strp_len, int *strp_alloc_size,
                   const char *chars, int chars_len, int options);
void string_unescape(char **strp, int *strp_len, int *strp_size);

/* libc doesn't often have strndup, so let's make our own */
char *string_ndup(const char *src, size_t size);

