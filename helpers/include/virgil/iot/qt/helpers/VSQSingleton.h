

#ifndef VIRGIL_IOTKIT_QT_VSQSINGLETON_H
#define VIRGIL_IOTKIT_QT_VSQSINGLETON_H

template <typename T>
class VSQSingleton
{
public:
    static T& instance()
    {
        static T _instance;
        return _instance;
    }
protected:
    VSQSingleton() {}
    ~VSQSingleton() {}
public:
    VSQSingleton(VSQSingleton const &) = delete;
    VSQSingleton& operator=(VSQSingleton const &) = delete;
};


#endif //VIRGIL_IOTKIT_QT_VSQSINGLETON_H
