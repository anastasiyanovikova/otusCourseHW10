#include "async.h"
#include "bulk.cpp"

void* connect(std::size_t N)
{
    return new commandController(N);
}
void receive(const char * data, std::size_t dataSize, void* context)
{
    auto controller = static_cast<commandController*>(context);
    if(nullptr == controller)
        return;
    controller->addCommand(std::string{data, dataSize});
}
void disconnect(void* context)
{
    std::string endStr = "EOF";
    receive(endStr.c_str(), endStr.size(), context);
    auto controller = static_cast<commandController*>(context);
    if(nullptr == controller)
        return;
    delete controller;
}
