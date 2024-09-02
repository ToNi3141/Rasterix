#ifndef ARDUINOBUSCONNECTOR_H
#define ARDUINOBUSCONNECTOR_H

#include "IBusConnector.hpp"
#include <tcb/span.hpp>

namespace rr
{
class ArduinoBusConnector : IBusConnector
{
public:
    virtual ~ArduinoBusConnector() = default;

    ArduinoBusConnector()
    {
    }

    virtual void writeData(const uint8_t index, const uint32_t size) override
    {
    }

    virtual bool clearToSend() override
    {
        return true;
    }


private:

};

} // namespace rr
#endif // ARDUINOBUSCONNECTOR_H
