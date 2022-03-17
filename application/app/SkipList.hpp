#include <tuple>
#include <vector>
#include <limits>
#include <iostream>
#include <random>
#include <initializer_list>
#include <assert.h>
#include <optional>

#define P(x) \
    {        \
        x, x \
    }
#define OUT(x, y)                                 \
    {                                             \
        std::cout << x << ": " << y << std::endl; \
    }

template <typename K, typename V>
struct _Pair
{
    K key;
    V value;
};

//跳表节点
//内存释放由SkipList负责
template <typename K, typename V>
struct SkipNode
{
    using Pair = _Pair<K, V>;
    using Node = SkipNode<K, V>;

    SkipNode(){};
    SkipNode(const Pair &p) : elem(p){};
    SkipNode(Pair &&p) : elem(p){};
    //拷贝构造不会复制next数组
    SkipNode(const Node &node) : elem(node.elem){};
    //移动构造不会移动next数组
    SkipNode(Node &&node) : elem(node.elem){};

    // 存放键值对
    Pair elem;
    // 绑定键值对的关键字，访问更方便
    K &key = elem.key;
    // 存放在i级链表中下一个元素的位置。next::size()即为该节点的级数。由于动态分配，不再需要给跳表maxLevel的约束。
    std::vector<Node *> next{};
};

template <typename K, typename V>
class SkipList
{
    using Pair = _Pair<K, V>;
    using Node = SkipNode<K, V>;

//-------------------------------------------------------------------------------迭代器---------------------------------------------------------------
public:
    class iterator
    {
        using Pair = _Pair<K, V>;
        using Node = SkipNode<K, V>;

    private:
        int _level = 0;
        Node *_node = nullptr;

    public:
        iterator(Node *node, int level = 0) : _level(level), _node(node){};
        iterator(const iterator &it) : _level(it._level), _node(it._node){};

    public:
        inline Pair &operator*() const
        {
            return _node->elem;
        }
        inline Pair *operator->() const
        {
            return &(_node->elem);
        }
        inline bool operator==(const iterator &it) const
        {
            return _node == it._node;
        }
        inline bool operator!=(const iterator &it) const
        {
            return _node != it._node;
        }
        inline iterator &operator++()
        {
            _node = _node->next[_level];
            return *this;
        }
        inline iterator operator++(int)
        {
            iterator temp(*this);
            _node = _node->next[_level];
            return temp;
        }
    };

public:
    inline iterator begin(int level = 0) const
    {
        return iterator(_head->next[level], level);
    }
    inline iterator end() const
    {
        return iterator(_tail, 0);
    }


//-------------------------------------------------------------------------------控制台输出---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
private:
    void _unformatted_print()
    {
        std::cout << "-------------------------------------------------" << std::endl;
        for (int curLevel = _levels - 1; curLevel >= 0; curLevel--)
        {
            std::cout << "Level " << curLevel << ": ";
            Node *curNode = _head->next[curLevel];
            while (curNode != _tail)
            {
                auto &[key, value] = curNode->elem;

                std::cout << key << '\t';
                curNode = curNode->next[curLevel];
            }
            std::cout << std::endl;
        }
        std::cout << "-------------------------------------------------" << std::endl;
    }
    void _formatted_print()
    {
        std::cout << "-------------------------------------------------" << std::endl;
        //是否整齐打印
        K *temp = new K[_size];
        int idx = 0;
        Node *p = _head->next[0];
        while (p != _tail)
        {
            temp[idx++] = p->elem.key;
            p = p->next[0];
        }
        idx = 0;
        for (int curLevel = _levels - 1; curLevel >= 0; curLevel--)
        {
            std::cout << "Level " << curLevel << ": ";
            Node *curNode = _head->next[curLevel];
            while (curNode != _tail)
            {
                auto &[key, value] = curNode->elem;
                while (key != temp[idx++])
                    std::cout << '\t';
                std::cout << key << '\t';
                curNode = curNode->next[curLevel];
            }
            idx = 0;
            std::cout << std::endl;
        }
        std::cout << "-------------------------------------------------" << std::endl;
        delete[] temp;
    }

public:
    void print(int formatted = 1)
    {
        using Callback = void (SkipList::*)(void);
        const Callback pFunc[2]{&SkipList<K, V>::_unformatted_print, &SkipList<K, V>::_formatted_print};
        (this->*pFunc[formatted])();
    }


//-------------------------------------------------------------------------------数据成员---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
private:
    //根据p获得的随机数阈值
    float _cutOff = RAND_MAX / 2.0f;
    //初始就有1层链表(0级链表)。_levels代表当前有多少层而不是当前最高层。
    int _levels = 1;
    //节点个数
    int _size = 0;
    //最大关键字
    K _maxKey;
    //头节点指针
    Node *_head = nullptr;
    //尾节点指针
    Node *_tail = nullptr;
    //存放查找中遇到的各级最后一个节点
    std::vector<Node *> _last;
    //实验需要
    int _totalCompare = 0;


//-------------------------------------------------------------------------------私有方法---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
private:
    inline void _initializeList()
    {
        srand(time(NULL));
        _head = new Node();
        _tail = new Node(Pair{_maxKey, V()});
        _head->next.push_back(_tail);
        _last.push_back(_head);
    }
    //生成级数。有可能会扩大最大级数。
    int _genLevel()
    {
        int level = 0;
        while (rand() < _cutOff && level < _levels)
            level++;
        //随机到一个比当前最大级数大的等级，拓展。
        if (level == _levels)
            _expandLevel();
        return level;
    }
    //用于拓展最大级数
    void _expandLevel()
    {
        //debug
        assert(_head->next.size() == _last.size());
        _levels++;
        _head->next.push_back(_tail);
        _last.push_back(_head);
    }
    //用于收缩最大级数
    void _shrinkLevel()
    {
        //避免无元素时收缩成0
        while (_head->next[_levels - 1] == _tail && _levels > 1)
        {
            _head->next.pop_back();
            _last.pop_back();
            _levels--;
        }
    }
    //用于把跳表退化为单链表
    void _degenerate()
    {
        Node *p = _head;
        while (p != _tail)
        {
            p->next.erase(p->next.begin() + 1, p->next.end());
            p = p->next[0];
        }
        _last.erase(_last.begin() + 1, _last.end());
        _levels = 1;
    }


//-------------------------------------------------------------------------构造/析构---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
public:
    SkipList() : _maxKey(std::numeric_limits<K>::max())
    {
        _initializeList();
    }
    SkipList(const K &maxKey, float prob = 0.5) : _maxKey(maxKey), _cutOff(RAND_MAX * prob)
    {
        _initializeList();
    }
    //复制构造
    SkipList(const SkipList &list) : _cutOff(list._cutOff), _levels(list._levels), _size(list._size), _maxKey(list._maxKey)
    {
        _initializeList();
        _head->next.clear();
        //逐节点复制，不会复制结构，会自动调整为最优结构
        Node *pList = list._head->next[0], *preThis = _head;
        while (pList != list._tail)
        {
            preThis->next.push_back(new Node(*pList));
            pList = pList->next[0];
            preThis = preThis->next[0];
        }
        preThis->next.push_back(_tail);
        settle();
    }
    //移动赋值
    SkipList &operator=(SkipList &&list)
    {
        _cutOff = list._cutOff;
        _levels = list._levels;
        _size = list._size;
        _maxKey = list._maxKey;
        _head = list._head;
        _tail = list._tail;
        _last = std::move(list._last);
        list._initializeList();
        list._levels = 1;
        list._size = 0;
        return *this;
    }
    //移动构造
    SkipList(SkipList &&list) : _cutOff(list._cutOff), _levels(list._levels), _size(list._size), _maxKey(list._maxKey)
    {
        _head = list._head;
        _tail = list._tail;
        _last = std::move(list._last);
        list._initializeList();
        list._levels = 1;
        list._size = 0;
    }
    //析构
    ~SkipList()
    {
        Node *p = _head, *next = _head->next[0];
        while (next != _tail)
        {
            next = p->next[0];
            delete p;
            p = next;
        }
        delete _tail;
    }


//-------------------------------------------------------------------------------简单成员接口---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
public:
    //返回表内元素总数
    inline int size() const
    {
        return _size;
    }
    //返回当前跳表总共拥有的级数
    inline int level() const
    {
        return _levels;
    }
    //返回最大key
    inline K maxKey() const
    {
        return _maxKey;
    }
    //如果表空，返回true
    inline bool empty() const
    {
        return !_size;
    }
    //返回目前为止元素比较的次数
    inline int compareCount() const
    {
        return _totalCompare;
    }


//-------------------------------------------------------------------------------主要方法---------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
public:
    //正常情况下接近于1/p分查找，最坏情况下等于线性查找
    //平均复杂度O(logn)，最坏复杂度O(n)
    //这个函数通常供类内其他成员函数调用，类外查找使用返回迭代器的find
    Node *search(const K &key)
    {
        Node *pre = _head;
        for (int curLevel = _levels - 1; curLevel >= 0; curLevel--)
        {
            while (pre->next[curLevel]->key < key)
            {
                pre = pre->next[curLevel];
                _totalCompare++;
            }
            //跳出while循环时也进行了一次比较，加上
            _totalCompare++;
            _last[curLevel] = pre;
        }
        return _last[0]->next[0];
    }
    //返回找到元素的迭代器
    //同上，平均复杂度O(logn)，最坏复杂度O(n)
    iterator find(const K &key)
    {
        Node *pre = _head;
        for (int curLevel = _levels - 1; curLevel >= 0; curLevel--)
        {
            while (pre->next[curLevel]->key < key)
            {
                pre = pre->next[curLevel];
                _totalCompare++;
            }
            _totalCompare++;
            _last[curLevel] = pre;
        }
        _totalCompare++;
        if (_last[0]->next[0]->key == key)
            return iterator(_last[0]->next[0], 0);
        return end();
    }
    // Insert要维护所有std::vector的长度，确保不会出现访问vector越界
    //调用了一次search，平均复杂度O(logn)，最坏复杂度O(n)
    void insert(const Pair &p)
    {
        //能找到key，直接修改value
        Node *node = search(p.key);
        _totalCompare++;
        if (node->elem.key == p.key)
        {
            node->elem.value = p.value;
            return;
        }
        //找不到key，进行插入
        int newLevel = _genLevel();
        Node *newNode = new Node(p);
        for (int curLevel = 0; curLevel <= newLevel; curLevel++)
        {
            newNode->next.push_back(_last[curLevel]->next[curLevel]);
            _last[curLevel]->next[curLevel] = newNode;
        }
        _size++;
    }
    //多元素插入
    void insert(std::initializer_list<Pair> list)
    {
        for (auto &p : list)
            insert(std::move(p));
    }
    //按关键字删除
    //调用search，平均复杂度O(logn)，最坏复杂度O(n)
    void erase(const K &key)
    {
        Node *node = search(key);
        //找不到就无事发生
        if (node->key != key)
            return;
        //找到就进行删除，并且可能进行收缩
        int level = node->next.size() - 1;
        for (int curLevel = level; curLevel >= 0; curLevel--)
            _last[curLevel]->next[curLevel] = node->next[curLevel];
        delete node;
        _size--;
        //如果这个节点是最高级链表节点，尝试收缩最大级数
        if (level == _levels - 1)
            _shrinkLevel();
    }
    //整理
    //先将所有节点降级为0级节点(next里只留下0级指针)，复杂度O(n)。
    //再通过遍历0级链表产生1级链表，遍历1级链表产生2级别链表，...
    //遍历元素n+n/2+n/4+...n/2^L=2n-(2n/2^(logn))，复杂度为O(n)。
    //故总体复杂度为O(n)。
    void settle()
    {
        //方法：对于i级链表，每1/p个i级链表节点升级为一个i+1级节点
        int interval = 1.0f / (_cutOff / RAND_MAX);
        //先把跳表置为只有0级链表的状态
        _degenerate();
        //从0级链表开始，挑出1级节点
        int curLevel = 0;
        Node *p = nullptr;
        while ((p = _head->next[curLevel]) != _tail)
        {
            //前一个升级的节点。
            Node *pre = _head;
            int count = 0;
            while (p != _tail)
            {
                count++;
                //到该升级的节点了
                if (count == interval)
                {
                    pre->next.push_back(p);
                    pre = p;
                    //清空count，重新计数间隔
                    count = 0;
                }
                p = p->next[curLevel];
            }
            //遍历完成，写入尾节点，增加总级数
            pre->next.push_back(_tail);
            curLevel++;
            _levels++;
        }
        //这个时候最顶级链表是空的，减少一层
        _head->next.pop_back();
        _levels--;
        _last.resize(_levels);
    }
    //删除起始元素
    //直接删除头节点的后一个元素，调整L层的指针，复杂度为O(L)=O(logn)，最好情况为O(1)
    std::optional<Pair> pop_front()
    {
        //如果表空，返回空对象
        if (empty())
            return std::nullopt;
        Node *delNode = _head->next[0];
        //获取这个节点等级
        int level = delNode->next.size() - 1;
        //这个等级内的链表都跳过这个节点
        for (int curLevel = 0; curLevel <= level; curLevel++)
            _head->next[curLevel] = delNode->next[curLevel];
        //准备返回值
        Pair ret = delNode->elem;
        delete delNode;
        _size--;
        //如果删除了最高级节点，尝试收缩链表等级
        if (level == _levels - 1)
            _shrinkLevel();
        return ret;
    }
    //删除尾元素
    //需要找到尾元素,折半逼近最后一个元素，复杂度O(logn)
    //找到尾元素后进行删除，调整L层的指针，复杂度O(logn)
    //故总体复杂度O(logn)
    std::optional<Pair> pop_back()
    {
        //如果表空，返回空对象
        if (empty())
            return std::nullopt;
        Node *delNode = _head;
        //查找最后一个节点
        _last[_levels - 1] = _head;
        for (int curLevel = _levels - 1; curLevel >= 0; curLevel--)
        {
            //确保正确记录路径
            if (curLevel != _levels - 1)
            {
                _last[curLevel] = _last[curLevel + 1];
                //先把_last[curLevel]指针指向delNode的前一个
                while (_last[curLevel]->next[curLevel] != delNode)
                    _last[curLevel] = _last[curLevel]->next[curLevel];
            }
            while (delNode->next[curLevel] != _tail)
            {
                //记录每层的搜索路径，方便后续删除
                _last[curLevel] = delNode;
                delNode = delNode->next[curLevel];
            }
        }
        //获取要删除节点的级数
        int level = delNode->next.size() - 1;
        //这些等级内的链表都跳过这个节点
        for (int curLevel = 0; curLevel <= level; curLevel++)
            _last[curLevel]->next[curLevel] = _tail;
        //准备返回值
        Pair ret = delNode->elem;
        delete delNode;
        _size--;
        //如果删除了最高级节点，尝试收缩链表等级
        if (level == _levels - 1)
            _shrinkLevel();
        return ret;
    }
};