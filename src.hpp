#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <initializer_list>
#include <typeinfo>

namespace sjtu {

class any_ptr {
 private:
  struct holder_base {
    int ref_count;
    
    holder_base() : ref_count(1) {}
    virtual ~holder_base() {}
    virtual const std::type_info& type() const = 0;
  };
  
  template <typename T>
  struct holder : holder_base {
    T* ptr;
    
    holder(T* p) : ptr(p) {}
    
    ~holder() override {
      delete ptr;
    }
    
    const std::type_info& type() const override {
      return typeid(T);
    }
  };
  
  holder_base* data;
  
 public:
  any_ptr() : data(nullptr) {}

  any_ptr(const any_ptr &other) : data(other.data) {
    if (data) {
      ++data->ref_count;
    }
  }
  
  template <class T> 
  any_ptr(T *ptr) : data(ptr ? new holder<T>(ptr) : nullptr) {}

  ~any_ptr() {
    if (data) {
      --data->ref_count;
      if (data->ref_count == 0) {
        delete data;
      }
    }
  }

  any_ptr &operator=(const any_ptr &other) {
    if (this != &other) {
      if (data) {
        --data->ref_count;
        if (data->ref_count == 0) {
          delete data;
        }
      }
      data = other.data;
      if (data) {
        ++data->ref_count;
      }
    }
    return *this;
  }
  
  template <class T> 
  any_ptr &operator=(T *ptr) {
    if (data) {
      --data->ref_count;
      if (data->ref_count == 0) {
        delete data;
      }
    }
    data = ptr ? new holder<T>(ptr) : nullptr;
    return *this;
  }

  template <class T> 
  T &unwrap() {
    if (!data) {
      throw std::bad_cast();
    }
    if (data->type() != typeid(T)) {
      throw std::bad_cast();
    }
    holder<T>* h = static_cast<holder<T>*>(data);
    return *(h->ptr);
  }
  
  template <class T> 
  const T &unwrap() const {
    if (!data) {
      throw std::bad_cast();
    }
    if (data->type() != typeid(T)) {
      throw std::bad_cast();
    }
    const holder<T>* h = static_cast<const holder<T>*>(data);
    return *(h->ptr);
  }
};

template <class T> 
any_ptr make_any_ptr(const T &t) { 
  return any_ptr(new T(t)); 
}

template <class T, class... Args>
any_ptr make_any_ptr(Args&&... args) {
  return any_ptr(new T{std::forward<Args>(args)...});
}

}  // namespace sjtu

#endif
