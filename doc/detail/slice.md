## 涉及文件
* slice.h

## 功能

Slice是LevelDB中最为基础的数据结构，通过直接操控字符指针来避免不必要的数据拷贝。

## 成员变量

* `const char* data_`：保存字符串
* `size_t size_`：保存字符串长度

## 构造函数和析构函数

Slice共提供了4种构造函数：
* 空构造函数
* 指定位数的字符串（`d[0,n-1]`）
* string类型
* 整个字符串（`s[0,strlen(s)-1]`）

Slice仅仅记录对应字符串的指针位置，而非深拷贝数据。

## 成员函数

Slice提供一些基本的功能，包括：
* 返回数据指针：`const char* data() const`
* 返回长度：`size_t size() const`
* 判断是否为空：`bool empty() const`
* 清空：`void clear()`
* 删除指定字节数的前缀：`void remove_prefix(size_t n)`
* 转为string：`std::string ToString() const`
* 比较：`int compare(const Slice& b) const`
* 判断是否以某个Slice开头：`bool start_with(const Slice x) const`

同时Slice重载了一些函数，包括：
* 括号取值：`char operator[](size_t n) const`
* 等于：`bool operator==(const Slice& x, const Slice& y)`
* 不等：`bool operator!=(const Slice& x, const Slice& y)`