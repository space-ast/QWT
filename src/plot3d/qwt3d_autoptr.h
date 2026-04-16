#ifndef qwt3d_autoptr_h
#define qwt3d_autoptr_h

namespace Qwt3D {

/**
 * \if ENGLISH
 * @brief Simple auto pointer providing deep copies for raw pointer
 * @details Requirements:\n
 *          virtual T* T::clone() const;\n
 *          T::destroy() const;\n
 *          virtual ~T() private/protected\n\n
 *          clone() is necessary for the pointer to preserve polymorphic behaviour.
 *          The pointer requires also heap based objects with regard to the template
 *          argument in order to be able to get ownership and control over destruction.
 * \endif
 *
 * \if CHINESE
 * @brief 提供原始指针深拷贝的简单自动指针
 * @details 要求：\n
 *          virtual T* T::clone() const;\n
 *          T::destroy() const;\n
 *          virtual ~T() private/protected\n\n
 *          clone() 是指针保持多态行为所必需的。
 *          指针还要求模板参数对应的对象是基于堆分配的，
 *          以便能够获取所有权并控制销毁过程。
 * \endif
 */
template<typename T>
class qwt3d_ptr
{
public:
    // Standard ctor
    explicit qwt3d_ptr(T *ptr = 0) : rawptr_(ptr) { }
    // Dtor (calls T::destroy)
    ~qwt3d_ptr() { destroyRawPtr(); }

    // Copy ctor (calls (virtual) clone())
    qwt3d_ptr(qwt3d_ptr const &val) { rawptr_ = val.rawptr_->clone(); }

    // Assignment in the same spirit as copy ctor
    qwt3d_ptr<T> &operator=(qwt3d_ptr const &val)
    {
        if (this == &val)
            return *this;

        destroyRawPtr();
        rawptr_ = val.rawptr_->clone();

        return *this;
    }

    // Pointer-like access operator
    T *operator->() const { return rawptr_; }

    // Dereferencing operator
    T &operator*() const { return *rawptr_; }

private:
    T *rawptr_;
    void destroyRawPtr()
    {
        if (rawptr_)
            rawptr_->destroy();
        rawptr_ = 0;
    }
};

} // ns

#endif /* include guarded */