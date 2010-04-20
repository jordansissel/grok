/**
 * @file stringhelper.h
 */
#ifndef _STRINGHELPER_H_ 
#define  _STRINGHELPER_H_ 

/**
 * substring replacement.
 *
 * This function handles growing the string if the new length would be 
 * longer than the alloc size.
 *
 * @param strp pointer to the string you want to modify
 * @param strp_len pointer to current length of the string. If negative, I will
 *        use strlen() to calculate the length myself.
 * @param strp_alloc_size pointer to current allocated size of string
 * @param start start position of the replacement. 0 offset. If negative, it is an offset
 *        against the end of the string; -1 for end of string.
 * @param end end position of the replacement. If negative, it is an offset
 *        against the end of the string; -1 for end of string. If zero (0),
 *        then end is set to start.
 * @param replace string to replace with
 * @param replace_len length of the replacement string
 */
void substr_replace(char **strp, int *strp_len, int *strp_alloc_size,
                    const int start, const int end, 
                    const char *replace, const int replace_len);

#define ESCAPE_LIKE_C 0x0001
#define ESCAPE_UNICODE 0x0002
#define ESCAPE_HEX 0x0004
#define ESCAPE_NONPRINTABLE 0x0008

/**
 * Escape a string by specific options.
 *
 * This function will 
 * @param strp pointer to string to escape
 */
void string_escape(char **strp, int *strp_len, int *strp_alloc_size,
                   const char *chars, int chars_len, int options);
void string_unescape(char **strp, int *strp_len, int *strp_size);

int string_count(const char *src, const char *charlist);
int string_ncount(const char *src, size_t srclen,
                  const char *charlist, size_t listlen);

/* libc doesn't often have strndup, so let's make our own */
char *string_ndup(const char *src, size_t size);

#endif /* _STRINGHELPER_H_ */
