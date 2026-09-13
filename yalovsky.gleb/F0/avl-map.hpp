#ifndef YALOVSKY_F0_AVL_MAP_HPP
#define YALOVSKY_F0_AVL_MAP_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace yalovsky
{
  namespace detail
  {
    template< class Key, class Value >
    struct AvlNode
    {
      const Key key;
      Value value;
      std::unique_ptr< AvlNode > left;
      std::unique_ptr< AvlNode > right;
      int height;

      AvlNode(const Key& newKey, Value newValue);
    };
  }

  template< class Key, class Value >
  class AvlMap
  {
  public:
    AvlMap() = default;
    AvlMap(const AvlMap& other);
    AvlMap(AvlMap&& other) noexcept;
    AvlMap& operator=(AvlMap other);

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    int getHeight() const noexcept;

    Value* find(const Key& key);
    const Value* find(const Key& key) const;
    Value& at(const Key& key);
    const Value& at(const Key& key) const;

    bool insert(const Key& key, Value value);
    bool erase(const Key& key);
    void clear() noexcept;
    void swap(AvlMap& other) noexcept;

    template< class Function >
    void forEach(Function function);

    template< class Function >
    void forEach(Function function) const;

  private:
    using Node = detail::AvlNode< Key, Value >;
    using NodePtr = std::unique_ptr< Node >;

    NodePtr root_;
    std::size_t size_ = 0;

    static int getNodeHeight(const NodePtr& node) noexcept;
    static void updateHeight(Node& node) noexcept;
    static void rotateLeft(NodePtr& node) noexcept;
    static void rotateRight(NodePtr& node) noexcept;
    static void balance(NodePtr& node) noexcept;
    static NodePtr clone(const NodePtr& node);
    static NodePtr takeMin(NodePtr& node) noexcept;
    static bool insertNode(NodePtr& node, const Key& key, Value&& value);
    static bool eraseNode(NodePtr& node, const Key& key);

    template< class NodeType >
    static NodeType* findNode(NodeType* node, const Key& key);

    template< class NodeType, class Function >
    static void traverse(NodeType* node, Function& function);
  };
}

template< class Key, class Value >
yalovsky::detail::AvlNode< Key, Value >::AvlNode(const Key& newKey, Value newValue):
  key(newKey),
  value(std::move(newValue)),
  left(nullptr),
  right(nullptr),
  height(1)
{}

template< class Key, class Value >
yalovsky::AvlMap< Key, Value >::AvlMap(const AvlMap& other):
  root_(clone(other.root_)),
  size_(other.size_)
{}

template< class Key, class Value >
yalovsky::AvlMap< Key, Value >::AvlMap(AvlMap&& other) noexcept:
  root_(std::move(other.root_)),
  size_(std::exchange(other.size_, 0))
{}

template< class Key, class Value >
yalovsky::AvlMap< Key, Value >&
yalovsky::AvlMap< Key, Value >::operator=(AvlMap other)
{
  swap(other);
  return *this;
}

template< class Key, class Value >
bool yalovsky::AvlMap< Key, Value >::empty() const noexcept
{
  return size_ == 0;
}

template< class Key, class Value >
std::size_t yalovsky::AvlMap< Key, Value >::size() const noexcept
{
  return size_;
}

template< class Key, class Value >
int yalovsky::AvlMap< Key, Value >::getHeight() const noexcept
{
  return getNodeHeight(root_);
}

template< class Key, class Value >
int yalovsky::AvlMap< Key, Value >::getNodeHeight(const NodePtr& node) noexcept
{
  return node ? node->height : 0;
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::updateHeight(Node& node) noexcept
{
  node.height = std::max(getNodeHeight(node.left), getNodeHeight(node.right)) + 1;
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::rotateLeft(NodePtr& node) noexcept
{
  NodePtr pivot = std::move(node->right);
  node->right = std::move(pivot->left);
  updateHeight(*node);
  pivot->left = std::move(node);
  updateHeight(*pivot);
  node = std::move(pivot);
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::rotateRight(NodePtr& node) noexcept
{
  NodePtr pivot = std::move(node->left);
  node->left = std::move(pivot->right);
  updateHeight(*node);
  pivot->right = std::move(node);
  updateHeight(*pivot);
  node = std::move(pivot);
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::balance(NodePtr& node) noexcept
{
  if (!node)
  {
    return;
  }

  updateHeight(*node);
  const int difference = getNodeHeight(node->right) - getNodeHeight(node->left);

  if (difference > 1)
  {
    if (getNodeHeight(node->right->left) > getNodeHeight(node->right->right))
    {
      rotateRight(node->right);
    }
    rotateLeft(node);
  }
  else if (difference < -1)
  {
    if (getNodeHeight(node->left->right) > getNodeHeight(node->left->left))
    {
      rotateLeft(node->left);
    }
    rotateRight(node);
  }
}

template< class Key, class Value >
typename yalovsky::AvlMap< Key, Value >::NodePtr
yalovsky::AvlMap< Key, Value >::clone(const NodePtr& node)
{
  if (!node)
  {
    return nullptr;
  }

  NodePtr result(new Node(node->key, node->value));
  result->left = clone(node->left);
  result->right = clone(node->right);
  result->height = node->height;
  return result;
}

template< class Key, class Value >
template< class NodeType >
NodeType* yalovsky::AvlMap< Key, Value >::findNode(NodeType* node, const Key& key)
{
  while (node)
  {
    if (key < node->key)
    {
      node = node->left.get();
    }
    else if (node->key < key)
    {
      node = node->right.get();
    }
    else
    {
      return node;
    }
  }
  return nullptr;
}

template< class Key, class Value >
Value* yalovsky::AvlMap< Key, Value >::find(const Key& key)
{
  Node* node = findNode(root_.get(), key);
  return node ? &node->value : nullptr;
}

template< class Key, class Value >
const Value* yalovsky::AvlMap< Key, Value >::find(const Key& key) const
{
  const Node* root = root_.get();
  const Node* node = findNode(root, key);
  return node ? &node->value : nullptr;
}

template< class Key, class Value >
Value& yalovsky::AvlMap< Key, Value >::at(const Key& key)
{
  Value* value = find(key);
  if (!value)
  {
    throw std::out_of_range("Key not found");
  }
  return *value;
}

template< class Key, class Value >
const Value& yalovsky::AvlMap< Key, Value >::at(const Key& key) const
{
  const Value* value = find(key);
  if (!value)
  {
    throw std::out_of_range("Key not found");
  }
  return *value;
}

template< class Key, class Value >
bool yalovsky::AvlMap< Key, Value >::insertNode(NodePtr& node, const Key& key, Value&& value)
{
  if (!node)
  {
    node.reset(new Node(key, std::move(value)));
    return true;
  }

  bool inserted = false;
  if (key < node->key)
  {
    inserted = insertNode(node->left, key, std::move(value));
  }
  else if (node->key < key)
  {
    inserted = insertNode(node->right, key, std::move(value));
  }
  else
  {
    return false;
  }

  balance(node);
  return inserted;
}

template< class Key, class Value >
bool yalovsky::AvlMap< Key, Value >::insert(const Key& key, Value value)
{
  const bool inserted = insertNode(root_, key, std::move(value));
  if (inserted)
  {
    ++size_;
  }
  return inserted;
}

template< class Key, class Value >
typename yalovsky::AvlMap< Key, Value >::NodePtr
yalovsky::AvlMap< Key, Value >::takeMin(NodePtr& node) noexcept
{
  if (!node->left)
  {
    NodePtr result = std::move(node);
    node = std::move(result->right);
    return result;
  }

  NodePtr result = takeMin(node->left);
  balance(node);
  return result;
}

template< class Key, class Value >
bool yalovsky::AvlMap< Key, Value >::eraseNode(NodePtr& node, const Key& key)
{
  if (!node)
  {
    return false;
  }

  bool removed = false;
  if (key < node->key)
  {
    removed = eraseNode(node->left, key);
  }
  else if (node->key < key)
  {
    removed = eraseNode(node->right, key);
  }
  else
  {
    NodePtr old = std::move(node);
    if (!old->right)
    {
      node = std::move(old->left);
    }
    else
    {
      node = takeMin(old->right);
      node->left = std::move(old->left);
      node->right = std::move(old->right);
    }
    removed = true;
  }

  balance(node);
  return removed;
}

template< class Key, class Value >
bool yalovsky::AvlMap< Key, Value >::erase(const Key& key)
{
  const bool removed = eraseNode(root_, key);
  if (removed)
  {
    --size_;
  }
  return removed;
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::clear() noexcept
{
  root_.reset();
  size_ = 0;
}

template< class Key, class Value >
void yalovsky::AvlMap< Key, Value >::swap(AvlMap& other) noexcept
{
  root_.swap(other.root_);
  std::swap(size_, other.size_);
}

template< class Key, class Value >
template< class NodeType, class Function >
void yalovsky::AvlMap< Key, Value >::traverse(NodeType* node, Function& function)
{
  if (!node)
  {
    return;
  }

  traverse(static_cast< NodeType* >(node->left.get()), function);
  function(node->key, node->value);
  traverse(static_cast< NodeType* >(node->right.get()), function);
}

template< class Key, class Value >
template< class Function >
void yalovsky::AvlMap< Key, Value >::forEach(Function function)
{
  traverse(root_.get(), function);
}

template< class Key, class Value >
template< class Function >
void yalovsky::AvlMap< Key, Value >::forEach(Function function) const
{
  const Node* root = root_.get();
  traverse(root, function);
}

#endif
