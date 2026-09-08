#include "DatabaseTaskWorker.hpp"


bool DatabaseTaskWorker::Initialize()
{
    if (!BasicThreadPoolElement::Initialize()) return false;

    return true;
}