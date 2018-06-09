/*
 * Copyright 2018 ARP Network
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <map>
#include <regex>

#include <cstdio>

using std::map;
using std::regex;

struct Block
{
  int id;
  int depth;
  Block *parent;
  Block *leader;

  // construtor
  Block();
  Block(int id, Block *parent);
};

typedef map<int, Block *> BlockMap;

struct BlockTree
{
  BlockMap blocks;

  BlockTree();
  ~BlockTree();

  // Inserts new block to the tree
  Block *insert(int id, int parent);
  // Returns the leader by given block id
  Block *leader(int id = 0);

  // Disallow copy and assign
  BlockTree(const BlockTree &) = delete;
  BlockTree &operator=(const BlockTree &) = delete;
};

int main(int argc, char *argv[])
{
  BlockTree btree;

  // Insert command `i <id> <parent>`
  regex p_insert("^\\s*i\\s+(\\d+)\\s(\\d+)\\s*$");
  // Leader query command `l <id>`
  regex p_leader("^\\s*l\\s+(\\d+)\\s*$");
  // Chain query command `c <id> <max>`
  regex p_chain("^\\s*c\\s+(\\d+)\\s(\\d+)\\s*$");

  char buf[BUFSIZ];
  while (std::fgets(buf, BUFSIZ, stdin) != nullptr)
  {
    auto parse_int = [](const std::csub_match &sm) { return std::strtol(sm.str().c_str(), nullptr, 10); };

    int result = 0;
    auto items = std::vector<int>();

    std::cmatch m;
    if (std::regex_match(buf, m, p_insert))
    {
      int id = parse_int(m[1]);
      int parent = parse_int(m[2]);
      auto block = btree.insert(id, parent);
      if (block != nullptr)
      {
        items.push_back(block->depth);
      }
    }
    else if (std::regex_match(buf, m, p_leader))
    {
      int id = parse_int(m[1]);
      auto block = btree.leader(id);
      if (block != nullptr)
      {
        items.push_back(block->id);
        items.push_back(block->depth);
      }
    }
    else if (std::regex_match(buf, m, p_chain))
    {
      int id = parse_int(m[1]);
      int max = parse_int(m[2]);
      auto block = btree.leader(id);
      if (block != nullptr)
      {
        Block *pb = block;
        for (auto i = 0; i < max && pb != nullptr; i++, pb = pb->parent)
        {
          items.push_back(pb->id);
        }
      }
    }
    else
    {
      result = 255;
    }

    if (!items.empty())
    {
      std::printf("0");
      for (auto it : items)
      {
        std::printf(" %d", it);
      }
      std::printf("\n");
    }
    else
    {
      std::printf("%d\n", result > 0 ? result : 1);
    }
    std::fflush(stdout);
  }

  return std::ferror(stdin);
}

Block::Block()
    : id(0),
      depth(0),
      parent(nullptr),
      leader(this)
{
}

Block::Block(int id, Block *parent)
    : id(id),
      depth(parent->depth + 1),
      parent(parent),
      leader(this)
{
}

BlockTree::BlockTree()
{
  blocks[0] = new Block();
}

BlockTree::~BlockTree()
{
  for (auto it : blocks)
  {
    delete it.second;
  }
}

Block *BlockTree::leader(int id /* = 0 */)
{
  Block *block = blocks[id];
  return block != nullptr ? block->leader : nullptr;
}

Block *BlockTree::insert(int id, int parent)
{
  Block *block = nullptr;

  auto pb = blocks[parent];
  if (pb != nullptr)
  {
    block = blocks[id] = new Block(id, pb);

    // Update current block chain with new leader
    for (auto depth = block->depth;
         pb != nullptr && pb->leader->depth < depth;
         pb = pb->parent)
    {
      pb->leader = block;
    }
  }

  return block;
}
