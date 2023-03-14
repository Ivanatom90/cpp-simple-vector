#pragma once
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include "array_ptr.h"
#include <utility>

class ReserveProxyObj{
public:
    ReserveProxyObj(size_t res):reserve_(res) {}
    size_t GetRes(){
        return reserve_;
    }
private:
    size_t reserve_;

};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size): s_vector_(size), size_(size), capacity_(size) {
        for (size_t i = 0; i < size; i++){
            s_vector_[i] = 0;
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value): s_vector_(size), size_(size), capacity_(size) {
        for (size_t i = 0; i < size; i++){
            s_vector_[i] = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init): s_vector_(init.size()), size_(init.size()), capacity_(init.size()) {
         std::copy(init.begin(), init.end(), begin());
    }


    SimpleVector(const SimpleVector& other) {
           SimpleVector tmp(other.size_);
           std::copy(other.begin(), other.end(), tmp.begin());
           tmp.size_ = other.size_;
           tmp.capacity_ = other.capacity_;
           swap(tmp);
       }

    SimpleVector(SimpleVector&& other) {
          capacity_ = std::move(other.capacity_);
          size_ =std::move(other.size_);
          swap(other);
          other.size_ =0;
       }

    SimpleVector(ReserveProxyObj Rpo) {
           SimpleVector tmp(Rpo.GetRes());
           tmp.size_ = 0;
           tmp.capacity_ = Rpo.GetRes();
           swap(tmp);
       }


    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return !size_;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return s_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return s_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
            if (index>=size_){
                throw std::out_of_range("Not-element");
            } else return s_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index>=size_){
            throw std::out_of_range("Not-element");
        } else return s_vector_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size<=size_){
            size_ = new_size;
            return;
        }
        if(capacity_ < new_size) {
            ArrayPtr<Type> tmp(new_size*2);
            //std::generate(&tmp[0], &tmp[new_size], Type());
            std::move(begin(), end(), &tmp[0]);
            s_vector_.swap(tmp);
            size_=new_size;
            capacity_=new_size*2;
        } else if(new_size>size_){
                for (Iterator it = end(); it != begin()+new_size; it++){
                    *it =std::move(Type());
                }
                size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator {s_vector_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator {&s_vector_[size_]};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
      return  cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
       return ConstIterator {&s_vector_[0]};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
      return  ConstIterator {&s_vector_[size_]};
    }

    void Reserve(size_t new_capacity){
        if (capacity_ == new_capacity){return;}
        if(capacity_<new_capacity){
            ReserveProxyObj Rpo(new_capacity);
            SimpleVector tmp(Rpo);
            tmp.size_ = size_;
            std::copy(begin(), end(), tmp.begin());
            swap(tmp);
        }

    }

       SimpleVector& operator=(const SimpleVector& rhs) {
           SimpleVector tmp(rhs);
           swap(tmp);
           return *this;
       }

       // Добавляет элемент в конец вектора
       // При нехватке места увеличивает вдвое вместимость вектора

       void PushBack(const Type& item) {
           if (capacity_> size_){
               s_vector_[size_++] = item;
               return;
           }
           if(capacity_ < size_) {
               ++capacity_ *= 3;
               ArrayPtr<Type> tmp(capacity_);
               std::fill(&tmp[0], &tmp[capacity_], Type());
               std::copy(begin(), end(), &tmp[0]);
               s_vector_.swap(tmp);
               s_vector_[size_++] = item;

           } else {
                   std::copy(end(), begin()+(++size_), Type());
                   s_vector_[size_-1] = item;
           }}

       void PushBack(Type&& item) {
           if (capacity_>size_ && capacity_ !=0){
                s_vector_[size_++] = std::move(item);
                return;
           } else{
               if (capacity_ == 0) {size_ = 0;}
               ++capacity_ *= 3;

               ArrayPtr<Type> tmp(capacity_);
               if(!IsEmpty()){
                    std::move(begin(), end(), &tmp[0]);
               }

               tmp[size_++] = std::move(item);
               s_vector_.swap(tmp);
               return;
           }
}

       // Вставляет значение value в позицию pos.
       // Возвращает итератор на вставленное значение
       // Если перед вставкой значения вектор был заполнен полностью,
       // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
       Iterator Insert(ConstIterator pos, const Type& value) {
            assert(begin() <= pos && end() >= pos);
               auto index = std::distance(cbegin(), pos);
               if (size_== capacity_) {
                   if(size_) {
                       ArrayPtr<Type> tmp(size_*=2);
                       std::copy(begin(), end(), &tmp[0]);
                       s_vector_.swap(tmp);
                       size_ = size_;
                       capacity_ = size_*2;
                   } else {
                       ArrayPtr<Type> tmp(++capacity_);
                       std::copy(begin(), end(), &tmp[0]);
                       s_vector_.swap(tmp);
                       capacity_ =1;
                   }
               }
               for (size_t i = size_; i > (size_t)index; --i) {
                   s_vector_[i] = s_vector_[i-1];
               }
               ++size_;
               s_vector_[index] = value;
               return const_cast<Iterator>(index +begin());

       }



       Iterator Insert(ConstIterator pos, Type&& value) {
            assert(begin() <= pos && end() >= pos);
                  if (!capacity_) {
                      ArrayPtr<Type> tmp(++capacity_);
                      std::move(begin(), end(), &tmp[0]);
                      s_vector_.swap(tmp);
                      s_vector_[size_++] = std::move(value);
                      return begin();
                  } else if (capacity_ < size_ || capacity_ == size_) {
                      auto index = std::distance(begin(), const_cast<Iterator>(pos));
                      ArrayPtr<Type> tmp(capacity_*=2);
                      std::move(begin(), end(), &tmp[0]);
                      std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(begin()+size_), (&tmp[1 + size_]));
                      tmp[index] = std::move(value);
                      ++size_;
                      s_vector_.swap(tmp);
                      return Iterator(&s_vector_[index]);
                  } else {
                      std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(end()), (&s_vector_[++size_+1]));
                      *const_cast<Iterator>(pos) = std::move(value);
                      return const_cast<Iterator>(pos);
                  }
          }


       // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
       void PopBack() noexcept {
           assert(!IsEmpty());
               size_--;
       }

       // Удаляет элемент вектора в указанной позиции
       Iterator Erase(ConstIterator pos) {
           assert(!IsEmpty());
           assert(begin() <= pos && end() >= pos);
           Iterator pos_no_const = const_cast<Iterator>(pos);
           std::move(pos_no_const + 1, end(), pos_no_const);
           size_--;
           return pos_no_const;
       }

       // Обменивает значение с другим вектором
       void swap(SimpleVector& other) noexcept {
           s_vector_.swap(other.s_vector_);
           std::swap(size_, other.size_);
           std::swap(capacity_, other.capacity_);
       }

private:
    ArrayPtr<Type> s_vector_;
    size_t size_= 0;
    size_t capacity_= 0;
};




ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) ;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs > rhs);
}

