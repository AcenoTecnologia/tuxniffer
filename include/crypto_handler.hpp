////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/evp.h>
#include <sstream> 

using namespace std;

/**
 * @class CryptoHandler
 * @brief Decrypt captured packets and extract keys from transport keys packets.
 */
class CryptoHandler {
private:    

    /**
     * @brief Encrypt Block with Advanced Encryption Standard AES-128 as described in annex B.1.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param input Byte vector with the message to be encrpyted.
     * @param output Pointer to byte vector which the encrpyted message will be written.
     * @param key Byte vector with the 128 bits key.
     */
    static void encryptBlock(const vector<uint8_t> input, vector<uint8_t>& output , vector<uint8_t> key);

    /**
     * @brief Pad message for Matyas-Meyer-Oseas hash function accordingly to annex B.4 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param message Byte vector with the message to be padded.
     * @param blockSize Block size for for Matyas-Meyer-Oseas hash function. Default = 16.
     * @return Byte vector with the padded message.
     */
    static vector<uint8_t> padMessageHash(const std::vector<uint8_t> message, const size_t blockSize = 16);

    /**
     * @brief Matyas-Meyer-Oseas hash function implemented accordingly to annex B.4 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param message Byte vector with the message to be hashed.
     * @param blockSize Block size for for Matyas-Meyer-Oseas hash function. Default = 16.
     * @return Byte vector with hash.
     */
    static vector<uint8_t> matyasMeyerOseas(const std::vector<uint8_t> message, const size_t blockSize = 16);

    /**
     * @brief HMAC implemented accordingly to https://nvlpubs.nist.gov/nistpubs/fips/nist.fips.198-1.pdf with the instantiations especified in annex B.4 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param key Byte vector with the key.
     * @param message Byte vector with the message.
     * @param blockSize Block size for for HMAC. Default = 16.
     * @return Byte vector with MAC.
     */
    static vector<uint8_t> hmac(const vector<uint8_t> key, const vector<uint8_t> message, const size_t blockSize = 16);

    /**
     * @brief Create length string from additional data for authentication accordingly to annex A.2.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param length Size of additional data string.
     * @return Byte vector with length string.
     */
    static vector<uint8_t> formLengthString(size_t length);

    /**
     * @brief right-concatene the input with the smallest non-negative number of all-zero octets such that the input has length divisible by 16.
     * 
     * @param input Byte vector with the input string to be padded.
     * @param blockSize Block size for for HMAC. Default = 16.
     * @return Byte vector with the padded input.
     */
    static vector<uint8_t> padToBlockSize(const vector<uint8_t> input, size_t blockSize);

    /**
     * @brief Create authentication tag accordingly to annex A.2.2 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param key Byte vector with the key.
     * @param plaintext Byte vector with the message that will be encrypted.
     * @param additionalData Byte vector with the additional data for authentication.
     * @param nonce Byte vector with the nonce formed accordingly to 4.5.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * @param M Size of authentication field. Can be 0, 4, 6, 8, 10, 12, 14, and 16.
     * @param blockSize Block size for for HMAC. Default = 16.
     * @return Byte vector with length string.
     */
    static vector<uint8_t> authentication(const vector<uint8_t> key, const vector<uint8_t> plaintext, const vector<uint8_t> additionalData, 
                 const vector<uint8_t> nonce, int M, const size_t blockSize = 16);

    /**
     * @brief Encrypt a message and create the authentication tag as especified on annex A.2 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param key Byte vector with the key.
     * @param plaintext Byte vector with the message that will be encrypted.
     * @param additionalData Byte vector with the additional data for authentication.
     * @param nonce Byte vector with the nonce formed accordingly to 4.5.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * @param M Size of authentication field. Can be 0, 4, 6, 8, 10, 12, 14, and 16.
     * @param cyphertext Pointer to byte vector where the resulting cyphertext will be saved.
     * @param authTag Pointer to byte vector where the resulting authentication tag will be saved.
     * @param blockSize Block size for for HMAC. Default = 16.
     */
    static void encrypt(const vector<uint8_t> key, const vector<uint8_t> plaintext, const vector<uint8_t> additionalData, const vector<uint8_t> nonce,
                 int M, vector<uint8_t>& ciphertext, vector<uint8_t>& authTag, const size_t blockSize = 16);

    /**
     * @brief Decrypt a message and validate the result with the authentication tag as especified on annex A.3 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param key Byte vector with the key.
     * @param cyphertext Byte vector with the message that will be decrypted.
     * @param additionalData Byte vector with the additional data for authentication.
     * @param nonce Byte vector with the nonce formed accordingly to 4.5.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * @param authTag Byte vector with the authentication.
     * @param M Size of authentication field. Can be 0, 4, 6, 8, 10, 12, 14, and 16.
     * @param plaintext Pointer to byte vector where the resulting plaintext will be saved.
     * @param blockSize Block size for for HMAC. Default = 16.
     * @return Boolean value with the validation result.
     */
    static bool decrypt(const vector<uint8_t> key, const vector<uint8_t> ciphertext, const vector<uint8_t> additionalData, const vector<uint8_t> nonce, 
                 const vector<uint8_t> authTag, int M, vector<uint8_t>& plaintext, const size_t blockSize = 16);

    /**
     * @brief Try to decrypt the payload from a zigbee layer.
     * 
     * @param header Byte vector with the layer header.
     * @param payload byte vector with the layer payload (including security header).
     * @param plaintext Pointer to byte vector where the decrypted payload will be saved.
     * @param isNwkLayer Bool value idicating the layer where the security header was extracted.
     * Zigbee Network Layer if true and Zigbee Application Support Layer if false.    
     * @return Bool value indicating if the payload and the necessary data for decryption was successful extracted.
     */
    bool handle_decryption(vector<uint8_t> header, vector<uint8_t> payload, vector<uint8_t>& plaintext, bool isNwkLayer);

public:

    vector<vector<uint8_t>> link_keys; //List of decyphered link keys from transport key packets. Initialized with zigbee's standard link key.

    vector<vector<uint8_t>> nwk_keys; //List of decyphered network keys from transport key packets.

    vector<vector<uint8_t>> transportPackets; //List of captured transport key packets.

    int security_level; //Int indicating the security level. Can be 5~7 (0-4 not supported). If value is -1 levels 5~7 will be tried.
   
    /**
     * @brief Constructor for the Crpyto class. Initialize keys and transportPackets vectors and add the zigbee's standard link key to keys.
     */
    CryptoHandler();

    /**
     * @brief Try to decrypt tranport keys messages with the known keys. Save the packet and the new key if successful.
     * 
     * @param payload Byte vector with Mac Layer payload data.
     * @return Boolean value with the validation result.
     */
    bool extract_key(std::vector<uint8_t> payload); 

    /**
     * @brief Tranforms a byte vector in a hex string.
     * 
     * @param bytes Byte vector.
     * @return Hex string.
     */
    static string bytesToHexString(const std::vector<uint8_t>& bytes);
};