## Data Structure
一些基本的数据结构和算法，基本都采用 C 语言来实现，方便快捷

* 红黑树
  性质
  ```
  1. 节点是红色或者黑色
  2. 根节点是黑色
  3. 不能有两个相邻的红色节点（红色节点的子节点只能是黑色节点）
  4. 所有的叶子节点都是黑色节点（nil是黑色节点）
  5. 从任意根节点到叶子节点的简单路径都包含相同个数的黑色节点
  ```