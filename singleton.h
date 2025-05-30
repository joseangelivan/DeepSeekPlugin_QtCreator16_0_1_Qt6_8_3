#ifndef SINGLETON_H
#define SINGLETON_H

#include <QMutex>
#include <QMutexLocker>
#include <QAtomicPointer>
#include <QDebug>

template<class T>
class Singleton
{
public:
    static T* inst() {
        // Doble verificación locking pattern para mejor performance
        if (m_instance.loadAcquire() == nullptr) {
            QMutexLocker locker(&m_mutex);
            if (m_instance.loadAcquire() == nullptr) {
                m_instance.storeRelease(new T);
                qDebug() << "Singleton instance created for type:" << typeid(T).name();
            }
        }
        return m_instance.loadAcquire();
    }

    static void destroyInstance() {
        QMutexLocker locker(&m_mutex);
        T* instance = m_instance.loadAcquire();
        if (instance != nullptr) {
            delete instance;
            m_instance.storeRelease(nullptr);
            qDebug() << "Singleton instance destroyed for type:" << typeid(T).name();
        }
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    Q_DISABLE_COPY(Singleton)

    static QMutex m_mutex;
    static QAtomicPointer<T> m_instance;
};

// Inicialización de los miembros estáticos
template<class T> QMutex Singleton<T>::m_mutex;
template<class T> QAtomicPointer<T> Singleton<T>::m_instance = nullptr;

#endif // SINGLETON_H