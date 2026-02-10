#pragma once

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Computes the SHA256 hash of the input name and returns it as a base64 encoded string.
 *        The hash is truncated to 6 bytes before encoding.
 *
 * @param name The input string to hash.
 * @return std::string The base64 encoded hash.
 */
std::string hashName_std(const char* name);

/**
 * @brief Encodes binary data into a base64 string.
 *
 * @param in Pointer to the input data.
 * @param len Length of the input data.
 * @return std::string The base64 encoded string.
 */
std::string base64encode_std(const uint8_t* in, size_t len);
