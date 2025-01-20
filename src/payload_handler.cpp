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

#include "payload_handler.hpp"

bool PayloadHandler::parseAddressingInfo(uint8_t highByte, uint8_t lowByte, size_t &offset) {//revisar essa função
    // Extrair modos de endereçamento de destino e origem
    uint8_t frameType = (lowByte) & 0b00000111; // Bits [0] (lowByte)
    if (frameType != 0x01){//data frame
        return false;
    }
    uint8_t destAddrMode = (highByte & 0b00001100) >> 2; // Bits [11:10] (highByte)
    uint8_t srcAddrMode = (highByte & 0b11000000) >> 6; // Bits [15:14] (highByte)

    offset += 3; // Frame Control and sequence number

    // Tamanho do endereço de destino
    if (destAddrMode == 0x02) {
        offset += 2; // Endereço curto (2 bytes)
    } else if (destAddrMode == 0x03) {
        offset += 8; // Endereço estendido (8 bytes)
    }

    // PAN ID Compression (bit 6 do lowByte)
    bool panIdCompression = lowByte & 0b01000000;

    // Adiciona PAN ID de destino se Addressing Mode do destino for diferente de 0b00
    if (destAddrMode != 0x00) {
        offset += 2; // PAN ID de destino
    }

    // Adiciona PAN ID de origem se PAN ID Compression estiver desativado e Addressing Mode da origem não for 0b00
    if (!panIdCompression && srcAddrMode != 0x00) {
        offset += 2; // PAN ID de origem
    }

    // Tamanho do endereço de origem
    if (srcAddrMode == 0x02) {
        offset += 2; // Endereço curto (2 bytes)
    } else if (srcAddrMode == 0x03) {
        offset += 8; // Endereço estendido (8 bytes)
    }

    return true;
}

bool PayloadHandler::getNwkLayer(std::vector<uint8_t> payload, std::vector<uint8_t>& nwkLayer){
    size_t offset = 0;
    if (!parseAddressingInfo(payload[1], payload[0], offset) || payload.size() <= offset + 3)
    {
        return false;
    }
    nwkLayer.insert(nwkLayer.begin(), payload.begin() + offset, payload.end() - 2);
    return true;
}

bool PayloadHandler::parseNwkHeader(const std::vector<uint8_t>& frame, size_t& offset, bool& securityEnabled) {
    if (frame.size() < offset + 2) {

        return false;
    }

    // Frame Control (2 bytes)
    uint8_t frameControlLow = frame[offset];
    uint8_t frameControlHigh = frame[offset + 1];
    offset += 2;

    // Extrair subcampos do Frame Control
    uint8_t frameType = frameControlLow & 0b00000011;              // Bits [1:0]
    //uint8_t protocolVersion = frameControlLow & 0b00111100; // Bits [5:2]
    //uint8_t discoverRoute = frameControlLow & 0b11000000;       // Bit [7:6]
    //bool multicastFlag = frameControlHigh & 0b00000001;       // Bit [8]
    securityEnabled = frameControlHigh & 0b00000010;    // Bit [9]
    bool sourceRoute = frameControlHigh & 0b00000100;        // Bit [10]
    bool destinationIeeeAddr = frameControlHigh & 0b00001000; // Bit [11]
    bool sourceIeeeAddr = frameControlHigh & 0b00010000;     // Bit [12]


    //if (securityEnabled){
        //std::cout << std::hex << static_cast<int>(frameControlHigh) << std::endl;
    //}
    // Validação do tipo de quadro
    if (frameType != 0x00) { // 0b00 indica um quadro de dados
        return false;
    }

    // Endereço de destino (2 bytes fixos)
    if (frame.size() < offset + 2) {
        return false;
    }
    offset += 2;

    // Endereço de origem (2 bytes fixos)
    if (frame.size() < offset + 2) {
        return false;
    }
    offset += 2;

    // Campo Radius (1 byte fixo)
    if (frame.size() < offset + 1) {
        return false;
    }
    offset += 1;

    // Campo Sequence Number (1 byte fixo)
    if (frame.size() < offset + 1) {
        return false;
    }
    offset += 1;

    // Endereço IEEE de destino (se presente, 8 bytes)
    if (destinationIeeeAddr) {
        if (frame.size() < offset + 8) {
            return false;
        }
        offset += 8;
    }

    // Endereço IEEE de origem (se presente, 8 bytes)
    if (sourceIeeeAddr) {
        if (frame.size() < offset + 8) {
            return false;
        }
        offset += 8;
    }

    // Subquadro de roteamento de origem (se presente)
    if (sourceRoute) {
        if (frame.size() < offset + 2) { // Pelo menos Relay Count e Relay Index
            return false;
        }
        uint8_t relayCount = frame[offset];
        //uint8_t relayIndex = frame[offset + 1];
        offset += 2;

        // Lista de relés (relayCount * 2 bytes)
        size_t relayListSize = relayCount * 2;
        if (frame.size() < offset + relayListSize) {
            return false;
        }
        offset += relayListSize;
    }

    // O offset agora está no início do payload
    return true;
}

bool PayloadHandler::extractNwkPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, bool& securityEnabled) {
    size_t offset = 0;
    // Processar cabeçalho NWK
    if(parseNwkHeader(frame, offset, securityEnabled))
    {
        payload.insert(payload.begin(), frame.begin() + offset, frame.end());
        header.insert(header.begin(), frame.begin(), frame.begin() + offset);
        return true;
    }

    // Extrair payload a partir do offset
    
    return false;
}


bool PayloadHandler::parseApsHeader(const std::vector<uint8_t>& frame, size_t& offset, bool& securityEnabled) {
    if (frame.size() < 1) {
        return false;
    }

    // Frame Control (1 byte)
    uint8_t frameControl = frame[offset];

    // Extrair subcampos do Frame Control
    uint8_t frameType = frameControl & 0b00000011;              // Bits [1:0]
    uint8_t deliveryMode = (frameControl & 0b00001100) >> 2; // Bits [3:2]
    bool ack = frameControl & 0b00010000;       // Bit [4]
    securityEnabled = frameControl & 0b00100000;       // Bit [5]
    bool ackReq = frameControl & 0b01000000;       // Bit [6]
    bool extendedHeader = frameControl & 0b10000000;       // Bit [7]
    // Validação do tipo de quadro
    if (frameType != 0x01) { // 0b00 indica um quadro de dados
        return false;
    } 
    
    if (ack)
    {
        offset += 1;//Source Endpoint Field
        if ((deliveryMode == 0x00 || deliveryMode == 0x02)) {//Destination Endpoint Field
            offset += 1;
        }
        if (deliveryMode == 0x03) { //Group Address Field
            offset += 2;
        }
    }
    //This if will be necessary if we start to read data or acknowledgement frame type
    //else if (frameType == 0x00 || frameType == 0x02) //Cluster Identifier Field and Profile Identifier Field
    //{
    //    offset += 4;
    //} 
    
    offset += 2; //frame control and counter

    // Endereço de destino (2 bytes fixos)
    if (frame.size() < offset) {
        return false;
    }
    

    if (extendedHeader){
        //std:: cout << "No support for extended header" << std::endl;
        return false;
    }
    return true;
}

bool PayloadHandler:: extractApsPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, bool& securityEnabled) {
    size_t offset = 0;
    // Processar cabeçalho Aps
    if(parseApsHeader(frame, offset, securityEnabled))
    {
        payload.insert(payload.begin(), frame.begin() + offset, frame.end());
        header.insert(header.begin(), frame.begin(), frame.begin() + offset);
        return true;
    }

    // Extrair payload a partir do offset
    
    return false;
}


bool PayloadHandler::extractAuxPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, std::vector<uint8_t>& nonce, bool isNwkLayer, std::vector<uint8_t>& hashMsg) {
    
    if (frame.size() < 15) {
        return false;
    }

    size_t offset = 13;
    // Frame Control (1 byte)

    uint8_t frameControl = frame[0];
    // Extrair subcampos do Frame Control
    uint8_t keyId = (frameControl & 0b00011000) >> 3; // Bits [4:3]

    //frameControl += security_level;
    nonce.insert(nonce.begin(), frame.begin() + 5, frame.begin() + 13);
    nonce.insert(nonce.end(), frame.begin() + 1, frame.begin() + 5);
    nonce.push_back(frameControl);

    if (isNwkLayer)
    {
        if (keyId != 0x01){
            return false;
        }
        offset += 1;
    }
    else 
    {
        if (keyId == 0x02) 
        {
            hashMsg.push_back(0x00);
        }
        else if(keyId == 0x03)
        {
            hashMsg.push_back(0x02);
        }
        else
        {
            return false;
        }
    }
    payload.insert(payload.begin(), frame.begin() + offset, frame.end());
    header.push_back(frameControl);
    header.insert(header.end(), frame.begin() + 1, frame.begin() + offset);
    
    return true;
}