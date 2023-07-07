# stl_study
动手实现STL



学习参考[Alinshans/MyTinySTL: Achieve a tiny STL in C++11 (github.com)](https://github.com/Alinshans/MyTinySTL)

在每一个文件头表明有些函数的逻辑没有学懂。

对于使用没有特别大的练习，主要学习标准库怎么实现经典的数据结构和相关算法，有助于模板练习。



红黑树rb_tree.h 200行之后没有学习懂，待看视频详细学习。

map/set 容器的插入删除以及红黑树自动调整都涉及rb_tree.h中的内容，主要注意map/set 与 multimap/multiset使用的插入等函数不一样，是否允许红黑树中的节点拥有相同的键值，很重要。想理解内部结构，需要从红黑树学。

- unordered_map
- unordered_set
- 底层使用hash结构，hashtable.h 底层逻辑实现复杂，没有学通。需要重新学习。遵照着unordered_map里的相关功能的函数调用去学习，例如，声明类型，构造函数，迭代器相关，容器增删改查等，以及哈希策略，重载比较运算符。

算法头文件 algo.h未学习完，有关于堆的知识的heap_algo.h,未学习完



文件中可能有些许bug无法run，参考源项目测试。
