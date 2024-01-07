#ifndef BASE64_H
#define BASE64_H

#include <stdlib.h>

/**
 * Encodes a binary data array into a Base64 string.
 *
 * @param data The binary data to encode.
 * @param input_length The length of the binary data.
 * @param output_length Pointer to a size_t where the output length (Base64 string length) will be stored.
 * @return The encoded Base64 string. The caller is responsible for freeing this memory.
 */
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);

/**
 * Decodes a Base64 encoded string back into binary data.
 *
 * @param data The Base64 encoded string.
 * @param input_length The length of the Base64 string.
 * @param output_length Pointer to a size_t where the output length (binary data length) will be stored.
 * @return The decoded binary data. The caller is responsible for freeing this memory.
 */
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);

#endif // BASE64_H
