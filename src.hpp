#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <initializer_list>
#include <typeinfo>
#include <utility>
#include <cstddef>

// 禁止使用 std::shared_ptr 与 std::any

namespace sjtu {

class any_ptr {
 private:
  struct control_block_base {
    std::size_t ref_cnt{1};
    virtual ~control_block_base() = default;
    virtual const std::type_info &type() const = 0;
    virtual void *ptr() = 0;
  };

  template <class T>
  struct control_block : control_block_base {
    T *p;
    explicit control_block(T *ptr) : p(ptr) {}
    ~control_block() override { delete p; }
    const std::type_info &type() const override { return typeid(T); }
    void *ptr() override { return static_cast<void *>(p); }
  };

  control_block_base *ctrl_{nullptr};

  void retain() {
    if (ctrl_) ++ctrl_->ref_cnt;
  }
  void release() {
    if (!ctrl_) return;
    if (--ctrl_->ref_cnt == 0) {
      delete ctrl_;
    }
    ctrl_ = nullptr;
  }

 public:
  /**
   * @brief 默认构造函数，行为应与创建空指针类似
   */
  any_ptr() = default;

  /**
   * @brief 拷贝构造函数，浅拷贝，共享同一块内存
   */
  any_ptr(const any_ptr &other) : ctrl_(other.ctrl_) { retain(); }

  /**
   * @brief 由原始指针构造，接管其生命周期
   */
  template <class T>
  explicit any_ptr(T *ptr) {
    if (ptr)
      ctrl_ = new control_block<T>(ptr);
    else
      ctrl_ = nullptr;
  }

  /**
   * @brief 析构函数
   */
  ~any_ptr() { release(); }

  /**
   * @brief 拷贝赋值运算符，浅拷贝
   */
  any_ptr &operator=(const any_ptr &other) {
    if (this == &other) return *this;
    // increase first to handle self/aliasing safety in exceptional scenarios
    if (other.ctrl_) ++other.ctrl_->ref_cnt;
    release();
    ctrl_ = other.ctrl_;
    return *this;
  }

  /**
   * @brief 由原始指针赋值，接管其生命周期
   */
  template <class T>
  any_ptr &operator=(T *ptr) {
    release();
    if (ptr)
      ctrl_ = new control_block<T>(ptr);
    else
      ctrl_ = nullptr;
    return *this;
  }

  /**
   * @brief 获取该对象指向的值的引用
   * @note 若该对象指向的值不是 T 类型，则抛出异常 std::bad_cast
   */
  template <class T>
  T &unwrap() {
    if (!ctrl_ || ctrl_->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<T *>(ctrl_->ptr());
  }

  template <class T>
  const T &unwrap() const {
    if (!ctrl_ || ctrl_->type() != typeid(T)) throw std::bad_cast();
    return *static_cast<const T *>(ctrl_->ptr());
  }
};

/**
 * @brief 由指定类型的值构造一个 any_ptr 对象
 */
template <class T>
any_ptr make_any_ptr(const T &t) {
  return any_ptr(new T(t));
}

// 由不定长参数构造（完美转发）
template <class T, class... Args>
any_ptr make_any_ptr(Args &&...args) {
  // Prefer initializer-list/brace initialization when available
  return any_ptr(new T{std::forward<Args>(args)...});
}

// 由初始化列表构造
template <class T>
any_ptr make_any_ptr(std::initializer_list<typename T::value_type> il) {
  return any_ptr(new T(il));
}

// 初始化列表 + 其余参数（如 allocator 等）
template <class T, class... Args>
any_ptr make_any_ptr(std::initializer_list<typename T::value_type> il,
                     Args &&...args) {
  return any_ptr(new T(il, std::forward<Args>(args)...));
}

}  // namespace sjtu

#endif
