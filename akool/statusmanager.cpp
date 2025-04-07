#include "statusmanager.h"

StatusManager::StatusManager()
{

}

StatusManager* StatusManager::getInstance()
{
    static StatusManager* pInstance = new StatusManager();
    return pInstance;
}
