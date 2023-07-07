// ���ͷ�ļ������������� construct��destroy
// construct : �������Ĺ���
// destroy   : ������������

#ifndef STL_STUDY_CONSTRUCT_H
#define STL_STUDY_CONSTRUCT_H

#include "type_traits.h"
#include "iterator.h"
#include "util.h"
//����ĳЩ����
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // unused parameter
#endif // _MSC_VER

namespace mystl
{
    //construct �������
    template<class Ty>
    void construct(Ty* ptr)
    {
        ::new((void*)ptr) Ty();
    }

    template<class Ty1,class Ty2>
    void construct(Ty1* ptr,const Ty2& value)
    {
        ::new((void*)ptr) Ty1(value);
    }

    template<class Ty,class ...Args>
    void construct(Ty* ptr,Args&&...args)
    {
        ::new((void*)ptr) Ty(mystl::forward<Args>(args)...);
    }

    //destory ����
    template<class Ty>
    void destory_one(Ty*,std::true_type){}

    template <class Ty>
    void destroy_one(Ty* pointer, std::false_type)
    {
        if(pointer!= nullptr)
        {
            pointer->~Ty();
        }
    }

    template <class ForwardIter>
    void destroy_cat(ForwardIter , ForwardIter , std::true_type) {}

    template<class ForwardIter >
    void destory_cat(ForwardIter first,ForwardIter last,std::false_type)
    {
        for(;first!=last;++first)
        {
            destory(&*first);
        }
    }

    template<class Ty>
    void destory(Ty* pointer)
    {
        destory_one(pointer,std::is_trivially_destructible<Ty>{});
    }

    template<class ForwardIter>
    void destory(ForwardIter first,ForwardIter last)
    {
        destory_cat(first,last,std::is_trivially_destructible<
                typename iterator_traits<ForwardIter>::value_type>{});
    }

}


#ifdef _MSC_VER
#pragma warning (pop)
#endif



#endif //STL_STUDY_CONSTRUCT_H