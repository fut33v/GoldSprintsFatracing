//! @file Singleton.h
//! Объявление и реализация класса-обертки для синглтона.
//! Использование класса:
//! - добавить "friend class SingletonHolder<CClass>;"  в объявление синглтона
//! - предотвратить непосредственное создание синглтона, 
//!   т.е. конструкторы и деструктрор синглтона нужно сделать не public
//! - добавить "typedef SingletonHolder<CClass> CClassSingleton";

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <stdint.h>

#include <boost/thread.hpp>
#include <assert.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define ATEXIT_CALLING_CONVENTION_QUALIFIER __cdecl 
#else
#define ATEXIT_CALLING_CONVENTION_QUALIFIER 
#endif

namespace VMM {

//! Обертка синглтона.
//! Позволяет создавать синглтоны заданных классов. 
//! Допускается использование в многопоточной программе. 
//! При создании используется Double-Checked Locking.
//! Синглтон создается при первом обращении к Instance() и уничтожается при завершении программы 
//! (std::atexit()).
template <class T>
class SingletonHolder
{
public:
    //  Returns a reference to singleton object
    static T& Instance();

private:
    // Helpers
    static void MakeInstance();
    static void ATEXIT_CALLING_CONVENTION_QUALIFIER DestroySingleton();

    // Protection
    SingletonHolder();

    // Data
    static /*volatile*/ T*  m_pInstance;
    static bool             m_destroyed;
};

// Данные синглтона
template <class T>
T* SingletonHolder<T>::m_pInstance = nullptr;
template <class T>
bool SingletonHolder<T>::m_destroyed = false;

//! Возвращает экземпляр синглтона
//! @return экземпляр синглтона
template <class T>
inline T& SingletonHolder<T>::Instance()
{
    if (!m_pInstance)
    {
        MakeInstance();
    }
    return *m_pInstance;
}

//! Создает синглтон
template <class T>
void SingletonHolder<T>::MakeInstance()
{
    static boost::mutex lockMakeInstance;

    boost::unique_lock<boost::mutex> lock(lockMakeInstance);

    if (!m_pInstance)
    {
        assert(!m_destroyed);

        m_pInstance = new T;
        atexit(&DestroySingleton);
    }
}

//! Удаляет синглтон. Вызывается во время завершения программы.
template <class T>
void ATEXIT_CALLING_CONVENTION_QUALIFIER SingletonHolder<T>::DestroySingleton()
{
    assert(!m_destroyed);

    delete m_pInstance;
    m_pInstance = NULL;
    m_destroyed = true;
}

} // namespace VMM
#endif // _SINGLETON_H
