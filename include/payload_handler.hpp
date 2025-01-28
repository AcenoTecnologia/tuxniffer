////////////////////////////////////////////////////////////////////////////////////////////////////
// Company:  Aceno Digital Tecnologia em Sistemas Ltda.
// Homepage: http://www.aceno.com
// Project:  Tuxniffer
// Version:  1.1.3
// Date:     2025
//
// Copyright (C) 2002-2025 Aceno Tecnologia.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <cstdint>

/**
 * @class PayloadHandler
 * @brief Navigate through Network Layer and exctract necessary data for decryption.
 */
class PayloadHandler {
private:    

/**
     * @brief Process the Frame Control field from MAC Layer and calculate the offset to the Zigbee Network Layer if possible.
     * Reference: annex G.3.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * Will return false if frame type is not data frame.
     * 
     * @param highByte High byte of the Frame Control field from MAC Layer.
     * @param lowByte Low byte of the Frame Control field from MAC Layer.
     * @param offset Poiter to int where the resulting offset will be saved.
     * @return Bool value indicating if the offset was calculated. Will return false if the operation fail or frame type is not data frame.
     */
static bool parseAddressingInfo(uint8_t highByte, uint8_t lowByte, size_t &offset);


/**
     * @brief Process the Frame Control field from Zigbee Network Layer and calculate the offset to the next layer if possible.
     * Reference: subsection 3.3.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * Will return false if frame type is not data frame.
     * 
     * @param frame Byte vector with the Zigbee Network Layer.
     * @param offset Poiter to int where the resulting offset will be saved.
     * @param securityEnabled Pointer to bool value indicating if the layer has security enabled.
     * @return Bool value indicating if the offset was calculated. Will return false if the operation fail or frame type is not data frame.
     */
static bool parseNwkHeader(const std::vector<uint8_t>& frame, size_t& offset, bool& securityEnabled);

/**
     * @brief Process the Frame Control field from Zigbee Application Support Layer and calculate the offset to the next layer if possible.
     * Reference: subsection 3.3.1 from 2.2.5.1 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * Will return false if frame type is not command frame.
     * 
     * @param frame Byte vector with the Zigbee Application Support Layer.
     * @param offset Poiter to int where the resulting offset will be saved.
     * @param securityEnabled Pointer to bool value indicating if the layer has security enabled.
     * @return Bool value indicating if the offset was calculated. Will return false if the operation fail or frame type is not command frame.
     */
static bool parseApsHeader(const std::vector<uint8_t>& frame, size_t& offset, bool& securityEnabled);
public:

/**
     * @brief Process the MAC layer (payload of the TI layer) and extract the Zigbee Network Layer if possible.
     * 
     * @param payload Byte vector with the MAC layer.
     * @param nwkLayer Poiter to byte vector where the Zigbee Network Layer will be saved.
     * @return Bool value indicating if the Zigbee Network Layer was successful extracted.
     */
static bool getNwkLayer(std::vector<uint8_t> payload, std::vector<uint8_t>& nwkLayer);

/**
     * @brief Process the Zigbee Network Layer and extract the next layer if possible.
     * 
     * @param frame Byte vector with the Zigbee Network Layer.
     * @param payload Poiter to byte vector where the next layer will be saved.
     * @param header Poiter to byte vector where the Zigbee Network Layer header will be saved.
     * @param securityEnabled Pointer to bool value indicating if the layer has security enabled.
     * @return Bool value indicating if the next layer was successful extracted.
     */
static bool extractNwkPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, bool& securityEnabled);

/**
     * @brief Process the Zigbee Application Support Layer and extract the next layer if possible.
     * 
     * @param frame Byte vector with the Zigbee Application Support Layer.
     * @param payload Poiter to byte vector where the next layer will be saved.
     * @param header Poiter to byte vector where the Zigbee Network Layer header will be saved.
     * @param securityEnabled Pointer to bool value indicating if the layer has security enabled.
     * @return Bool value indicating if the next layer was successful extracted.
     */
static bool extractApsPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, bool& securityEnabled);

/**
     * @brief Process the security header from a zigbee layer and extract the necessary data for its decryption.
     * Reference: sections 4.3 to 4.5 from https://csa-iot.org/wp-content/uploads/2023/04/05-3474-23-csg-zigbee-specification-compressed.pdf.
     * 
     * @param frame Byte vector with the layer payload including the security header.
     * @param payload Poiter to byte vector where the layer payload will be saved.
     * @param header Poiter to byte vector with the layer header and where the security header will be appended.
     * @param nonce Poiter to byte vector where the nonce from the security header will be saved.
     * @param hashMsg Poiter to byte vector where the message to be hashed will be saved if necessary. 
     * Only used for Zigbee Application Support Layer depending on the key type.
     * @param isNwkLayer Bool value idicating the layer where the security header was extracted.
     * Zigbee Network Layer if true and Zigbee Application Support Layer if false.    
     * @return Bool value indicating if the payload and the necessary data for decryption was successful extracted.
     */
static bool extractAuxPayload(const std::vector<uint8_t> frame, std::vector<uint8_t>& payload, std::vector<uint8_t>& header, std::vector<uint8_t>& nonce, bool isNwkLayer, std::vector<uint8_t>& hashMsg);

};
