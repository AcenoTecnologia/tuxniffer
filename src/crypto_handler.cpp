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


#include "crypto_handler.hpp"
#include "common.hpp"
#include <string>
#include <iomanip>
#include "payload_handler.hpp"

string CryptoHandler::bytesToHexString(const std::vector<uint8_t>& bytes) {
    ostringstream oss;

    for (size_t i = 0; i < bytes.size(); ++i) {
        // Adiciona "0x" antes de cada byte, exceto o primeiro
        if (i > 0) {
            oss << " ";
        }
        oss << "0x" 
            << std::hex << std::uppercase << setw(2) << setfill('0') 
            << static_cast<int>(bytes[i]);
    }

    oss << " (";
    for (size_t i = 0; i < bytes.size(); ++i) {
        // Adiciona "0x" antes de cada byte, exceto o primeiro
        oss << std::hex << std::uppercase << setw(2) << setfill('0') 
            << static_cast<int>(bytes[i]);
    }
    oss << ")";

    return oss.str(); // Retorna a string construída
}

CryptoHandler::CryptoHandler()
{
    link_keys.push_back({0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39});
}


void CryptoHandler::encryptBlock(const vector<uint8_t> input, vector<uint8_t>& output , vector<uint8_t> key)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key.data(), nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int outlen;
    EVP_EncryptUpdate(ctx, output.data(), &outlen, input.data(), input.size());

    EVP_CIPHER_CTX_free(ctx);
}


vector<uint8_t> CryptoHandler::padMessageHash(const vector<uint8_t> message, const size_t blockSize)
{
    bool isLongMessage = (message.size() * 8) >= 65536;
    
    size_t l = message.size() * 8; // Comprimento da mensagem em bits
    int paddingSize;

    if (!isLongMessage) {
        paddingSize = (7 * blockSize - (l + 1)) % (8 * blockSize);
    } else {
        paddingSize = (5 * blockSize - (l + 1)) % (8 * blockSize);
    }

    if (paddingSize < 0) {
        paddingSize += 8 * blockSize;
    }

    vector<uint8_t> paddedMessage = message;
    paddedMessage.push_back(0x80); // Adiciona o bit '1'

    
    // Adiciona os bits '0' necessários
    size_t paddingBytes = (paddingSize - 7) / 8;
    paddedMessage.insert(paddedMessage.end(), paddingBytes, 0x00);


    // Adiciona o comprimento original em bits
    size_t lengthSize = isLongMessage ? 2 * blockSize / 8: blockSize / 8;
    for (size_t i = 0; i < lengthSize; ++i) {
        paddedMessage.push_back((l >> (8 * (lengthSize - 1 - i))) & 0xFF);
    }

    if (isLongMessage) {
        paddedMessage.insert(paddedMessage.end(), blockSize/8, 0x00); // Adiciona a string de n bits de zeros
    }

    return paddedMessage;
}

vector<uint8_t> CryptoHandler::matyasMeyerOseas(const vector<uint8_t> message, const size_t blockSize)
{
    
    vector<uint8_t> paddedMessage = padMessageHash(message, blockSize);

    // Parse a mensagem em blocos de n octetos
    size_t t = paddedMessage.size() / blockSize;
    vector<uint8_t> hash(blockSize, 0x00); // Inicializa com 0^8n

    for (size_t j = 0; j < t; ++j) {
        vector<uint8_t> block(paddedMessage.begin() + j * blockSize, paddedMessage.begin() + (j + 1) * blockSize);
        
        vector<uint8_t> encryptedBlock(blockSize, 0);

        encryptBlock(block, encryptedBlock, hash);

        //vector<uint8_t> encrypted = encryptAES(hash, block);
        for (size_t i = 0; i < blockSize; ++i) {
            hash[i] = encryptedBlock[i] ^ block[i];
        }
    }

    return hash;
}

vector<uint8_t> CryptoHandler::hmac(const vector<uint8_t> key, const vector<uint8_t> message, const size_t blockSize)
{
    vector<uint8_t> keyAdjusted = key;

    if (key.size() > blockSize) {
        keyAdjusted = matyasMeyerOseas(key);
    }
    keyAdjusted.resize(blockSize, 0);

    vector<uint8_t> o_key_pad(blockSize, 0x5C);
    vector<uint8_t> i_key_pad(blockSize, 0x36);

    for (size_t i = 0; i < blockSize; ++i) {
        o_key_pad[i] ^= keyAdjusted[i];
        i_key_pad[i] ^= keyAdjusted[i];
    }

    vector<uint8_t> innerHashInput = i_key_pad;
    innerHashInput.insert(innerHashInput.end(), message.begin(), message.end());

    vector<uint8_t> innerHash = matyasMeyerOseas(innerHashInput);

    vector<uint8_t> outerHashInput = o_key_pad;
    outerHashInput.insert(outerHashInput.end(), innerHash.begin(), innerHash.end());

    return matyasMeyerOseas(outerHashInput);
}

vector<uint8_t> CryptoHandler::formLengthString(size_t length)
{
    vector<uint8_t> L;
        if (length == 0) {
            // Case (a): Empty string
            return L;
        } else if (length < (1 << 16) - (1 << 8)) {
            // Case (b): 2-octet encoding
            L.push_back((length >> 8) & 0xFF);
            L.push_back(length & 0xFF);
        } else if (length < (1ULL << 32)) {
            // Case (c): 0xFFFE + 4-octet encoding
            L.push_back(0xFF);
            L.push_back(0xFE);
            for (int i = 3; i >= 0; --i) {
                L.push_back((length >> (8 * i)) & 0xFF);
            }
        //} else if (length < (1ULL << 64)) {
            // Case (d): 0xFFFF + 8-octet encoding
        //    L.push_back(0xFF);
        //    L.push_back(0xFF);
        //    for (int i = 7; i >= 0; --i) {
        //        L.push_back((length >> (8 * i)) & 0xFF);
        //    }
        } else {
            throw invalid_argument("Length too large for encoding.");
        }
        return L;
}

vector<uint8_t> CryptoHandler::padToBlockSize(const vector<uint8_t> input, size_t blockSize)
{
    vector<uint8_t> padded = input;
    while (padded.size() % blockSize != 0) {
        padded.push_back(0);
    }
    return padded;    
}

vector<uint8_t> CryptoHandler::authentication(const vector<uint8_t> key, const vector<uint8_t> plaintext, const vector<uint8_t> additionalData, 
                 const vector<uint8_t> nonce, int M, const size_t blockSize)
{
    // Step 1: Form B0
    size_t l = plaintext.size();
    vector<uint8_t> B0(blockSize, 0);
    B0[0] = ((additionalData.empty() ? 0 : 1) << 6) | (((M - 2) / 2) << 3) | (14 - nonce.size()); //15 ou 14?
    copy(nonce.begin(), nonce.end(), B0.begin() + 1);
    B0[blockSize - 2] = (l >> 8) & 0xFF; //pq & 0xFF?
    B0[blockSize - 1] = l & 0xFF; //pq & 0xFF?

    // Step 2: Form Authentication Data (AddAuthData)
    vector<uint8_t> AddAuthData;
    if (!additionalData.empty()) {
        vector<uint8_t> L_a = formLengthString(additionalData.size());
        AddAuthData.insert(AddAuthData.end(), L_a.begin(), L_a.end());
        AddAuthData.insert(AddAuthData.end(), additionalData.begin(), additionalData.end());
    }
    AddAuthData = padToBlockSize(AddAuthData, blockSize);

    // Step 3: Form PlaintextData
    vector<uint8_t> PlaintextData = padToBlockSize(plaintext, blockSize);

    // Step 4: Combine AuthData = AddAuthData || PlaintextData
    vector<uint8_t> AuthData = AddAuthData;
    AuthData.insert(AuthData.end(), PlaintextData.begin(), PlaintextData.end());

    // Step 5: CBC-MAC calculation
    vector<uint8_t> mac(blockSize, 0);
    encryptBlock(B0, mac, key);

    for (size_t i = 0; i < AuthData.size(); i += blockSize) {
        for (size_t j = 0; j < blockSize; ++j) {
            mac[j] ^= AuthData[i + j];
        }
        encryptBlock(mac, mac, key);
    }

    return vector<uint8_t>(mac.begin(), mac.begin() + M);
}

void CryptoHandler::encrypt(const vector<uint8_t> key, const vector<uint8_t> plaintext, const vector<uint8_t> additionalData, const vector<uint8_t> nonce,
                 int M, vector<uint8_t>& ciphertext, vector<uint8_t>& authTag, const size_t blockSize)
{
    if (nonce.size() != 13) {
        throw invalid_argument("Nonce must be 13 bytes.");
    }

    authTag = authentication(key, plaintext, additionalData, nonce, M);

    // Step 6: Encryption
    vector<uint8_t> S0(blockSize, 0);
    vector<uint8_t> A0(blockSize, 0);

    
    A0[0] = (14 - nonce.size()); //15 ou 14?
    copy(nonce.begin(), nonce.end(), A0.begin() + 1);
    A0[blockSize - 2] = 0;
    A0[blockSize - 1] = 0;


    encryptBlock(A0, S0, key);

    for (size_t i = 0; i < M; ++i) {
        authTag[i] ^= S0[i];
    }

    //ciphertext.resize(plaintext.size());
    for (size_t i = 0; i < plaintext.size(); ++i) {
        if (i % blockSize == 0) {
            uint16_t counter = (i / blockSize) + 1;
            A0[blockSize - 2] = (counter >> 8) & 0xFF; // Bits mais significativos
            A0[blockSize - 1] = counter & 0xFF;        // Bits menos significativos
            encryptBlock(A0, S0, key);
        }
        ciphertext[i] = plaintext[i] ^ S0[i % blockSize];
    }
}

bool CryptoHandler::decrypt(const vector<uint8_t> key, const vector<uint8_t> ciphertext, const vector<uint8_t> additionalData, const vector<uint8_t> nonce, 
                 const vector<uint8_t> authTag, int M, vector<uint8_t>& plaintext, const size_t blockSize)
{
    if (nonce.size() != 13) {
        throw invalid_argument("Nonce must be 13 bytes.");
    }

    vector<uint8_t> tag(M, 0);

    // Step 1: Generate S0
    vector<uint8_t> S0(blockSize, 0);
    vector<uint8_t> A0(blockSize, 0);

    A0[0] = (14 - nonce.size());
    
    copy(nonce.begin(), nonce.end(), A0.begin() + 1);
    A0[blockSize - 2] = 0;
    A0[blockSize - 1] = 0;
    

    encryptBlock(A0, S0, key);

    for (size_t i = 0; i < M; ++i) {
        tag[i] = authTag[i] ^ S0[i];
    }

    // Step 2: Decrypt Ciphertext
    //plaintext.resize(ciphertext.size());
    plaintext = vector<uint8_t>(ciphertext.size(),0);
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        if (i % blockSize == 0) {
            uint16_t counter = (i / blockSize) + 1;
            A0[blockSize - 2] = (counter >> 8) & 0xFF; // Bits mais significativos
            A0[blockSize - 1] = counter & 0xFF;        // Bits menos significativos
            encryptBlock(A0, S0, key);
        }
        plaintext[i] = ciphertext[i] ^ S0[i % blockSize];
    }

    // Step 3: Recalculate AuthTag
    vector<uint8_t> computedAuthTag;
    computedAuthTag = authentication(key, plaintext, additionalData, nonce, M);


    //Step 4: Verify AuthTag
    if (computedAuthTag != tag) {
        return false;
    }
    return true;
}



bool CryptoHandler::handle_decryption(vector<uint8_t> header, vector<uint8_t> payload, vector<uint8_t>& plaintext, bool isNwkLayer)//add prints de debug
{
    vector<uint8_t> newPayload;
    vector<uint8_t> nonce;
    vector<vector<uint8_t>> keys;
    uint8_t keyId;
    if(isNwkLayer)
    {
        keys = nwk_keys;
    }
    else
    {
        keys = link_keys;
    }
    std::vector<uint8_t> hashMsg;
    int frameControlHedearIndex = header.size();
    if (!PayloadHandler::extractAuxPayload(payload, newPayload, header, nonce, isNwkLayer, hashMsg))
    {
        return false;
    }
    for(int i = 0; i < keys.size(); i++)
    {
        vector<uint8_t> key;
        if (isNwkLayer)
        {
            key = keys[i]; 
        }
        else
        {
            key = hmac(keys[i], hashMsg);
        }
        if(security_level == -1)
        {
            int levels[4] = {4,8,16};
            for(int level = 0; level < 3; level++)
            {
                int M = levels[level];
                header[frameControlHedearIndex] += level + 5; // security level is overwritten with 0 on the packets
                nonce[12] += level + 5; // security level is overwritten with 0 on the packets
                vector<uint8_t> cyphertext = vector<uint8_t>(newPayload.begin(), newPayload.end() - M);
                vector<uint8_t> authTag = vector<uint8_t>(newPayload.end() - M, newPayload.end());
                
                if(decrypt(key, cyphertext, header, nonce, authTag, M, plaintext))
                {
                    security_level = level + 5;
                    cout << "[INFO] Security level found: " << security_level << "." << endl;
                    return true;
                }
            }
        }
        else
        {
            int M = (security_level - 4) * 4;
            vector<uint8_t> cyphertext = vector<uint8_t>(newPayload.begin(), newPayload.end() - M);
            vector<uint8_t> authTag = vector<uint8_t>(newPayload.end() - M, newPayload.end());
            header[frameControlHedearIndex] += security_level; // security level is overwritten with 0 on the packets
            nonce[12] += security_level; // security level is overwritten with 0 on the packets
            if(decrypt(key, cyphertext, header, nonce, authTag, M, plaintext))
            {
                return true;
            }
        }
    }
    return false;
}

bool CryptoHandler::extract_key(vector<uint8_t> payload)
{
    if (security_level < 5 && security_level != -1){
        return false;
    }
    vector<uint8_t> nwkLayer;
    if(!PayloadHandler::getNwkLayer(payload, nwkLayer))
    {
        return false;
    }
    bool security;
    vector<uint8_t> apsLayer;
    vector<uint8_t> nwkHeader;
    if(!PayloadHandler::extractNwkPayload(nwkLayer, apsLayer, nwkHeader, security))
    {
        return false;
    }
    
    if (security)
    {
        vector<uint8_t> plaintext;
        if(!CryptoHandler::handle_decryption(nwkHeader, apsLayer, plaintext, true)){
            return false;
        }
        apsLayer = plaintext;
    }
    vector<uint8_t> apsHeader;
    vector<uint8_t> auxLayer;
    if(!PayloadHandler::extractApsPayload(apsLayer, auxLayer, apsHeader, security))
    {
        return false;
    }
    if (!security)
    {
        return false; // if in the future we need to read other packets types this might be used
    }
    vector<uint8_t> plaintext;
    if (!CryptoHandler::handle_decryption(apsHeader,auxLayer, plaintext, false)){
        return false;
    }
    if (plaintext[0] == 0x05)
    {
        vector<uint8_t> key =  vector<uint8_t>(plaintext.begin() + 2, plaintext.begin() + 18);
        if (plaintext[1] == 0x01)
        {
            //add compare keys
            nwk_keys.push_back(key);
            cout << "[INFO] New key added to known Network Keys: " << bytesToHexString(key) << "." << endl;
        }
        else if (plaintext[1] == 0x04)
        {
            link_keys.push_back(key);
            cout << "[INFO] New key added to known Link Keys: " << bytesToHexString(key) << "." << endl;
        }
    }

    return true;
}