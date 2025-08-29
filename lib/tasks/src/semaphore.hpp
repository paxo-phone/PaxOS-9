#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <vector>

class Lockers {};

class Semaphore // The object that will be locked
{
    public:
    Semaphore();

    bool isLocked() { return !lockers.empty(); }
    int count() { return lockers.size(); }

    std::vector <Lockers*> lockers; // List of lockers that have locked this semaphore
};

class Locker : public Lockers // The object that will lock the semaphore
{
    public:
    Locker(Semaphore* semaphore);
    ~Locker();

    Semaphore* semaphore;
};

class ManualLocker : public Lockers // A locker that needs to be manually unlocked
{
    public:
    ManualLocker(Semaphore* semaphore);

    void lock();
    void unlock();

    Semaphore* semaphore;
};

#endif