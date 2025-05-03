//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class SingletonMixin {
public:
    static T& getInstance() {
        static T instance;  // Guaranteed to be created and destroyed once in a thread-safe way
        return instance;
    }

    SingletonMixin(const SingletonMixin&) = delete;             // Disable copy constructor
    SingletonMixin& operator=(const SingletonMixin&) = delete;  // Disable copy assignment constructor

protected:
    SingletonMixin() {
        // Protected constructor to prevent direct instantiation

    }
    virtual ~SingletonMixin() {

    }
};

#endif // SINGLETON_H
