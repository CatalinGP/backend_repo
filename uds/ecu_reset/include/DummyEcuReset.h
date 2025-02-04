/**
 * @file DummyEcuReset.h
 * @author Stefan Corneteanu
 * @brief ECU Reset service Dummy class.
 * To be used in unit tests instead of the actual EcuReset
 */

#ifndef DUMMY_ECU_RESET_H
#define DUMMY_ECU_RESET_H

#include <cstdint>

#include "EcuReset.h"
#include "Logger.h"

class DummyEcuReset : public EcuReset {
public:
    /**
     * @brief Parameterized constructor.
     * 
     * @param can_id CAN identifier
     * @param sub_function Subfunction for ECU Reset
     * @param socket Socket to be used for response
     * @param logger The logger
     * @see EcuReset ctor
     */
    DummyEcuReset(uint32_t can_id, uint8_t sub_function, int socket, Logger &logger);

    /**
     * @brief Default dtor
     */
    ~DummyEcuReset() = default;

    void hardReset() override;
};

#endif /* DUMMY_ECU_RESET_H */