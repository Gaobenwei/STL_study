//
// Created by 11048 on 2023/6/12.
//
// 这个头文件包含了一个模板类 deque
// deque: 双端队列
/*
 *   //插入元素
        template<class ...Args>
        iterator insert_aux(iterator pos,Args&& ...args);
        void fill_insert(iterator pos,size_type n,const value_type& value);
        template<class IIter>
        void copy_insert(iterator pos,IIter first,IIter last,size_type n);

        template<class IIter>
        void insert_dispatch(iterator pos,IIter first,IIter last,input_iterator_tag);

        template<class FIter>
        void insert_dispatch(iterator pos,FIter first,FIter last,forward_iterator_tag);

        //reallocate 当map空间不足时，重新配置map空间
        void require_capacity(size_type n, bool front); // n为需要的容量大小
        void reallocate_map_at_front(size_type need); // 在map的前端重新配置空间
        void reallocate_map_at_back(size_type need); // 在map的尾端重新配置空间
        void reallocate_map(size_type need, bool front); // 重新配置map空间
以上函数；逻辑懂不了
 */
// notes:
//
// 异常保证：
// mystl::deque<T> 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证：
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert
#ifndef STL_STUDY_DEQUE_H
#define STL_STUDY_DEQUE_H

#include <initializer_list>
#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

namespace mystl
{
#ifdef max
    #pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
    #pragma message("#undefing marco min")
#undef min
#endif // min

    // deque map 初始化的大小
#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif

    template<class T>
    struct deque_buf_size
    {
        static constexpr size_t value=sizeof (T)<256?4096/sizeof (T):16;
    };

    // deque 的迭代器设计
    template<class T,class Ref,class Ptr>
    struct deque_iterator:public iterator<random_access_iterator_tag,T>
    {
        typedef deque_iterator<T,T&,T*>         iterator;
        typedef deque_iterator<T,const T&,const T*>         const_iterator;
        typedef deque_iterator              self;


        typedef T            value_type;
        typedef Ptr          pointer;
        typedef Ref          reference;
        typedef size_t       size_type;
        typedef ptrdiff_t    difference_type;
        typedef T*           value_pointer;
        typedef T**         map_pointer;

        static const size_type buffer_size=deque_buf_size<T>::value;

        // 迭代器所含成员数据
        value_pointer cur; //指向所在缓冲区的当前元素
        value_pointer first;//指向所在缓冲区的头部
        value_pointer last;// 指向所在缓冲区的尾部
        map_pointer  node;  // 缓冲区所在节点

        // 构造、复制、移动函数
        deque_iterator() noexcept
                :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

        deque_iterator(value_pointer v, map_pointer n):cur(v),first(*n),last(*n+buffer_size),node(n){}

        deque_iterator(const iterator & rhs):cur(rhs.cur),first(rhs.first),last(rhs.last), node(rhs.node){}

        deque_iterator(iterator && rhs)noexcept:cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node)
        {
            rhs.cur = nullptr;
            rhs.first = nullptr;
            rhs.last = nullptr;
            rhs.node = nullptr;
        }

        deque_iterator(const const_iterator &rhs):cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node){}

        self operator=(const iterator &rhs)
        {
            if(this!=&rhs)
            {
                cur = rhs.cur;
                first = rhs.first;
                last = rhs.last;
                node = rhs.node;
            }
            return *this;
        }

        // 转到另一个缓冲区
        void set_node(map_pointer new_node)
        {
            node=new_node;
            first=new_node;
            last=first+buffer_size;
        }

        // 重载运算符
        reference operator*()const{return *cur;}
        pointer operator->()const{return cur;}

        difference_type operator-(const self& x)const
        {
            return static_cast<difference_type>(buffer_size)*(node-x.node)+(cur-first)-(x.cur-x.first);
        }

        self & operator++()
        {
            ++cur;
            if(cur==last)
            {
                // 如果到达缓冲
                set_node(node+1);
                cur=first;
            }
            return *this;
        }

        self operator++(int )
        {
            self tmp=*this;
            ++*this;
            return tmp;
        }

        self & operator--()
        {
            if(cur==first)
            {
                // 如果到达缓冲区的头
                set_node(node-1);
                cur=last;
            }
            --cur;
            return *this;
        }

        self operator--(int)
        {
            self tmp=*this;
            --*this;
            return tmp;
        }

        self &operator+=(difference_type n)
        {
            const auto offset=n+(cur-first);
            if(offset>=0 && offset<static_cast<difference_type>(buffer_size))
            {
                // 仍在当前缓冲区
                cur+=n;
            } else
            {
                //需要跳转到其他缓冲区
                const auto node_offset=offset>0?offset / static_cast<difference_type>(buffer_size):-static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
                set_node(node+node_offset);
                cur=first+(offset-node_offset*static_cast<difference_type>(buffer_size));
            }
            return *this;
        }

        self operator+(difference_type n)const
        {
            self tmp=*this;
            return tmp+=n;
        }

        self & operator-=(difference_type n)
        {
            return *this+=-n;
        }

        self operator-(difference_type n)const
        {
            self tmp=*this;
            return tmp-=n;
        }

        reference operator[](difference_type n)const{return *(*this+n);}

        // 重载比较操作符
        bool operator==(const self& rhs)const{return cur==rhs.cur;}
        bool operator<(const self& rhs)const
        {
            return node==rhs.node?(cur<rhs.cur):(node<rhs.node);
        }
        bool operator!=(const self& rhs)const{return !(*this==rhs);}
        bool operator> (const self& rhs) const { return rhs < *this; }
        bool operator<=(const self& rhs) const { return !(rhs < *this); }
        bool operator>=(const self& rhs) const { return !(*this < rhs); }

    };

    // 模板类 deque
    // 模板参数代表数据类型
    template<class T>
    class deque
    {
public:
        // deque 的嵌套型别定义
        typedef mystl::allocator<T> allocator_type;
        typedef mystl::allocator<T> data_allocator;
        typedef mystl::allocator<T*> map_allocator;

        typedef typename allocator_type::value_type      value_type;
        typedef typename allocator_type::pointer         pointer;
        typedef typename allocator_type::const_pointer   const_pointer;
        typedef typename allocator_type::reference       reference;
        typedef typename allocator_type::const_reference const_reference;
        typedef typename allocator_type::size_type       size_type;
        typedef typename allocator_type::difference_type difference_type;

        typedef pointer* map_pointer;
        typedef const_pointer* const_map_pointer;

        typedef deque_iterator<T,T&,T*>         iterator;
        typedef deque_iterator<T,const T&,const T*>         const_iterator;
        typedef mystl::reverse_iterator<iterator> reverse_iterator;
        typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

        allocator_type get_allocator()const{return allocator_type();}

        static const size_type buffer_size=deque_buf_size<T>::value;

private:
        //用以下四个数据成员来表现一个deque
        iterator  begin_; // 表现第一个节点
        iterator  end_;   // 表现最后一个节点
        map_pointer map_; // 指向map，map是块连续空间，每个元素都是一个指针，指向一个缓冲区
        size_type map_size_; // map内有多少指针
    public:
        // 构造、复制、移动、析构函数
        deque() { fill_init(0, value_type()); }

        explicit deque(size_type n) { fill_init(n, value_type()); }

        deque(size_type n, const value_type& value) { fill_init(n, value); }

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value, int>::type = 0>
        deque(IIter first, IIter last)
        {
            copy_init(first, last, iterator_category(first));
        }

        deque(std::initializer_list<T> ilist)
        {
            copy_init(ilist.begin(), ilist.end(), mystl::forward_iterator_tag());
        }

        deque(const deque& rhs)
        {
            copy_init(rhs.begin(), rhs.end(), mystl::forward_iterator_tag());
        }

        deque(deque&& rhs) noexcept
                :begin_(mystl::move(rhs.begin_)),
                 end_(mystl::move(rhs.end_)),
                 map_(rhs.map_),
                 map_size_(rhs.map_size_)
        {
            rhs.map_=nullptr;
            rhs.map_size_=0;
        }

        deque& operator=(const deque& rhs)
        {
            if(this!=&rhs)
            {
                const auto len=size();
                if(len>=rhs.size())
                {
                    erase(mystl::copy(rhs.begin(),rhs.end(),begin_),end_);
                } else
                {
                    const_iterator mid=rhs.begin()+static_cast<difference_type>(len);
                    mystl::copy(rhs.begin(),mid,begin_);
                    insert(end_,mid,rhs.end());
                }
            }
            return *this;
        }

        deque& operator=(deque&& rhs) noexcept
        {
            clear();
            begin_=mystl::move(rhs.begin_);
            end_=mystl::move(rhs.end_);
            map_=rhs.map_;
            map_size_=rhs.map_size_;
            rhs.map_=nullptr;
            rhs.map_size_=0;
            return *this;
        }

        deque& operator=(std::initializer_list<T> ilist)
        {
            deque tmp(ilist.begin(),ilist.end());
            swap(tmp);
            return *this;
        }

        ~deque()
        {
            if(map_!=nullptr)
            {
                clear();
                data_allocator::deallocate(*begin_.node,buffer_size);
                *begin_.node=nullptr;
                map_allocator::deallocate(map_,map_size_);
                map_=nullptr;
                map_size_=0;
            }
        }

    public:
        // 迭代器相关操作
        iterator               begin()        { return begin_; }
        const_iterator         begin()  const { return begin_; }
        iterator               end()          { return end_; }
        const_iterator         end()    const { return end_; }

        reverse_iterator       rbegin()       { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
        reverse_iterator       rend()         { return reverse_iterator(begin()); }
        const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }

        const_iterator         cbegin() const { return begin(); }
        const_iterator         cend()   const { return end(); }
        const_reverse_iterator crbegin()const { return rbegin(); }
        const_reverse_iterator crend()  const { return rend(); }

        // 容量相关操作
        bool      empty()    const { return begin_ == end_; }
        size_type size()     const { return end_ - begin_; }
        size_type max_size() const { return static_cast<size_type>(-1); }
        void     resize(size_type new_size) { resize(new_size, value_type()); }
        void     resize(size_type new_size, const value_type& value);
        void    shrink_to_fit()noexcept;

        // 访问元素相关操作
        reference       operator[](size_type n)       {
            MYSTL_DEBUG(n<size());
            return begin_[n]; }

        const_reference operator[](size_type n) const {
            MYSTL_DEBUG(n<size());
            return begin_[n];
        }

        reference       at(size_type n)
        {
            THROW_OUT_OF_RANGE_IF(!(n<size()),"deque<T>::at() subscript out of range");
            return (*this)[n];
        }

        const_reference at(size_type n) const
        {
            THROW_OUT_OF_RANGE_IF(!(n<size()),"deque<T>::at() subscript out of range");
            return (*this)[n];
        }

        reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }

        const_reference front()const
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }

        reference back()
        {
            MYSTL_DEBUG(!empty());
            return *(end()-1);
        }

        const_reference back()const
        {
            MYSTL_DEBUG(!empty());
            return *(end()-1);
        }

        // 修改容器相关操作

        // assign
        void assign(size_type n, const value_type& value)
        {
            fill_assign(n,value);
        }

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value,int>::type=0>
        void assign(IIter first,IIter last)
        {
            copy_assign(first,last,mystl::iterator_category(first));
        }

        void assign(std::initializer_list<T> ilist)
        {
            copy_assign(ilist.begin(),ilist.end(),mystl::forward_iterator_tag());
        }

        // emplace_front 在头部就地构建元素
        template<class ...Args>
        void emplace_front(Args&& ...args);

        // emplace_back 在尾部就地构建元素
        template<class ...Args>
        void emplace_back(Args&& ...args);

        // emplace 在指定位置就地构建元素
        template<class ...Args>
        iterator emplace(const_iterator pos,Args&& ...args);

        // push_front 在头部插入元素
        void push_front(const value_type& value);
        void push_front(value_type&& value)
        {
            emplace_front(mystl::move(value));
        }

        // push_back 在尾部插入元素
        void push_back(const value_type& value);
        void push_back(value_type&& value)
        {
            emplace_back(mystl::move(value));
        }

        // pop_front 删除头部元素
        void pop_front();

        // pop_back 删除尾部元素
        void pop_back();

        // insert 在指定位置插入元素
        iterator insert(iterator pos,const value_type& value);
        iterator insert(iterator pos,value_type&& value);
        void    insert(iterator pos,size_type n,const value_type& value);

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value,int>::type=0>
        void insert(iterator pos,IIter first,IIter last)
        {
            insert_dispatch(pos,first,last,mystl::iterator_category(first));
        }

        //erase 删除指定位置的元素
        iterator erase(iterator pos);
        iterator erase(iterator first,iterator last);

        //clear 清空所有元素
        void clear() noexcept;

        //swap 交换两个deque
        void swap(deque& rhs) noexcept;

    private:
        //helpful function
        // 创建一个map，使其拥有n个节点
        map_pointer create_map(size_type size);
        void        create_buffer(map_pointer nstart, map_pointer nfinish);
        void        destroy_buffer(map_pointer nstart, map_pointer nfinish);

        // 以填充的方式创建节点
        void fill_init(size_type n, const value_type& value);
        void map_init(size_type n);

        template<class IIter>
        void copy_init(IIter first, IIter last, mystl::input_iterator_tag);

        template<class FIter>
        void copy_init(FIter first, FIter last, mystl::forward_iterator_tag);

        // 以填充的方式赋值
        void fill_assign(size_type n, const value_type& value);
        // 以复制的方式赋值
        template<class IIter>
        void copy_assign(IIter first, IIter last, mystl::input_iterator_tag);

        template<class FIter>
        void copy_assign(FIter first, FIter last, mystl::forward_iterator_tag);

        //插入元素
        template<class ...Args>
        iterator insert_aux(iterator pos,Args&& ...args);
        void fill_insert(iterator pos,size_type n,const value_type& value);
        template<class IIter>
        void copy_insert(iterator pos,IIter first,IIter last,size_type n);

        template<class IIter>
        void insert_dispatch(iterator pos,IIter first,IIter last,input_iterator_tag);

        template<class FIter>
        void insert_dispatch(iterator pos,FIter first,FIter last,forward_iterator_tag);

        //reallocate 当map空间不足时，重新配置map空间
        void require_capacity(size_type n, bool front); // n为需要的容量大小
        void reallocate_map_at_front(size_type need); // 在map的前端重新配置空间
        void reallocate_map_at_back(size_type need); // 在map的尾端重新配置空间
        void reallocate_map(size_type need, bool front); // 重新配置map空间

    };

    /*******************************************************/

    //重置容器大小
    template<class T>
    void deque<T>::resize(size_type new_size, const value_type& value)
    {
        const auto len = size();
        if(new_size < len)
            erase(begin()+new_size,end());
        else
            insert(end(),new_size-len,value);
    }

    //减小容器容量
    template<class T>
    void deque<T>::shrink_to_fit()noexcept
    {
        //至少留下头部缓冲区
        for(auto cur=map_;cur<begin_.node;++cur)//释放前端多余的缓冲区
        {
            data_allocator ::deallocate(*cur,buffer_size());
            *cur = nullptr;
        }
        for(auto cur=end_.node +1;cur<=map_+map_size_;++cur)//释放尾端多余的缓冲区
        {
            data_allocator ::deallocate(*cur,buffer_size());
            *cur = nullptr;
        }
    }

    //在头部就地构建元素
    template<class T>
    template<class ...Args>
    void deque<T>::emplace_front(Args &&...args) {
        if(begin_.cur!=begin_.first)//头部缓冲区还有空间
        {
            data_allocator::construct(begin_.cur-1,mystl::forward<Args>(args)...);//在头部缓冲区就地构建元素
            --begin_.cur;//调整头部迭代器
        }
        else//头部缓冲区没有空间
        {
            require_capacity(1,true);//重新配置map空间,传参true表示在头部重新配置，传参1表示需要的容量大小
            try
            {
                --begin_;//调整头部迭代器
                data_allocator::construct(begin_.cur,mystl::forward<Args>(args)...);//在头部缓冲区就地构建元素
            }
            catch (...)
            {
                ++begin_;//构建失败，调整头部迭代器
                //reallocate_map_at_front(1);//重新配置map空间
                throw;
            }
        }
    }

    //在尾部就地构建元素
    template<class T>
    template<class ...Args>
    void deque<T>::emplace_back(Args &&...args)
    {
        if(end_.cur!=end_.last-1)//尾部缓冲区还有空间
        {
            data_allocator::construct(end_.cur,mystl::forward<Args>(args)...);//在尾部缓冲区就地构建元素
            ++end_;//调整尾部迭代器
        }
        else//尾部缓冲区没有空间
        {
            require_capacity(1,false);//重新配置map空间,传参false表示在尾部重新配置，传参1表示需要的容量大小
            data_allocator ::construct(end_.cur,mystl::forward<Args>(args)...);//在尾部缓冲区就地构建元素
            ++end_;
        }
    }

    //在指定位置就地构建元素
    template<class T>
    template<class ...Args>
    typename deque<T>::iterator deque<T>::emplace(const_iterator pos,Args&&...args)
    {
        if(pos.cur==begin_.cur)//在头部就地构建元素
        {
            emplace_front(mystl::forward<Args>(args)...);
            return begin_;
        }
        else if(pos.cur==end_.cur)//在尾部就地构建元素
        {
            emplace_back(mystl::forward<Args>(args)...);
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else//在中间就地构建元素
        {
            return insert_aux(pos,mystl::forward<Args>(args)...);
        }
    }

    //在头部插入元素
    template<class T>
    void deque<T>::push_front(const value_type& value)
    {
        if(begin_.cur!=begin_.first)
        {
            data_allocator::construct(begin_.cur-1,value);//在头部缓冲区就地构建元素
            --begin_.cur;
        }
        else
        {
            require_capacity(1,true);//重新配置map空间.参数意义：true表示在头部重新配置，1表示需要的容量大小
            try
            {
                --begin_;
                data_allocator::construct(begin_.cur,value);
            }
            catch (...)
            {
                ++begin_;
                //reallocate_map_at_front(1);
                throw;
            }
        }
    }

    //在尾部插入元素
    template<class T>
    void deque<T>::push_back(const value_type& value)
    {
        //
        if(end_.cur!=end_.last-1)
        {
            data_allocator::construct(end_.cur,value);//在尾部缓冲区就地构建元素
            ++end_;
        }
        else
        {
            require_capacity(1,false);//重新配置map空间.参数意义：false表示在尾部重新配置，1表示需要的容量大小
            try {
                data_allocator::construct(end_.cur, value);
                ++end_;
            }
            catch (...)
            {
                //reallocate_map_at_back(1);
                throw;
            }
        }
    }

    //弹出头部元素
    template<class T>
    void deque<T>::pop_front()
    {
        MYSTL_DEBUG(!empty());
        if(begin_.cur!=begin_.last-1)//头部缓冲区还有元素
        {
            data_allocator::destroy(begin_.cur);//在头部缓冲区就地销毁元素
            ++begin_.cur;
        }
        else//头部缓冲区没有元素
        {
            data_allocator::destroy(begin_.cur);//在头部缓冲区就地销毁元素
            ++begin_;
            destroy_buffer(begin_.node-1,begin_.node-1);//销毁前端多余的缓冲区
        }
    }

    //弹出尾部元素
    template<class T>
    void deque<T>::pop_back()
    {
        MYSTL_DEBUG(!empty());
        if(end_.cur!=end_.first)//尾部缓冲区还有元素
        {
            --end_.cur;
            data_allocator::destroy(end_.cur);//在尾部缓冲区就地销毁元素
        }
        else//尾部缓冲区没有元素
        {
            --end_;
            data_allocator::destroy(end_.cur);//在尾部缓冲区就地销毁元素
            destroy_buffer(end_.node+1,end_.node+1);//销毁尾端多余的缓冲区
        }
    }

    //在指定位置插入元素
    template<class T>
    typename deque<T>::iterator deque<T>::insert(iterator position,const value_type& value)
    {
        if(position.cur==begin_.cur)//在头部插入元素
        {
            push_front(value);
            return begin_;
        }
        else if(position.cur==end_.cur)//在尾部插入元素
        {
            push_back(value);
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else//在中间插入元素
        {
            return insert_aux(position,value);
        }
    }

    template<class T>
    typename deque<T>::iterator deque<T>::insert(iterator position,value_type&& value)
    {
        if(position.cur==begin_.cur)// 在头部插入元素
        {
            emplace_front(mystl::move(value));
            return begin_;
        }
        else if(position.cur==end_.cur)// 在尾部插入元素
        {
            emplace_back(mystl::move(value));
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else// 在中间插入元素
        {
            return insert_aux(position,mystl::move(value));
        }
    }

    //在指定位置插入n个元素
    template<class T>
    void deque<T>::insert(iterator position,size_type n,const value_type &value)
    {
        if(position.cur==begin_.cur)//在头部插入元素
        {
            require_capacity(n,true);//重新配置map空间.参数意义：true表示在头部重新配置，n表示需要的容量大小
            auto new_begin=begin_-n;
            mystl::uninitialized_fill_n(new_begin,n,value);
            begin_=new_begin;
        }
        else if(position.cur==end_.cur)//在尾部插入元素
        {
            require_capacity(n,false);//重新配置map空间.参数意义：false表示在尾部重新配置，n表示需要的容量大小
            auto new_end=end_+n;
            mystl::uninitialized_fill_n(end_,n,value);
            end_=new_end;
        }
        else//在中间插入元素
        {
            fill_insert(position,n,value);
        }
    }

    //删除position位置的元素
    template<class T>
    typename deque<T>::iterator deque<T>::erase(iterator position)
    {
        MYSTL_DEBUG(position!=end());//position不能是尾后迭代器
        auto next=position;//删除元素后的下一个元素
        ++next;
        const size_type elems_before=position-begin_;//position之前的元素个数
        if(elems_before<(size()/2)) //如果position之前的元素个数小于size()/2
        {
            mystl::copy_backward(begin_,position,next);//将position之前的元素向后移动一个位置
            pop_front();//弹出头部元素
        }
        else//如果position之前的元素个数大于等于size()/2
        {
            mystl::copy(next,end_,position);//将position之后的元素向前移动一个位置
            pop_back();//弹出尾部元素
        }
        return begin_+elems_before;
    }

    //删除[first,last)区间的元素
    template<class T>
    typename deque<T>::iterator
    deque<T>::erase(iterator first,iterator last)
    {
        if(first==begin_ && last==end_)// 删除整个deque
        {
            clear();
            return end_;
        }
        else
        {
            const size_type n=last-first;//要删除的元素个数
            const size_type elems_before=first-begin_;//first之前的元素个数
            if(elems_before<(size()-n)/2)//如果first之前的元素个数小于size()-n的一半
            {
                mystl::copy_backward(begin_,first,last);//将first之前的元素向后移动n个位置
                auto new_begin=begin_+n;
                //destroy_buffer(begin_.node,new_begin.node);//销毁前端多余的缓冲区
                data_allocator ::destory(begin_.cur,new_begin.cur);//销毁前端多余的元素
                begin_=new_begin;
            }
            else//如果first之前的元素个数大于等于size()-n的一半
            {
                mystl::copy(last,end_,first);//将last之后的元素向前移动n个位置
                auto new_end=end_-n;
                //destroy_buffer(new_end.node+1,end_.node+1);//销毁尾端多余的缓冲区
                data_allocator ::destory(new_end.cur,end_.cur);//销毁尾端多余的元素
                end_=new_end;
            }
            return begin_+elems_before;
        }
    }

    //清空
    template<class T>
    void deque<T>::clear()noexcept
    {
        //clear 会保留一个缓冲区
        for(map_pointer node=begin_.node+1;node<end_.node;++node)
        {
            //destroy_buffer(node,node+1);//销毁缓冲区
            data_allocator ::destory(*node,*node+buffer_size);//销毁元素
        }
        if(begin_.node!=end_.node)//如果deque中有两个以上的缓冲区
        {
            //destroy_buffer(begin_.node+1,end_.node+1);//销毁缓冲区
            data_allocator ::destory(begin_.cur,begin_.last);//销毁元素
            data_allocator ::destory(end_.first,end_.cur);//销毁元素
        }
        else//如果deque中只有一个缓冲区
        {
            data_allocator ::destory(begin_.cur,end_.cur);//销毁元素
        }
        shrink_to_fit();//缩小map的大小
        end_=begin_;//调整状态
    }

    //交换两个deque
    template<class T>
    void deque<T>::swap(deque& rhs)noexcept
    {
        if(this!=&rhs)
        {
            mystl::swap(begin_,rhs.begin_);
            mystl::swap(end_,rhs.end_);
            mystl::swap(map_,rhs.map_);
            mystl::swap(map_size_,rhs.map_size_);
        }
    }

    /*********************************************/
    //辅助函数

    //create_map 用来产生并安排好deque的map空间
    template<class T>
    typename deque<T>::map_pointer deque<T>::create_map(size_type size)
    {
        map_pointer mp=nullptr;
        mp=map_allocator::allocate(size);//配置map空间
        for(size_type i=0;i<size;++i)
        {
            *(mp+i)= nullptr;//配置节点空间
        }
        return mp;
    };

    //creatr_buffer 用来产生并安排好deque的缓冲区
    template<class T>
    void deque<T>:: create_buffer(map_pointer nstart, map_pointer nfinish)
    {
        map_pointer cur;
        try
        {
            for(cur=nstart;cur<=nfinish;++cur)
            {
                *cur=data_allocator::allocate(buffer_size);//配置缓冲区
            }
        }
        catch(...)
        {
            while (cur!=nstart)//如果配置过程中出现异常，释放已经配置的缓冲区
            {
                --cur;  //回退
                data_allocator::deallocate(*cur,buffer_size);//释放缓冲区
                *cur= nullptr;//指向空
            }
            throw;//重新抛出异常
        }
    }

    //destroy_buffer 用来释放deque的缓冲区
    template<class T>
    void deque<T>::destroy_buffer(map_pointer nstart, map_pointer nfinish)
    {
        for(map_pointer node=nstart;node<=nfinish;++node)
        {
            data_allocator::deallocate(*node,buffer_size);//释放缓冲区
            *node= nullptr;//指向空
        }
    }

    //map_init 用来初始化map
    template<class T>
    void deque<T>::map_init(size_type nElem)
    {
        //计算需要的节点数（每个节点管理一个缓冲区）
        const size_type nNode=nElem/buffer_size+1;
        map_size_=mystl::max(static_cast<size_type>(DEQUE_MAP_INIT_SIZE), nNode + 2);//map的大小至少为8
        try
        {
            map_=create_map(map_size_);//产生map空间
        }
        catch(...)
        {
            map_= nullptr;//指向空
            map_size_=0;//map大小为0
            throw;//重新抛出异常
        }
        //令nstart和nfinish指向map所拥有的全部节点的最中央区段
        map_pointer nstart=map_+(map_size_-nNode)/2;
        map_pointer nfinish=nstart+nNode-1;
        try
        {
            create_buffer(nstart,nfinish);//产生缓冲区
        }
        catch(...)
        {
            map_allocator::deallocate(map_,map_size_);//释放map空间
            map_= nullptr;//指向空
            map_size_=0;//map大小为0
            throw;//重新抛出异常
        }
        begin_.set_node(nstart);//设定deque的起始缓冲区
        end_.set_node(nfinish);//设定deque的结束缓冲区
        begin_.cur=begin_.first;//设定deque的起始元素
        end_.cur=end_.first+nElem%buffer_size;//设定deque的结束元素
    }

    //fill_init 用来填充deque
    template<class T>
    void deque<T>::fill_init(size_type n, const value_type& value)
    {
        map_init(n);//初始化map
        if(n!=0)
        {
            for(auto cur=begin_.node;cur<end_.node;++cur)
            {
                mystl::uninitialized_fill(*cur,*cur+buffer_size,value);//填充每个缓冲区
            }
            mystl::uninitialized_fill(end_.first,end_.cur,value);//填充最后一个缓冲区
        }
    }

    //copy_init 用来拷贝deque
    template<class T>
    template<class IIter>
    void deque<T>::copy_init(IIter first, IIter last, mystl::input_iterator_tag)
    {
        const size_type n = mystl::distance(first, last);//计算需要拷贝的元素个数
        map_init(n);//初始化map
        for(;first!=last;++first)
        {
            emplace_back(*first);//将元素一个个插入到deque中
        }
    }

    template<class T>
    template<class FIter>
    void deque<T>::copy_init(FIter first,FIter last,forward_iterator_tag)
    {
        const size_type n=mystl::distance(first,last);//计算需要拷贝的元素个数
        map_init(n);//初始化map
        for(auto cur=begin_.node;cur<end_.node;++cur)
        {
            auto next=first;
            mystl::advance(next,buffer_size);//计算每个缓冲区需要拷贝的元素个数
            mystl::uninitialized_copy(first,next,*cur);//拷贝元素
            first=next;//调整first
        }
        mystl::uninitialized_copy(first,last,end_.first);//拷贝最后一个缓冲区的元素
    }

    //fill_assign 用来填充deque
    template<class T>
    void deque<T>::fill_assign(size_type n, const value_type& value)
    {
        if(n>size())
        {
            mystl::fill(begin(),end(),value);//填充已有元素
            insert(end(),n-size(),value);//插入新元素
        }
        else
        {
            erase(begin()+n,end());//擦除多余元素
            mystl::fill(begin(),end(),value);//填充已有元素
        }
    }

    //copy_assign 用来拷贝deque
    template<class T>
    template<class IIter>
    void deque<T>::copy_assign(IIter first,IIter last,input_iterator_tag)
    {
        auto first1=begin();
        auto last1=end();
        for(;first!=last && first1!= last1;++first,++first1)
        {
            *first1=*first;//拷贝元素
        }
        if(first1!=last1)
        {
            erase(first1,last1);//擦除多余元素
        }
        else
        {
            insert_dispatch(end(),first,last,input_iterator_tag());//插入新元素
        }
    }

    template<class T>
    template<class FIter>
    void deque<T>::copy_assign(FIter first,FIter last,forward_iterator_tag)
    {
        const size_type len=mystl::distance(first,last);
        if(len>size())
        {
            auto mid=first;
            mystl::advance(mid,size());//找到中间位置
            mystl::copy(first,mid,begin());//拷贝前半段
            insert_dispatch(end(),mid,last,forward_iterator_tag());//插入后半段
        }
        else
        {
            erase(mystl::copy(first,last,begin()),end());//拷贝并擦除多余元素
        }
    }

    //insert_aux 函数 用来在指定位置插入元素
    template<class T>
    template<class ...Args>
    typename deque<T>::iterator
    deque<T>::insert_aux(iterator position,Args&&...args)
    {
        const size_type elems_before=position-begin();//插入点之前的元素个数
        value_type value_copy=value_type(mystl::forward<Args>(args)...);//新元素的内容
        if(elems_before< size()/2)//如果插入点之前的元素个数比较少
        {
            emplace_front(front());//在最前端插入与第一元素同值的元素
            auto front1=begin();
            ++front1;
            auto front2=front1;
            ++front2;
            position=begin()+elems_before;
            auto pos1=position;
            ++pos1;
            mystl::copy(front2,pos1,front1);//将插入点之前的元素向前移动一个位置
        }
        else//如果插入点之后的元素个数比较少
        {
            emplace_back(back());//在最尾端插入与最后元素同值的元素
            auto back1=end();
            --back1;
            auto back2=back1;
            --back2;
            position=begin()+elems_before;
            mystl::copy_backward(position,back2,back1);//将插入点之后的元素向后移动一个位置
        }
        *position=value_copy;//在插入点上设定新值
        return position;
    }

    //fill_insert 函数 用来在指定位置插入n个元素
    template<class T>
    void deque<T>::fill_insert(iterator position,size_type n,const value_type& value)
    {
        const size_type elems_before=position-begin_;//插入点之前的元素个数
        const size_type length=size();//deque的长度
        auto value_copy=value;//拷贝一份新值
        if(elems_before<(length/2))
        {
            //如果插入点之前的元素个数比较少
            require_capacity(n, true);//检查是否需要扩容,参数为true表示在前端扩容
            //原来的迭代器可能失效，需要重新定位
            auto old_begin=begin_;//记录原始的begin
            auto new_begin=begin_-n;//计算新的起始点
            position=begin_+elems_before;//重新定位插入点
            try
            {
                if(elems_before>=n)
                {
                    //插入点之前的元素个数大于等于新增元素个数
                    auto begin_n=begin_+n;
                    mystl::uninitialized_copy(begin_,begin_n,new_begin);//将插入点之前的元素拷贝到新的起始点
                    begin_=new_begin;//设定deque的起始点
                    mystl::copy(begin_n,position,old_begin);//将插入点之后的元素拷贝到原始的起始点
                    mystl::fill(position,begin_n,value_copy);//填充新增元素
                }
                else
                {
                    //插入点之前的元素个数小于新增元素个数
                    mystl::uninitialized_fill(
                            mystl::uninitialized_copy(begin_, position, new_begin), begin_, value_copy);//填充新增元素
                    begin_ = new_begin;//设定deque的起始点
                    mystl::fill(old_begin, position, value_copy);//填充原始起始点到插入点的元素
                }
            }
            catch (...)
            {
                if (new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else
        {
            require_capacity(n, false);
            // 原来的迭代器可能会失效
            auto old_end = end_;
            auto new_end = end_ + n;
            const size_type elems_after = length - elems_before;
            position = end_ - elems_after;
            try
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    mystl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    mystl::copy_backward(position, end_n, old_end);
                    mystl::fill(position, position + n, value_copy);
                }
                else
                {
                    mystl::uninitialized_fill(end_, position + n, value_copy);
                    mystl::uninitialized_copy(position, end_, position + n);
                    end_ = new_end;
                    mystl::fill(position, old_end, value_copy);
                }
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
    }

    //copy_insert 函数 用来在指定位置插入[first,last)区间的元素
    template<class T>
    template <class FIter>
    void deque<T>::
    copy_insert(iterator position, FIter first, FIter last, size_type n)
    {
        const size_type elems_before = position - begin_;
        auto len = size();
        if (elems_before < (len / 2))
        {
            require_capacity(n, true);
            // 原来的迭代器可能会失效
            auto old_begin = begin_;
            auto new_begin = begin_ - n;
            position = begin_ + elems_before;
            try
            {
                if (elems_before >= n)
                {
                    auto begin_n = begin_ + n;
                    mystl::uninitialized_copy(begin_, begin_n, new_begin);
                    begin_ = new_begin;
                    mystl::copy(begin_n, position, old_begin);
                    mystl::copy(first, last, position - n);
                }
                else
                {
                    auto mid = first;
                    mystl::advance(mid, n - elems_before);
                    mystl::uninitialized_copy(first, mid,
                                              mystl::uninitialized_copy(begin_, position, new_begin));
                    begin_ = new_begin;
                    mystl::copy(mid, last, old_begin);
                }
            }
            catch (...)
            {
                if(new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else
        {
            require_capacity(n, false);
            // 原来的迭代器可能会失效
            auto old_end = end_;
            auto new_end = end_ + n;
            const auto elems_after = len - elems_before;
            position = end_ - elems_after;
            try
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    mystl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    mystl::copy_backward(position, end_n, old_end);
                    mystl::copy(first, last, position);
                }
                else
                {
                    auto mid = first;
                    mystl::advance(mid, elems_after);
                    mystl::uninitialized_copy(position, end_,
                                              mystl::uninitialized_copy(mid, last, end_));
                    end_ = new_end;
                    mystl::copy(first, mid, position);
                }
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
    }

    // insert_dispatch 函数
    template <class T>
    template <class IIter>
    void deque<T>::
    insert_dispatch(iterator position, IIter first, IIter last, input_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = mystl::distance(first, last);
        const size_type elems_before = position - begin_;
        if (elems_before < (size() / 2))
        {
            require_capacity(n, true);
        }
        else
        {
            require_capacity(n, false);
        }
        position = begin_ + elems_before;
        auto cur = --last;//cur指向最后一个元素
        for (size_type i = 0; i < n; ++i, --cur)//从后往前插入
        {                                //防止插入的时候迭代器失效
            insert(position, *cur);//调用insert函数
        }
    }

    template <class T>
    template <class FIter>
    void deque<T>::
    insert_dispatch(iterator position, FIter first, FIter last, forward_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = mystl::distance(first, last);
        if (position.cur == begin_.cur) //在头部插入
        {
            require_capacity(n, true);
            auto new_begin = begin_ - n;
            try
            {
                mystl::uninitialized_copy(first, last, new_begin);
                begin_ = new_begin;
            }
            catch (...)
            {
                if(new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else if (position.cur == end_.cur)
        {
            require_capacity(n, false);
            auto new_end = end_ + n;
            try
            {
                mystl::uninitialized_copy(first, last, end_);
                end_ = new_end;
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
        else
        {
            copy_insert(position, first, last, n);
        }
    }

    // require_capacity 函数 用来判断是否需要扩容
    template <class T>
    void deque<T>::require_capacity(size_type n, bool front)
    {
        if (front && (static_cast<size_type>(begin_.cur - begin_.first) < n))
        {   //在头部插入
            const size_type need_buffer = (n - (begin_.cur - begin_.first)) / buffer_size + 1;
            if (need_buffer > static_cast<size_type>(begin_.node - map_))
            {
                reallocate_map_at_front(need_buffer);
                return;
            }
            create_buffer(begin_.node - need_buffer, begin_.node - 1);
        }
        else if (!front && (static_cast<size_type>(end_.last - end_.cur - 1) < n))
        {
            const size_type need_buffer = (n - (end_.last - end_.cur - 1)) / buffer_size + 1;
            if (need_buffer > static_cast<size_type>((map_ + map_size_) - end_.node - 1))
            {
                reallocate_map_at_back(need_buffer);
                return;
            }
            create_buffer(end_.node + 1, end_.node + need_buffer);
        }
    }

    // reallocate_map_at_front 函数 用来在头部扩容

    template <class T>
    void deque<T>::reallocate_map_at_front(size_type need_buffer)
    {
        const size_type new_map_size = mystl::max(map_size_ << 1,
                                                  map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        // 另新的 map 中的指针指向原来的 buffer，并开辟新的 buffer
        auto begin = new_map + (new_map_size - new_buffer) / 2;
        auto mid = begin + need_buffer;
        auto end = mid + old_buffer;
        create_buffer(begin, mid - 1);
        for (auto begin1 = mid, begin2 = begin_.node; begin1 != end; ++begin1, ++begin2)
            *begin1 = *begin2;

        // 更新数据
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        begin_ = iterator(*mid + (begin_.cur - begin_.first), mid);
        end_ = iterator(*(end - 1) + (end_.cur - end_.first), end - 1);
    }

    // reallocate_map_at_back 函数 用来在尾部扩容

    template <class T>
    void deque<T>::reallocate_map_at_back(size_type need_buffer)
    {
        const size_type new_map_size = mystl::max(map_size_ << 1,
                                                  map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        // 另新的 map 中的指针指向原来的 buffer，并开辟新的 buffer
        auto begin = new_map + ((new_map_size - new_buffer) / 2);
        auto mid = begin + old_buffer;
        auto end = mid + need_buffer;
        for (auto begin1 = begin, begin2 = begin_.node; begin1 != mid; ++begin1, ++begin2)
            *begin1 = *begin2;
        create_buffer(mid, end - 1);

        // 更新数据
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        begin_ = iterator(*begin + (begin_.cur - begin_.first), begin);
        end_ = iterator(*(mid - 1) + (end_.cur - end_.first), mid - 1);
    }

    // 重载比较操作符
    template <class T>
    bool operator==(const deque<T>& lhs, const deque<T>& rhs)
    {
        return lhs.size() == rhs.size() &&
               mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <class T>
    bool operator<(const deque<T>& lhs, const deque<T>& rhs)
    {
        return mystl::lexicographical_compare(lhs.begin(), lhs.end(),
                                              rhs.begin(), rhs.end());
    }

    template <class T>
    bool operator!=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    bool operator>(const deque<T>& lhs, const deque<T>& rhs)
    {
        return rhs < lhs;
    }

    template <class T>
    bool operator<=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class T>
    bool operator>=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs < rhs);
    }

    // 重载 mystl 的 swap
    template <class T>
    void swap(deque<T>& lhs, deque<T>& rhs)
    {
        lhs.swap(rhs);
    }

}
#endif //STL_STUDY_DEQUE_H
