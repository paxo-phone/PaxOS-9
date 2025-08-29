#include <semaphore.hpp>
#include <algorithm>
#include <vector>

Semaphore::Semaphore()
{

}

Locker::Locker(Semaphore* semaphore)
{
    this->semaphore = semaphore;
    semaphore->lockers.push_back(this);
}

Locker::~Locker()
{
    auto& lockers = semaphore->lockers;
    lockers.erase(std::remove(lockers.begin(), lockers.end(), this), lockers.end());
}

ManualLocker::ManualLocker(Semaphore* semaphore)
{
    this->semaphore = semaphore;
}

void ManualLocker::lock()
{
    auto& lockers = semaphore->lockers;
    if (std::find(lockers.begin(), lockers.end(), this) == lockers.end())
        lockers.push_back(this);
}

void ManualLocker::unlock()
{
    auto& lockers = semaphore->lockers;
    lockers.erase(std::remove(lockers.begin(), lockers.end(), this), lockers.end());
}