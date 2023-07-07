//
// Created by 11048 on 2023/6/15.
//
// 这个头文件包含了两个模板类 map 和 multimap
// map      : 映射，元素具有键值和实值，会根据键值大小自动排序，键值不允许重复
// multimap : 映射，元素具有键值和实值，会根据键值大小自动排序，键值允许重复

// notes:
//
// 异常保证：
// mystl::map<Key, T> / mystl::multimap<Key, T> 满足基本异常保证，对以下等函数做强异常安全保证：
//   * emplace
//   * emplace_hint
//   * insert
#ifndef STL_STUDY_MAP_H
#define STL_STUDY_MAP_H

#include "rb_tree.h"
namespace mystl
{
    // 模板类 map，键值不允许重复
// 参数一代表键值类型，参数二代表实值类型，参数三代表键值的比较方式，缺省使用 mystl::less

    template<class Key,class T,class Compare=mystl::less<Key>>
    class map{
    public:
        //map 的型别定义
        typedef Key key_type;
        //typedef T                                       data_type;
        typedef T mapped_type;
        typedef mystl::pair<const Key, T> value_type;
        typedef Compare key_compare;

        // 定义一个 functor，其接受两个 value_type，返回 bool,进行元素比较
        class value_compare : public mystl::binary_function<value_type, value_type, bool> {
            friend class map<Key, T, Compare>; // map 可以直接访问 value_compare 的成员
        private:
            Compare comp;

            value_compare(Compare c) : comp(c) {}

        public:
            bool operator()(const value_type &lhs, const value_type &rhs) const {
                return comp(lhs.first, rhs.first); // 只比较键值
            }
        };

    private:
        // 以 mystl::rb_tree 作为底层机制
        typedef mystl::rb_tree<value_type, key_compare> base_type; // 底层 rb_tree 容器
        base_type tree_; // 底层 rb_tree 容器

    public:
        // 使用 rb_tree 的型别
        typedef typename base_type::node_type node_type; // 用于构造插入节点
        typedef typename base_type::pointer pointer;
        typedef typename base_type::const_pointer const_pointer;
        typedef typename base_type::reference              reference;
        typedef typename base_type::const_reference        const_reference;
        typedef typename base_type::iterator               iterator;
        typedef typename base_type::const_iterator         const_iterator;
        typedef typename base_type::reverse_iterator       reverse_iterator;
        typedef typename base_type::const_reverse_iterator const_reverse_iterator;
        typedef typename base_type::size_type              size_type;
        typedef typename base_type::difference_type        difference_type;
        typedef typename base_type::allocator_type         allocator_type;

    public:
        // 构造、复制、移动、析构函数
        map()=default;

        template<class InputIterator>
                map(InputIterator first,InputIterator last):tree_(){
            tree_.insert_unique(first,last);
        }

        map(std::initializer_list<value_type> ilist):tree_(){
            tree_.insert_unique(ilist.begin(),ilist.end());
        }

        map(const map& rhs):tree_(rhs.tree_){}

        map(map&& rhs)noexcept:tree_(mystl::move(rhs.tree_)){}

        map& operator=(const map& rhs){
            tree_=rhs.tree_;
            return *this;
        }

        map& operator=(map&& rhs)noexcept{
            tree_=mystl::move(rhs.tree_);
            return *this;
        }

        map& operator=(std::initializer_list<value_type> ilist){
            tree_.clear();
            tree_.insert_unique(ilist.begin(),ilist.end());
            return *this;
        }

        // 相关接口
        key_compare key_comp()const{return tree_.key_comp();} // 返回键值比较规则
        value_compare value_comp()const{return value_compare(tree_.key_comp());} // 返回元素比较规则
        allocator_type get_allocator()const{return tree_.get_allocator();} // 返回配置器

        // 迭代器相关
        iterator begin()noexcept{return tree_.begin();}
        const_iterator begin()const noexcept{return tree_.begin();}
        iterator end()noexcept{return tree_.end();}
        const_iterator end()const noexcept{return tree_.end();}

        reverse_iterator rbegin()noexcept{return tree_.rbegin();}
        const_reverse_iterator rbegin()const noexcept{return tree_.rbegin();}
        reverse_iterator rend()noexcept{return tree_.rend();}
        const_reverse_iterator rend()const noexcept{return tree_.rend();}

        const_iterator cbegin()const noexcept{return tree_.cbegin();}
        const_iterator cend()const noexcept{return tree_.cend();}
        const_reverse_iterator crbegin()const noexcept{return tree_.crbegin();}
        const_reverse_iterator crend()const noexcept{return tree_.crend();}

        // 容量相关
        bool empty()const noexcept{return tree_.empty();}
        size_type size()const noexcept{return tree_.size();}
        size_type max_size()const noexcept{return tree_.max_size();}

        // 访问元素相关

        //若键值不存在，at() 会抛出 std::out_of_range 异常
        mapped_type& at(const key_type& key)
        {
            iterator it =lower_bound(key);
            //it->first 为大于等于 key 的第一个元素
            THROW_OUT_OF_RANGE_IF(it==end()||key_comp()(key,it->first),"map<Key,T> at() subscript out of range");
            return it->second;
        }

        mapped_type & operator[](const key_type& key)
        {
            iterator it =lower_bound(key);
            //it->first 为大于等于 key 的第一个元素
            if(it==end()||key_comp()(key,it->first))
                it=emplace_hint(it,key,T{}); // 以 key 为键值，T{} 为元素值构造节点并插入
            return it->second;
        }

        mapped_type & operator[](key_type&& key)
        {
            iterator it =lower_bound(key);
            //it->first 为大于等于 key 的第一个元素
            if(it==end()||key_comp()(key,it->first))
                it=emplace_hint(it,mystl::move(key),T{}); // 以 key 为键值，T{} 为元素值构造节点并插入
            return it->second;
        }


        // 插入删除相关
        template<class ...Args>
        pair<iterator,bool> emplace(Args&& ...args) // 以 args 构造元素并插入,返回值为迭代器和是否插入成功
        {
            return tree_.emplace_unique(mystl::forward<Args>(args)...);
        }

        template<class ...Args>
        iterator emplace_hint(const_iterator hint,Args&& ...args) // 以 args 构造元素并插入,返回值为迭代器和是否插入成功
        {
            return tree_.emplace_unique_use_hint(hint,mystl::forward<Args>(args)...);
        }

        pair<iterator,bool> insert(const value_type& value) // 插入元素
        {
            return tree_.insert_unique(value);
        }

        pair<iterator,bool> insert(value_type&& value) // 插入元素
        {
            return tree_.insert_unique(mystl::move(value));
        }

        iterator insert(iterator hint, const value_type& value)
        {
            return tree_.insert_unique(hint, value);
        }

        iterator insert(iterator hint, value_type&& value)
        {
            return tree_.insert_unique(hint, mystl::move(value));
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            tree_.insert_unique(first, last);
        }

        void      erase(iterator position)             { tree_.erase(position); }
        size_type erase(const key_type& key)           { return tree_.erase_unique(key); }
        void      erase(iterator first, iterator last) { tree_.erase(first, last); }

        void      clear()                              { tree_.clear(); }


        //map 相关操作
        iterator find(const key_type& key){return tree_.find(key);} // 返回键值为 key 的元素的迭代器
        const_iterator find(const key_type& key)const{return tree_.find(key);}

        iterator lower_bound(const key_type& key){return tree_.lower_bound(key);} // 返回键值大于等于 key 的第一个元素的迭代器
        const_iterator lower_bound(const key_type& key)const{return tree_.lower_bound(key);}

        size_type count(const key_type& key)const{return tree_.count(key);} // 返回键值等于 key 的元素个数

        iterator upper_bound(const key_type& key){return tree_.upper_bound(key);} // 返回键值不小于 key 的最后一个元素的迭代器
        const_iterator upper_bound(const key_type& key)const{return tree_.upper_bound(key);}

        pair<iterator,iterator>
        equal_range(const key_type& key){return tree_.equal_range_unique(key);} // 返回键值等于 key 的元素的迭代器对

        pair<const_iterator,const_iterator>
        equal_range(const key_type& key)const{return tree_.equal_range_unique(key);}

        void swap(map& rhs) noexcept
        {
            tree_.swap(rhs.tree_);
        }
    public:
        friend bool operator==(const map& lhs,const map& rhs)
        {
            return lhs.tree_==rhs.tree_;
        }
        friend bool operator<(const map& lhs,const map& rhs)
        {
            return lhs.tree_<rhs.tree_;
        }

    };

    // 重载比较操作符
    template <class Key, class T, class Compare>
    bool operator==(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return lhs == rhs;
    }

    template <class Key, class T, class Compare>
    bool operator<(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return lhs < rhs;
    }

    template <class Key, class T, class Compare>
    bool operator!=(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class Key, class T, class Compare>
    bool operator>(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return rhs < lhs;
    }

    template <class Key, class T, class Compare>
    bool operator<=(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class Key, class T, class Compare>
    bool operator>=(const map<Key, T, Compare>& lhs, const map<Key, T, Compare>& rhs)
    {
        return !(lhs < rhs);
    }

    // 重载 mystl 的 swap
    template <class Key, class T, class Compare>
    void swap(map<Key, T, Compare>& lhs, map<Key, T, Compare>& rhs) noexcept {
        lhs.swap(rhs);
    }

// 模板类 multimap，键值允许重复
// 参数一代表键值类型，参数二代表实值类型，参数三代表键值的比较方式，缺省使用 mystl::less
    template <class Key, class T, class Compare = mystl::less<Key>>
    class multimap
    {
    public:
        // multimap 的型别定义
        typedef Key                        key_type;
        typedef T                          mapped_type;
        typedef mystl::pair<const Key, T>  value_type;
        typedef Compare                    key_compare;

        // 定义一个 functor，用来进行元素比较
        class value_compare : public binary_function <value_type, value_type, bool>
        {
            friend class multimap<Key, T, Compare>;
        private:
            Compare comp;
            value_compare(Compare c) : comp(c) {}
        public:
            bool operator()(const value_type& lhs, const value_type& rhs) const
            {
                return comp(lhs.first, rhs.first);
            }
        };

    private:
        // 用 mystl::rb_tree 作为底层机制
        typedef mystl::rb_tree<value_type, key_compare>  base_type;
        base_type tree_;

    public:
        // 使用 rb_tree 的型别
        typedef typename base_type::node_type              node_type;
        typedef typename base_type::pointer                pointer;
        typedef typename base_type::const_pointer          const_pointer;
        typedef typename base_type::reference              reference;
        typedef typename base_type::const_reference        const_reference;
        typedef typename base_type::iterator               iterator;
        typedef typename base_type::const_iterator         const_iterator;
        typedef typename base_type::reverse_iterator       reverse_iterator;
        typedef typename base_type::const_reverse_iterator const_reverse_iterator;
        typedef typename base_type::size_type              size_type;
        typedef typename base_type::difference_type        difference_type;
        typedef typename base_type::allocator_type         allocator_type;

    public:
        // 构造、复制、移动函数

        multimap() = default;

        template <class InputIterator>
        multimap(InputIterator first, InputIterator last)
                :tree_()
        { tree_.insert_multi(first, last); }
        multimap(std::initializer_list<value_type> ilist)
                :tree_()
        { tree_.insert_multi(ilist.begin(), ilist.end()); }

        multimap(const multimap& rhs)
                :tree_(rhs.tree_)
        {
        }
        multimap(multimap&& rhs) noexcept
                :tree_(mystl::move(rhs.tree_))
        {
        }

        multimap& operator=(const multimap& rhs)
        {
            tree_ = rhs.tree_;
            return *this;
        }
        multimap& operator=(multimap&& rhs)
        {
            tree_ = mystl::move(rhs.tree_);
            return *this;
        }

        multimap& operator=(std::initializer_list<value_type> ilist)
        {
            tree_.clear();
            tree_.insert_multi(ilist.begin(), ilist.end());
            return *this;
        }

        // 相关接口

        key_compare            key_comp()      const { return tree_.key_comp(); }
        value_compare          value_comp()    const { return value_compare(tree_.key_comp()); }
        allocator_type         get_allocator() const { return tree_.get_allocator(); }

        // 迭代器相关

        iterator               begin()         noexcept
        { return tree_.begin(); }
        const_iterator         begin()   const noexcept
        { return tree_.begin(); }
        iterator               end()           noexcept
        { return tree_.end(); }
        const_iterator         end()     const noexcept
        { return tree_.end(); }

        reverse_iterator       rbegin()        noexcept
        { return reverse_iterator(end()); }
        const_reverse_iterator rbegin()  const noexcept
        { return const_reverse_iterator(end()); }
        reverse_iterator       rend()          noexcept
        { return reverse_iterator(begin()); }
        const_reverse_iterator rend()    const noexcept
        { return const_reverse_iterator(begin()); }

        const_iterator         cbegin()  const noexcept
        { return begin(); }
        const_iterator         cend()    const noexcept
        { return end(); }
        const_reverse_iterator crbegin() const noexcept
        { return rbegin(); }
        const_reverse_iterator crend()   const noexcept
        { return rend(); }

        // 容量相关
        bool                   empty()    const noexcept { return tree_.empty(); }
        size_type              size()     const noexcept { return tree_.size(); }
        size_type              max_size() const noexcept { return tree_.max_size(); }

        // 插入删除操作

        template <class ...Args>
        iterator emplace(Args&& ...args)
        {
            return tree_.emplace_multi(mystl::forward<Args>(args)...);
        }

        template <class ...Args>
        iterator emplace_hint(iterator hint, Args&& ...args)
        {
            return tree_.emplace_multi_use_hint(hint, mystl::forward<Args>(args)...);
        }

        iterator insert(const value_type& value)
        {
            return tree_.insert_multi(value);
        }
        iterator insert(value_type&& value)
        {
            return tree_.insert_multi(mystl::move(value));
        }

        iterator insert(iterator hint, const value_type& value)
        {
            return tree_.insert_multi(hint, value);
        }
        iterator insert(iterator hint, value_type&& value)
        {
            return tree_.insert_multi(hint, mystl::move(value));
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            tree_.insert_multi(first, last);
        }

        void           erase(iterator position)             { tree_.erase(position); }
        size_type      erase(const key_type& key)           { return tree_.erase_multi(key); }
        void           erase(iterator first, iterator last) { tree_.erase(first, last); }

        void           clear() { tree_.clear(); }

        // multimap 相关操作

        iterator       find(const key_type& key)              { return tree_.find(key); }
        const_iterator find(const key_type& key)        const { return tree_.find(key); }

        size_type      count(const key_type& key)       const { return tree_.count_multi(key); }

        iterator       lower_bound(const key_type& key)       { return tree_.lower_bound(key); }
        const_iterator lower_bound(const key_type& key) const { return tree_.lower_bound(key); }

        iterator       upper_bound(const key_type& key)       { return tree_.upper_bound(key); }
        const_iterator upper_bound(const key_type& key) const { return tree_.upper_bound(key); }

        pair<iterator, iterator>
        equal_range(const key_type& key)
        { return tree_.equal_range_multi(key); }

        pair<const_iterator, const_iterator>
        equal_range(const key_type& key) const
        { return tree_.equal_range_multi(key); }

        void swap(multimap& rhs) noexcept
        { tree_.swap(rhs.tree_); }

    public:
        friend bool operator==(const multimap& lhs, const multimap& rhs) { return lhs.tree_ == rhs.tree_; }
        friend bool operator< (const multimap& lhs, const multimap& rhs) { return lhs.tree_ <  rhs.tree_; }
    };

// 重载比较操作符
    template <class Key, class T, class Compare>
    bool operator==(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return lhs == rhs;
    }

    template <class Key, class T, class Compare>
    bool operator<(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return lhs < rhs;
    }

    template <class Key, class T, class Compare>
    bool operator!=(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class Key, class T, class Compare>
    bool operator>(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return rhs < lhs;
    }

    template <class Key, class T, class Compare>
    bool operator<=(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class Key, class T, class Compare>
    bool operator>=(const multimap<Key, T, Compare>& lhs, const multimap<Key, T, Compare>& rhs)
    {
        return !(lhs < rhs);
    }

// 重载 mystl 的 swap
    template <class Key, class T, class Compare>
    void swap(multimap<Key, T, Compare>& lhs, multimap<Key, T, Compare>& rhs) noexcept
    {
        lhs.swap(rhs);
    }



}
#endif //STL_STUDY_MAP_H
