/***********************************************************************
一、将buffer分为四部分，第1部分是mem_pool结构体；第2部分是内存映射表；第3部分是内存chunk结构体缓冲区；
第4部分是实际可分配的内存区。
1.第1部分的作用是可以通过该mem_pool结构体控制整个内存池。
2.第2部分的作用是记录第4部分，即实际可分配的内存区的使用情况。
表中的每一个单元表示一个固定大小的内存块（block），多个连续的block组成一个chunk，
其中count表示该block后面的与该block同属于一个chunk的blokc的个数，start表示该block所在的chunk的起始block索引。
其实start这个域只有在每个chunk的最后一个block中才会用到（用于从当前chunk寻找前一个chunk的起始位置），
而pmem_chunk则是一个指针，指向一个mem_chunk结构体。任意一块大小的内存都会被取向上整到block大小的整数倍。
3.第3部分是一个mem_chunk pool，其作用是存储整个程序可用的mem_chunk结构体。
mem_chunk pool中的mem_chunk被组织成双向链表结构（快速插入和删除）。
其中pmem_block指向该chunk在内存映射表中的位置，others表示其他一些域，不同的实现对应该域的内容略有不同。
4.第4部分就是实际可以被分配给用户的内存。
5.整个内存池管理程序除了这四部分外，还有一个重要的内容就是memory chunk set。
虽然其中的每个元素都来自mem_chunk pool，但是它与mem_chunk pool的不同之处在于其中的每个memory chunk中
记录了当前可用的一块内存的相关信息。而mem_chunk pool中的memory chunk的内容是无定以的。
可以这样理解mem_chunk pool与memory chunk set：mem_chunk pool是为memory chunk set分配内存的“内存池”，
只是该“内存池”每次分配的内存大小是固定的，为mem_chunk结构体的大小。
内存池程序主要是通过搜索这个memory chunk set来获取可被分配的内存。
在memory chunk set上建立不同的数据结构就构成了不同的内存池实现方法，
同时也导致了不同的搜索效率，直接影响内存池的性能，本文稍后会介绍两种内存池的实现。
二、内存池管理程序运行过程
1.初始化：内存映射表中只有一块可用的内存信息，大小为内存池中所有可用的内存。
从memory chunk pool中分配一个mem_chunk，使其指向内存映射表中的第一个block，
并根据具体的内存池实现方式填充mem_chunk中的其他域，然后将该mem_chunk添加到memory chunk set中。
2.申请内存：当用户申请一块内存时，首先在memory chunk set中查找合适的内存块。
如果找到符合要求的内存块，就在内存映射表中找到相应的chunk，并修改chunk中相应block结构体的内容，
然后根据修改后的chunk修改memory chunk set中chunk的内容，最后返回分配内存的起始地址；否则返回NULL。
3.释放内存：当用户释放一块内存时，首先根据这块内存的起始地址找到其在内存映射表中对应的chunk，
然后尝试将该chunk和与其相邻的chunk合并，修改chunk中相应block的内容并修改memory chunk set中相应chunk的内容
或者向memory chunk set加入新的mem_chunk（这种情况在不能合并内存是发生）。
***********************************************************************/	

/**************************************************************************
 * 链表结构内存池：
**************************************************************************/
	MemoryPool.h
[cpp] view plain copy
#ifndef _MEMORYPOOL_H  
#define _MEMORYPOOL_H  
#include <stdlib.h>  
#define MINUNITSIZE 64  
#define ADDR_ALIGN 8  
#define SIZE_ALIGN MINUNITSIZE  
struct memory_chunk;  
typedef struct memory_block  
{  
    size_t count;  
    size_t start;  
    memory_chunk* pmem_chunk;  
}memory_block;  
// 可用的内存块结构体  
typedef struct memory_chunk  
{  
    memory_block* pfree_mem_addr;  
    memory_chunk* pre;  
    memory_chunk* next;  
}memory_chunk;  
// 内存池结构体  
typedef struct MEMORYPOOL  
{  
    void *memory;  
    size_t size;  
    memory_block* pmem_map;   
    memory_chunk* pfree_mem_chunk;  
    memory_chunk* pfree_mem_chunk_pool;  
    size_t mem_used_size; // 记录内存池中已经分配给用户的内存的大小  
    size_t mem_map_pool_count; // 记录链表单元缓冲池中剩余的单元的个数，个数为0时不能分配单元给pfree_mem_chunk  
    size_t free_mem_chunk_count; // 记录 pfree_mem_chunk链表中的单元个数  
    size_t mem_map_unit_count; //   
    size_t mem_block_count; // 一个 mem_unit 大小为 MINUNITSIZE  
}MEMORYPOOL, *PMEMORYPOOL;  
/************************************************************************/  
/* 生成内存池 
 * pBuf: 给定的内存buffer起始地址 
 * sBufSize: 给定的内存buffer大小 
 * 返回生成的内存池指针 
/************************************************************************/  
PMEMORYPOOL CreateMemoryPool(void* pBuf, size_t sBufSize);  
/************************************************************************/  
/* 暂时没用 
/************************************************************************/  
void ReleaseMemoryPool(PMEMORYPOOL* ppMem) ;   
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
 * pMem: 内存池 指针 
 * sMemorySize: 要分配的内存大小 
 * 成功时返回分配的内存起始地址，失败返回NULL 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, PMEMORYPOOL pMem) ;  
  
/************************************************************************/  
/* 从内存池中释放申请到的内存 
 * pMem：内存池指针 
 * ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void FreeMemory(void *ptrMemoryBlock, PMEMORYPOOL pMem) ;  
  
#endif //_MEMORYPOOL_H  
 
	MemoryPool.cpp
[cpp] view plain copy
#include "stdafx.h"  
#include <memory.h>  
#include "MemoryPool.h"  
/************************************************************************/  
/* 内存池起始地址对齐到ADDR_ALIGN字节 
/************************************************************************/  
size_t check_align_addr(void*& pBuf)  
{  
    size_t align = 0;  
    size_t addr = (int)pBuf;  
    align = (ADDR_ALIGN - addr % ADDR_ALIGN) % ADDR_ALIGN;  
    pBuf = (char*)pBuf + align;  
    return align;  
}  
/************************************************************************/  
/* 内存block大小对齐到MINUNITSIZE字节 
/************************************************************************/  
size_t check_align_block(size_t size)  
{  
    size_t align = size % MINUNITSIZE;  
      
    return size - align;   
}  
/************************************************************************/  
/* 分配内存大小对齐到SIZE_ALIGN字节 
/************************************************************************/  
size_t check_align_size(size_t size)  
{  
    size = (size + SIZE_ALIGN - 1) / SIZE_ALIGN * SIZE_ALIGN;  
    return size;  
}  
/************************************************************************/  
/* 以下是链表相关操作 
/************************************************************************/  
memory_chunk* create_list(memory_chunk* pool, size_t count)  
{  
    if (!pool)  
    {  
        return NULL;  
    }  
    memory_chunk* head = NULL;  
    for (size_t i = 0; i < count; i++)  
    {  
        pool->pre = NULL;  
        pool->next = head;  
        if (head != NULL)  
        {  
            head->pre = pool;              
        }  
        head = pool;  
        pool++;  
    }  
    return head;  
}  
memory_chunk* front_pop(memory_chunk*& pool)  
{  
    if (!pool)  
    {  
        return NULL;  
    }  
    memory_chunk* tmp = pool;  
    pool = tmp->next;  
    pool->pre = NULL;  
    return  tmp;  
}  
void push_back(memory_chunk*& head, memory_chunk* element)  
{  
    if (head == NULL)  
    {  
        head = element;  
        head->pre = element;  
        head->next = element;  
        return;  
    }  
    head->pre->next = element;  
    element->pre = head->pre;  
    head->pre = element;  
    element->next = head;  
}  
void push_front(memory_chunk*& head, memory_chunk* element)  
{  
    element->pre = NULL;  
    element->next = head;  
    if (head != NULL)  
    {  
        head->pre = element;           
    }  
    head = element;  
}  
void delete_chunk(memory_chunk*& head, memory_chunk* element)  
{  
    // 在双循环链表中删除元素  
    if (element == NULL)  
    {  
        return;  
    }  
    // element为链表头  
    else if (element == head)  
    {  
        // 链表只有一个元素  
        if (head->pre == head)  
        {  
            head = NULL;  
        }  
        else  
        {  
            head = element->next;  
            head->pre = element->pre;  
            head->pre->next = head;  
        }  
    }  
    // element为链表尾  
    else if (element->next == head)  
    {  
        head->pre = element->pre;  
        element->pre->next = head;  
    }  
    else  
    {  
        element->pre->next = element->next;  
        element->next->pre = element->pre;  
    }  
    element->pre = NULL;  
    element->next = NULL;  
}  
/************************************************************************/  
/* 内存映射表中的索引转化为内存起始地址                                                                     
/************************************************************************/  
void* index2addr(PMEMORYPOOL mem_pool, size_t index)  
{  
    char* p = (char*)(mem_pool->memory);  
    void* ret = (void*)(p + index *MINUNITSIZE);  
      
    return ret;  
}  
/************************************************************************/  
/* 内存起始地址转化为内存映射表中的索引                                                                     
/************************************************************************/  
size_t addr2index(PMEMORYPOOL mem_pool, void* addr)  
{  
    char* start = (char*)(mem_pool->memory);  
    char* p = (char*)addr;  
    size_t index = (p - start) / MINUNITSIZE;  
    return index;  
}  
/************************************************************************/  
/* 生成内存池 
* pBuf: 给定的内存buffer起始地址 
* sBufSize: 给定的内存buffer大小 
* 返回生成的内存池指针 
/************************************************************************/  
PMEMORYPOOL CreateMemoryPool(void* pBuf, size_t sBufSize)  
{  
    memset(pBuf, 0, sBufSize);  
    PMEMORYPOOL mem_pool = (PMEMORYPOOL)pBuf;  
    // 计算需要多少memory map单元格  
    size_t mem_pool_struct_size = sizeof(MEMORYPOOL);  
    mem_pool->mem_map_pool_count = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;  
    mem_pool->mem_map_unit_count = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;  
    mem_pool->pmem_map = (memory_block*)((char*)pBuf + mem_pool_struct_size);  
    mem_pool->pfree_mem_chunk_pool = (memory_chunk*)((char*)pBuf + mem_pool_struct_size + sizeof(memory_block) * mem_pool->mem_map_unit_count);  
      
    mem_pool->memory = (char*)pBuf + mem_pool_struct_size+ sizeof(memory_block) * mem_pool->mem_map_unit_count + sizeof(memory_chunk) * mem_pool->mem_map_pool_count;  
    mem_pool->size = sBufSize - mem_pool_struct_size - sizeof(memory_block) * mem_pool->mem_map_unit_count - sizeof(memory_chunk) * mem_pool->mem_map_pool_count;  
    size_t align = check_align_addr(mem_pool->memory);  
    mem_pool->size -= align;  
    mem_pool->size = check_align_block(mem_pool->size);  
    mem_pool->mem_block_count = mem_pool->size / MINUNITSIZE;  
    // 链表化  
    mem_pool->pfree_mem_chunk_pool = create_list(mem_pool->pfree_mem_chunk_pool, mem_pool->mem_map_pool_count);  
    // 初始化 pfree_mem_chunk，双向循环链表  
    memory_chunk* tmp = front_pop(mem_pool->pfree_mem_chunk_pool);  
    tmp->pre = tmp;  
    tmp->next = tmp;  
    tmp->pfree_mem_addr = NULL;  
    mem_pool->mem_map_pool_count--;  
      
    // 初始化 pmem_map  
    mem_pool->pmem_map[0].count = mem_pool->mem_block_count;  
    mem_pool->pmem_map[0].pmem_chunk = tmp;  
    mem_pool->pmem_map[mem_pool->mem_block_count-1].start = 0;  
      
    tmp->pfree_mem_addr = mem_pool->pmem_map;  
    push_back(mem_pool->pfree_mem_chunk, tmp);  
    mem_pool->free_mem_chunk_count = 1;  
    mem_pool->mem_used_size = 0;  
    return mem_pool;  
}  
/************************************************************************/  
/* 暂时没用 
/************************************************************************/  
void ReleaseMemoryPool(PMEMORYPOOL* ppMem)   
{  
}  
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
* pMem: 内存池 指针 
* sMemorySize: 要分配的内存大小 
* 成功时返回分配的内存起始地址，失败返回NULL 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, PMEMORYPOOL pMem)  
{  
    sMemorySize = check_align_size(sMemorySize);  
    size_t index = 0;  
    memory_chunk* tmp = pMem->pfree_mem_chunk;  
    for (index = 0; index < pMem->free_mem_chunk_count; index++)  
    {  
        if (tmp->pfree_mem_addr->count * MINUNITSIZE >= sMemorySize)  
        {             
            break;  
        }  
          
        tmp = tmp->next;  
    }  
      
    if (index == pMem->free_mem_chunk_count)  
    {  
        return NULL;  
    }  
    pMem->mem_used_size += sMemorySize;  
    if (tmp->pfree_mem_addr->count * MINUNITSIZE == sMemorySize)  
    {  
        // 当要分配的内存大小与当前chunk中的内存大小相同时，从pfree_mem_chunk链表中删除此chunk  
        size_t current_index = (tmp->pfree_mem_addr - pMem->pmem_map);  
        delete_chunk(pMem->pfree_mem_chunk, tmp);  
        tmp->pfree_mem_addr->pmem_chunk = NULL;  
          
        push_front(pMem->pfree_mem_chunk_pool, tmp);  
        pMem->free_mem_chunk_count--;  
        pMem->mem_map_pool_count++;  
          
        return index2addr(pMem, current_index);  
    }  
    else  
    {  
        // 当要分配的内存小于当前chunk中的内存时，更改pfree_mem_chunk中相应chunk的pfree_mem_addr  
          
        // 复制当前mem_map_unit  
        memory_block copy;  
        copy.count = tmp->pfree_mem_addr->count;  
        copy.pmem_chunk = tmp;  
        // 记录该block的起始和结束索引  
        memory_block* current_block = tmp->pfree_mem_addr;  
        current_block->count = sMemorySize / MINUNITSIZE;  
        size_t current_index = (current_block - pMem->pmem_map);  
        pMem->pmem_map[current_index+current_block->count-1].start = current_index;  
        current_block->pmem_chunk = NULL; // NULL表示当前内存块已被分配  
        // 当前block被一分为二，更新第二个block中的内容  
        pMem->pmem_map[current_index+current_block->count].count = copy.count - current_block->count;  
        pMem->pmem_map[current_index+current_block->count].pmem_chunk = copy.pmem_chunk;  
        // 更新原来的pfree_mem_addr  
        tmp->pfree_mem_addr = &(pMem->pmem_map[current_index+current_block->count]);  
      
        size_t end_index = current_index + copy.count - 1;  
        pMem->pmem_map[end_index].start = current_index + current_block->count;  
        return index2addr(pMem, current_index);  
    }     
}  
/************************************************************************/  
/* 从内存池中释放申请到的内存 
* pMem：内存池指针 
* ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void FreeMemory(void *ptrMemoryBlock, PMEMORYPOOL pMem)   
{  
    size_t current_index = addr2index(pMem, ptrMemoryBlock);  
    size_t size = pMem->pmem_map[current_index].count * MINUNITSIZE;  
    // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并  
    memory_block* pre_block = NULL;  
    memory_block* next_block = NULL;  
    memory_block* current_block = &(pMem->pmem_map[current_index]);  
    // 第一个  
    if (current_index == 0)  
    {  
        if (current_block->count < pMem->mem_block_count)  
        {  
            next_block = &(pMem->pmem_map[current_index+current_block->count]);  
            // 如果后一个内存块是空闲的，合并  
            if (next_block->pmem_chunk != NULL)  
            {  
                next_block->pmem_chunk->pfree_mem_addr = current_block;  
                pMem->pmem_map[current_index+current_block->count+next_block->count-1].start = current_index;  
                current_block->count += next_block->count;  
                current_block->pmem_chunk = next_block->pmem_chunk;  
                next_block->pmem_chunk = NULL;  
            }  
            // 如果后一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {  
                memory_chunk* new_chunk = front_pop(pMem->pfree_mem_chunk_pool);  
                new_chunk->pfree_mem_addr = current_block;  
                current_block->pmem_chunk = new_chunk;  
                push_back(pMem->pfree_mem_chunk, new_chunk);  
                pMem->mem_map_pool_count--;  
                pMem->free_mem_chunk_count++;  
            }  
        }  
        else  
        {  
            memory_chunk* new_chunk = front_pop(pMem->pfree_mem_chunk_pool);  
            new_chunk->pfree_mem_addr = current_block;  
            current_block->pmem_chunk = new_chunk;  
            push_back(pMem->pfree_mem_chunk, new_chunk);  
            pMem->mem_map_pool_count--;  
            pMem->free_mem_chunk_count++;  
        }         
    }  
      
    // 最后一个  
    else if (current_index == pMem->mem_block_count-1)  
    {  
        if (current_block->count < pMem->mem_block_count)  
        {  
            pre_block = &(pMem->pmem_map[current_index-1]);  
            size_t index = pre_block->count;  
            pre_block = &(pMem->pmem_map[index]);  
              
            // 如果前一个内存块是空闲的，合并  
            if (pre_block->pmem_chunk != NULL)  
            {  
                pMem->pmem_map[current_index+current_block->count-1].start = current_index - pre_block->count;  
                pre_block->count += current_block->count;  
                current_block->pmem_chunk = NULL;  
            }  
            // 如果前一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {  
                memory_chunk* new_chunk = front_pop(pMem->pfree_mem_chunk_pool);  
                new_chunk->pfree_mem_addr = current_block;  
                current_block->pmem_chunk = new_chunk;  
                push_back(pMem->pfree_mem_chunk, new_chunk);  
                pMem->mem_map_pool_count--;  
                pMem->free_mem_chunk_count++;  
            }  
        }  
        else  
        {  
            memory_chunk* new_chunk = front_pop(pMem->pfree_mem_chunk_pool);  
            new_chunk->pfree_mem_addr = current_block;  
            current_block->pmem_chunk = new_chunk;  
            push_back(pMem->pfree_mem_chunk, new_chunk);  
            pMem->mem_map_pool_count--;  
            pMem->free_mem_chunk_count++;  
        }  
    }  
    else  
    {         
        next_block = &(pMem->pmem_map[current_index+current_block->count]);  
        pre_block = &(pMem->pmem_map[current_index-1]);  
        size_t index = pre_block->start;  
        pre_block = &(pMem->pmem_map[index]);  
        bool is_back_merge = false;  
        if (next_block->pmem_chunk == NULL && pre_block->pmem_chunk == NULL)  
        {  
            memory_chunk* new_chunk = front_pop(pMem->pfree_mem_chunk_pool);  
            new_chunk->pfree_mem_addr = current_block;  
            current_block->pmem_chunk = new_chunk;  
            push_back(pMem->pfree_mem_chunk, new_chunk);  
            pMem->mem_map_pool_count--;  
            pMem->free_mem_chunk_count++;  
        }  
        // 后一个内存块  
        if (next_block->pmem_chunk != NULL)  
        {  
            next_block->pmem_chunk->pfree_mem_addr = current_block;  
            pMem->pmem_map[current_index+current_block->count+next_block->count-1].start = current_index;  
            current_block->count += next_block->count;  
            current_block->pmem_chunk = next_block->pmem_chunk;  
            next_block->pmem_chunk = NULL;  
            is_back_merge = true;  
        }  
        // 前一个内存块  
        if (pre_block->pmem_chunk != NULL)  
        {  
            pMem->pmem_map[current_index+current_block->count-1].start = current_index - pre_block->count;  
            pre_block->count += current_block->count;  
            if (is_back_merge)  
            {  
                delete_chunk(pMem->pfree_mem_chunk, current_block->pmem_chunk);  
                push_front(pMem->pfree_mem_chunk_pool, current_block->pmem_chunk);  
                pMem->free_mem_chunk_count--;  
                pMem->mem_map_pool_count++;  
            }  
            current_block->pmem_chunk = NULL;              
        }         
    }  
    pMem->mem_used_size -= size;  
}  
 
	MemoryPoolTest.cpp
[cpp] view plain copy
// memory pool test.cpp : Defines the entry point for the console application.  
#include <tchar.h>  
#include "MemoryPool.h"  
#include <iostream>  
#include <windows.h>  
#include <vector>  
#include <time.h>  
#include <math.h>  
#include <fstream>  
using namespace std;  
int break_time = 0;  
// 检测内存池相关参数  
void check_mem_pool(int& max_chunk_size, int& free_chunk_count, int& min_chunk_size, int& total_free_mem, MEMORYPOOL* mem_pool)  
{  
    memory_chunk* head = mem_pool->pfree_mem_chunk;  
    memory_chunk* tmp = head;  
    free_chunk_count = 0;  
    total_free_mem = 0;  
    max_chunk_size = 0;  
    min_chunk_size = 500*1024*1024;  
    if (head == NULL)  
    {  
        min_chunk_size = 0;  
        return;  
    }  
    while (tmp->next != head)  
    {  
        free_chunk_count++;  
        total_free_mem += tmp->pfree_mem_addr->count * MINUNITSIZE;  
        if (tmp->pfree_mem_addr->count * MINUNITSIZE > max_chunk_size )  
        {  
            max_chunk_size = tmp->pfree_mem_addr->count * MINUNITSIZE;  
        }  
        if (tmp->pfree_mem_addr->count * MINUNITSIZE < min_chunk_size)  
        {  
            min_chunk_size = tmp->pfree_mem_addr->count * MINUNITSIZE;  
        }  
        tmp = tmp->next;  
    }  
    free_chunk_count++;  
    total_free_mem += tmp->pfree_mem_addr->count * MINUNITSIZE;  
    if (tmp->pfree_mem_addr->count * MINUNITSIZE > max_chunk_size )  
    {  
        max_chunk_size = tmp->pfree_mem_addr->count * MINUNITSIZE;  
    }  
    if (tmp->pfree_mem_addr->count * MINUNITSIZE < min_chunk_size)  
    {  
        min_chunk_size = tmp->pfree_mem_addr->count * MINUNITSIZE;  
    }  
}  
// 申请后紧接着释放  
double test_mem_pool_perf_1(PMEMORYPOOL mem_pool, int iter, int* sizes)  
{  
    cout << "*********************test_mem_pool_perf_1*********************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    for (int i = 0; i < iter; i++)  
    {  
        void *p = GetMemory(sizes[i], mem_pool);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << sizes[i] << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
        FreeMemory(p,  mem_pool);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_1: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "*********************test_mem_pool_perf_1*********************" <<  endl << endl << endl;  
    return t;  
}  
double test_std_perf_1(int iter, int* sizes)  
{  
    cout << "*********************test_std_perf_1*********************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    for (int i = 0; i < iter; i++)  
    {  
        void *p = malloc(sizes[i]);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << sizes[i] << " Byte" << endl;  
            break;  
        }  
        free(p);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf_1: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "*********************test_std_perf_1*********************" <<  endl << endl << endl;  
    return t;  
}  
// 连续申请iter/2次，然后释放所有申请内存；再重复一次  
double test_mem_pool_perf_2(PMEMORYPOOL mem_pool, int iter, int size)  
{  
    cout << "*********************test_mem_pool_perf_2*********************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    void **p = new void*[iter];  
    if (p == NULL)  
    {  
        cout << "new faild" << endl;  
        return -1;  
    }  
    int count = 0;  
    for (int i = 0; i < iter/2; i++)  
    {  
        p[i] = GetMemory(size, mem_pool);  
        if (p[i] == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
        count++;  
    }  
    for (int i = 0; i < count; i++)  
    {  
        FreeMemory(p[i],  mem_pool);  
    }  
    count = 0;  
    for (int i = 0; i < iter/2; i++)  
    {         
        p[i] = GetMemory(size, mem_pool);  
        if (p[i] == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
              
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
        count++;  
    }  
    for (int i = 0; i < count; i++)  
    {  
        if (p[i] == NULL)  
        {  
            cout << i << endl;  
            break;  
        }  
        FreeMemory(p[i],  mem_pool);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_2: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    delete []p;  
    cout << "*********************test_mem_pool_perf_2*********************" <<  endl << endl << endl;  
    return t;  
}  
// 连续申请inner_iter次，释放；重复iter/inner_iter次  
double test_mem_pool_perf_3(PMEMORYPOOL mem_pool, int iter, int size)  
{  
    cout << "*********************test_mem_pool_perf_3*********************" << endl;  
    int inner_iter = 10;  
    void **p = new void*[inner_iter];  
    if (p == NULL)  
    {  
        cout << "new faild" << endl;  
        return -1;  
    }  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2, start, finish;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    for (int k = 0; k < iter / inner_iter; k++)  
    {  
        int j = 0;  
        for (j = 0; j < inner_iter; j++)  
        {  
            p[j] = GetMemory(size, mem_pool);  
            if (p[j] == NULL)  
            {  
                cout << "break @ iterator = " << j << " / " << iter << ",    need memory " << size << " Byte" << endl;  
                cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
                cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
                cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
                int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
                check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
                cout << "check memory pool result:" << endl;  
                cout << "free_chunk_count:    " << free_chunk_count << endl  
                    << "total_free_mem:   " << total_free_mem << endl  
                    << "max_chunk_size:   " << max_chunk_size << endl  
                    << "min_chunk_size:   " << min_chunk_size << endl;  
                break;  
            }  
        }  
        for (int i = 0; i < j; i++)  
        {  
                FreeMemory(p[i],  mem_pool);          
        }  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_3: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "*********************test_mem_pool_perf_3*********************" <<  endl << endl << endl;  
    return t;  
}  
// 随机内存大小，随机释放操作  
double test_mem_pool_perf_rand(PMEMORYPOOL mem_pool, int iter, int* sizes, int* instruction)  
{  
    cout << "-----------------------test_mem_pool_perf_rand----------------------- "<< endl;  
    void** p = new void*[iter];  
    if (p == NULL)  
    {  
        cout << "new failed" << endl;  
        return -1;  
    }  
    LARGE_INTEGER litmp, gftime;   
    LONGLONG QPart1, QPart2, start, finish;  
    double t, GetMemory_time, FreeMemory_time;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    int index = 0;  
    int size;  
    int free_tmp = 0;  
    double seach_time;  
    for (int i = 0; i < iter; i++)  
    {  
        size = sizes[i];  
        p[index++] = GetMemory(size, mem_pool);  
        if (p[index-1] == NULL)  
        {             
            break_time++;  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
          
        if (instruction[i] == 1)  
        {             
            FreeMemory(p[--index],  mem_pool);  
        }     
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_rand: iter = " << iter << endl;  
    cout << "time: " << t << endl << endl;  
    delete []p;  
    return t;  
}  
double test_std_perf(int iter, int* sizes, int* instruction)  
{  
    cout << "test_std_perf" << endl;  
    void** p =new void*[iter];  
    if (p == NULL)  
    {  
        cout << "new failed" << endl;  
        return -1;  
    }  
      
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
//  cout << "test start" << endl;  
    int index = 0;  
    int size;  
    for (int i = 0; i < iter; i++)  
    {  
        size = sizes[i];  
        p[index++] = malloc(size);  
        if (p[index-1] == NULL)  
        {  
            cout << i << endl;  
            break;  
        }  
        if (instruction[i] == 1)  
        {  
            free(p[--index]);  
        }         
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf: iter = " << iter << endl;  
    cout << "time: " << t << endl << endl;  
    for (int k = 0; k < index; k++)  
    {  
        free(p[k]);  
    }  
    return t;  
}  
double test_std_perf_fix_size(int iter, int size)  
{  
    cout << "******************* test_std_perf_fix_size *******************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    int index = 0;  
      
    for (int i = 0; i < iter; i++)  
    {  
        void *p = malloc(size);  
        if (p == NULL)  
        {  
            cout << i << endl;  
            break;  
        }  
        free(p);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "******************* test_std_perf_fix_size *******************" << endl << endl << endl;  
    return t;  
}  
void test_correct_1(PMEMORYPOOL mem_pool, int iter, int size)  
{  
    vector<void*>vec;  
    vector<void*>::iterator vec_iter;  
    int i = 0;  
    cout << "**************************** Get Memory Test Start ****************************"<< endl << endl;  
    for (i = 0; i < iter; i++)  
    {  
        void *p = GetMemory(size, mem_pool);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl << endl;  
            break;  
        }  
        vec.push_back(p);  
    }  
    cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
    cout << "memory left is: " << mem_pool->size << " Byte" << endl;  
    cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl << endl;  
    cout << "verify memory size" << endl;  
    memory_chunk* tmp = mem_pool->pfree_mem_chunk;  
    int free_size = 0;  
    for (int k = 0; k < mem_pool->free_mem_chunk_count; k++)  
    {  
        free_size += tmp->pfree_mem_addr->count * MINUNITSIZE;  
        tmp = tmp->next;  
    }  
    cout << "memory free size is " << free_size << " Byte" << endl;  
    cout << "memory used size is " << mem_pool->mem_used_size << " Byte" << endl;  
    cout << "*************************** Get Memory Test Finish ***************************"<< endl << endl;  
    cout << "*************************** Free Memory Test Start ***************************"<< endl << endl;  
    int index = 0;  
    for (vec_iter = vec.begin(); vec_iter != vec.end(); vec_iter++)  
    {  
        index++;  
        FreeMemory(*vec_iter, mem_pool);  
    }  
    cout << "memory left is: " << mem_pool->size << " Byte" << endl;  
    cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl << endl;  
    cout << "*************************** Free Memory Test Finish ***************************"<< endl << endl;  
    cout << "********************* Get Memory Test (after Free) Start *********************"<< endl << endl;  
    for (i = 0; i < iter; i++)  
    {  
        void *p = GetMemory(size, mem_pool);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl << endl;  
            int max_size = 0;  
            memory_chunk* tmp = mem_pool->pfree_mem_chunk;  
            for (int k = 0; k < mem_pool->free_mem_chunk_count; k++)  
            {  
                if (tmp->pfree_mem_addr->count * MINUNITSIZE > max_size)  
                {  
                    max_size = tmp->pfree_mem_addr->count * MINUNITSIZE > max_size;  
                }  
            }  
            cout << "max chunk size is: " << max_size << " Byte" << endl;  
            break;  
        }  
        vec.push_back(p);  
    }  
    cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
    cout << "memory left is: " << mem_pool->size << " Byte" << endl;  
    cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl << endl;  
    cout << "verify memory size" << endl;  
    tmp = mem_pool->pfree_mem_chunk;  
    free_size = 0;  
    for (int k = 0; k < mem_pool->free_mem_chunk_count; k++)  
    {  
        free_size += tmp->pfree_mem_addr->count * MINUNITSIZE;  
        tmp = tmp->next;  
    }  
    cout << "memory free size is " << free_size << " Byte" << endl;  
    cout << "memory used size is " << mem_pool->mem_used_size << " Byte" << endl;  
    cout << "********************* Get Memory Test (after Free) Finish *********************"<< endl << endl;  
}  
/************************************************************************/  
/* 内存池性能测试代码 
 * 固定大小 
 /************************************************************************/  
/* 
void test_mem_pool_fix_size(PMEMORYPOOL mem_pool) 
{ 
    int iter = 200000; 
    int size = 512; 
    double t1 = test_std_perf_fix_size(iter, size); 
    double t2 = test_mem_pool_perf_1(mem_pool, iter, size); 
    double t3 = test_mem_pool_perf_2(mem_pool, iter, size); 
    double t4 = test_mem_pool_perf_3(mem_pool, iter, size); 
    cout  << endl << endl  
        << "test count: " << iter << ", test size: " << size << endl 
        << "test result (system time / mem_pool time) : " << endl; 
    cout << "test_mem_pool_perf_1:    " << t1 / t2 << endl 
        << "test_mem_pool_perf_2: " << t1 / t3 << endl 
        << "test_mem_pool_perf_3: " << t1 / t4 << endl; 
} 
*/  
/************************************************************************/  
/* 内存池性能测试代码 
  
 * 随机大小，随机释放操作 
/************************************************************************/  
void rand_test()  
{  
    size_t sBufSize = 500* 1024*1024;  
    void*pBuf = malloc(sBufSize);  
    if (pBuf == NULL)  
    {  
        cout << "malloc failed" << endl;  
        return;  
    }  
    PMEMORYPOOL mem_pool = CreateMemoryPool(pBuf, sBufSize);  
    ofstream out("rand_test.txt");  
    int iter = 2000;  
    int* instruction = new int[iter];  
    int* sizes = new int[iter];   
    if (instruction == NULL || sizes == NULL)  
    {  
        cout << "new memory failed" << endl;  
        return;  
    }  
    srand(time(NULL));   
    cout << "generate rand number" << endl;  
    // instruction 中元素为1时表示在GetMemory后执行FreeMemory，0表示不执行FreeMemory  
    // sizes中是每次分配内存的大小，范围从64B~1024B  
    for (int i = 0; i < iter; i++)  
    {  
        instruction[i] = rand() % 2;  
        sizes[i] = (rand() % 16 + 1) * 64;  
    }  
    int test_count = 200;  
    double t1, t2;  
    double* ratio = new double[test_count];  
    int count = 0;  
    for (int k = 0; k < test_count; k++)  
    {  
        if (break_time != 0)  
        {  
            cout << "break @ " << k << " / " << test_count << endl;  
            break;  
        }  
        count++;  
        cout << "******************************************test " << k+1 << " *************************************************" << endl;  
        t1 = test_std_perf(iter, sizes, instruction);  
        t2 = test_mem_pool_perf_rand(mem_pool, iter,  sizes, instruction);  
        cout << "total memory: " << mem_pool->size << ", memory used: " << mem_pool->mem_used_size   
            << ", memory left: " << mem_pool->size - mem_pool->mem_used_size  << endl;  
        ratio[k] = t1 / t2;  
          
    }  
    if(break_time == 0)  
        break_time = test_count;  
    break_time = count - 1;  
    cout << "*************************** ratio (system time / mem_pool time) ***************************" << endl;  
    for (int k = 0; k < break_time; k++)  
    {  
        out << ratio[k] << ",";  
        if (k % 10 == 0 && k != 0)  
        {  
            cout << endl;  
        }  
        cout << ratio[k] << " ";  
    }  
    cout << endl;  
    delete []ratio;  
    delete []instruction;  
    delete []sizes;  
    free(pBuf);  
}  
// 申请紧接着释放  
void rand_test_2()  
{  
    size_t sBufSize = 500* 1024*1024;  
    void*pBuf = malloc(sBufSize);  
    if (pBuf == NULL)  
    {  
        cout << "malloc failed" << endl;  
        return;  
    }  
    PMEMORYPOOL mem_pool = CreateMemoryPool(pBuf, sBufSize);  
    int iter = 2000;  
    int test_count = 511;  
    int* sizes = new int[iter];   
    double* ratio = new double[test_count];  
    if (sizes == NULL || ratio == NULL)  
    {  
        cout << "new memory failed" << endl;  
        return;  
    }  
    srand(time(NULL));   
    cout << "generate rand number" << endl;  
    ofstream out("rand_test_2.txt");  
    for (int k = 0; k < test_count; k++)  
    {  
        for (int i = 0; i < iter; i++)  
        {  
            sizes[i] = (rand() % 16 + 1) * 64 + 1024 * k;  
        }  
        double mem_pool_t = test_mem_pool_perf_1(mem_pool, iter, sizes);  
        double std_t = test_std_perf_1(iter, sizes);  
          
        ratio[k] = std_t / mem_pool_t;  
    }  
    cout << "*************************** ratio (system time / mem_pool time) ***************************" << endl;  
    for (int k = 0; k < test_count; k++)  
    {  
        out << ratio[k] << ",";  
        if (k % 10 == 0 && k != 0)  
        {  
            cout << endl;  
        }  
        cout << ratio[k] << " ";  
    }  
    cout << endl;  
      
    delete []sizes;  
    delete ratio;  
    free(pBuf);  
}  
int _tmain(int argc, _TCHAR* argv[])  
{  
    rand_test();  
//  rand_test_2();  
      
    return 0;  
}  

/***********************************************************************************
 * 大顶堆结构内存池：
***********************************************************************************/
	MemoryPool.h
[cpp] view plain copy
#ifndef _MEMORYPOOL_H  
#define _MEMORYPOOL_H  
#include <stdlib.h>  
#define MINUNITSIZE 64  
#define ADDR_ALIGN 8  
#define SIZE_ALIGN MINUNITSIZE  
#define MAXCHUNKSIZE 1024*1024*64  
struct memory_chunk;  
typedef struct memory_block  
{  
    size_t count;  
    size_t start;  
    memory_chunk* pmem_chunk;  
}memory_block;  
typedef struct memory_chunk  
{  
    size_t chunk_size;  
    memory_block* pfree_mem_addr;  
}memory_chunk;  
typedef struct max_heap  
{  
    memory_chunk *heap;    
    size_t maxSize;     
    size_t currentSize;    
}max_heap;  
typedef struct MEMORYPOOL  
{  
    void *memory;  
    size_t size;  
    memory_block* pmem_map;   
    max_heap heap;  
    size_t mem_used_size; // 记录内存池中已经分配给用户的内存的大小  
    size_t free_mem_chunk_count; // 记录 pfree_mem_chunk链表中的单元个数  
    size_t mem_map_unit_count; //   
    size_t mem_block_count; // 一个 mem_unit 大小为 MINUNITSIZE  
}MEMORYPOOL, *PMEMORYPOOL;  
/************************************************************************/  
/* 生成内存池 
* pBuf: 给定的内存buffer起始地址 
* sBufSize: 给定的内存buffer大小 
* 返回生成的内存池指针 
/************************************************************************/  
PMEMORYPOOL CreateMemoryPool(void* pBuf, size_t sBufSize);  
/************************************************************************/  
/* 暂时没用 
/************************************************************************/  
void ReleaseMemoryPool(PMEMORYPOOL* ppMem) ;   
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
* pMem: 内存池 指针 
* sMemorySize: 要分配的内存大小 
* 成功时返回分配的内存起始地址，失败返回NULL 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, PMEMORYPOOL pMem) ;  
/************************************************************************/  
/* 从内存池中释放申请到的内存 
* pMem：内存池指针 
* ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void FreeMemory(void *ptrMemoryBlock, PMEMORYPOOL pMem) ;  
#endif //_MEMORYPOOL_H  
 
	MemoryPool.cpp
[cpp] view plain copy
#include <memory.h>  
#include "MemoryPool.h"  
/************************************************************************/  
/* 以下为大顶堆操作*/                                                                          
void init_max_heap(size_t max_heap_size, memory_chunk* heap_arr, max_heap* heap)  
{  
    heap->maxSize = max_heap_size;  
    heap->currentSize = 0;  
    heap->heap = heap_arr;  
}  
bool is_heap_empty(max_heap* heap)  
{  
    return heap->currentSize == 0;    
}  
bool is_heap_full(max_heap* heap)  
{  
    return heap->currentSize == heap->maxSize;    
}  
memory_chunk* filter_up(max_heap* heap, size_t start)  
{  
    size_t i = start;  
    size_t j = ( i - 1 ) / 2;    
    memory_chunk temp = heap->heap[i];    
    while(i > 0)  
    {    
        if(temp.chunk_size <= heap->heap[j].chunk_size)  
            break;    
        else  
        {             
            heap->heap[i] = heap->heap[j];    
            heap->heap[j].pfree_mem_addr->pmem_chunk = &(heap->heap[i]);  
            i = j;    
            j = (i - 1) / 2;    
        }    
    }    
    heap->heap[i] = temp;    
    return &(heap->heap[i]);  
}  
memory_chunk* filter_down(max_heap* heap, size_t start, size_t endOfHeap)  
{  
    size_t i = start;  
    size_t j = i * 2 + 1;    
    memory_chunk temp = heap->heap[i];    
    while(j <= endOfHeap)  
    {    
        if(j < endOfHeap && heap->heap[j].chunk_size < heap->heap[j+1].chunk_size)  
            j++;    
        if(temp.chunk_size > heap->heap[j].chunk_size)  
            break;    
        else  
        {    
            heap->heap[i] = heap->heap[j];    
            heap->heap[j].pfree_mem_addr->pmem_chunk = &(heap->heap[i]);  
            i = j;    
            j = 2 * i + 1;    
        }    
    }    
    heap->heap[i] = temp;    
    return &(heap->heap[i]);  
}  
memory_chunk* insert_heap(memory_chunk& chunk, max_heap* heap)  
{  
    if (is_heap_full(heap))  
    {  
        return NULL;  
    }  
    heap->heap[heap->currentSize] = chunk;  
    memory_chunk* ret = filter_up(heap, heap->currentSize);  
    heap->currentSize++;    
    return ret;    
}  
bool get_max(memory_chunk*& chunk, max_heap* heap)  
{  
    if(is_heap_empty(heap))  
    {    
        return false;    
    }    
    chunk = heap->heap;    
    return true;  
}  
bool remove_max(max_heap* heap)  
{  
    if(is_heap_empty(heap))  
    {    
        return false;    
    }    
    heap->heap[0] = heap->heap[heap->currentSize - 1];    
    heap->currentSize--;    
    if (heap->currentSize > 0)  
    {  
        filter_down(heap, 0, heap->currentSize-1);    
    }  
    return true;    
}  
void remove_element(memory_chunk* chunk, max_heap* heap)  
{  
    // 删除某个非max元素有两步组成：  
    // 1. 将该元素size增至最大（大于max element），然后将其上移至堆顶；  
    // 2. 删除堆顶元素  
    size_t index = chunk - heap->heap;  
    chunk->chunk_size = MAXCHUNKSIZE;  
    filter_up(heap, index);  
    remove_max(heap);  
}  
memory_chunk* increase_element_value(memory_chunk* chunk, max_heap* heap, size_t increase_value)  
{  
    size_t index = chunk - heap->heap;  
    chunk->chunk_size += increase_value;  
    return filter_up(heap, index);  
}  
memory_chunk* decrease_element_value(memory_chunk* chunk, max_heap* heap, size_t decrease_value)  
{  
    size_t index = chunk - heap->heap;  
    chunk->chunk_size -= decrease_value;  
    return filter_down(heap, index, heap->currentSize-1);  
}  
/************************************************************************/  
/* 内存池起始地址对齐到ADDR_ALIGN字节 
/************************************************************************/  
size_t check_align_addr(void*& pBuf)  
{  
    size_t align = 0;  
    size_t addr = (int)pBuf;  
    align = (ADDR_ALIGN - addr % ADDR_ALIGN) % ADDR_ALIGN;  
    pBuf = (char*)pBuf + align;  
    return align;  
}  
/************************************************************************/  
/* 内存block大小对齐到MINUNITSIZE字节 
/************************************************************************/  
size_t check_align_block(size_t size)  
{  
    size_t align = size % MINUNITSIZE;  
      
    return size - align;   
}  
/************************************************************************/  
/* 分配内存大小对齐到SIZE_ALIGN字节 
/************************************************************************/  
size_t check_align_size(size_t size)  
{  
    size = (size + SIZE_ALIGN - 1) / SIZE_ALIGN * SIZE_ALIGN;  
    return size;  
}  
/************************************************************************/  
/* 内存映射表中的索引转化为内存起始地址                                                                     
/************************************************************************/  
void* index2addr(PMEMORYPOOL mem_pool, size_t index)  
{  
    char* p = (char*)(mem_pool->memory);  
    void* ret = (void*)(p + index *MINUNITSIZE);  
      
    return ret;  
}  
/************************************************************************/  
/* 内存起始地址转化为内存映射表中的索引                                                                     
/************************************************************************/  
size_t addr2index(PMEMORYPOOL mem_pool, void* addr)  
{  
    char* start = (char*)(mem_pool->memory);  
    char* p = (char*)addr;  
    size_t index = (p - start) / MINUNITSIZE;  
    return index;  
}  
/************************************************************************/  
/* 生成内存池 
* pBuf: 给定的内存buffer起始地址 
* sBufSize: 给定的内存buffer大小 
* 返回生成的内存池指针 
/************************************************************************/  
PMEMORYPOOL CreateMemoryPool(void* pBuf, size_t sBufSize)  
{  
    memset(pBuf, 0, sBufSize);  
    PMEMORYPOOL mem_pool = (PMEMORYPOOL)pBuf;  
    // 计算需要多少memory map单元格  
    size_t mem_pool_struct_size = sizeof(MEMORYPOOL);  
    mem_pool->mem_map_unit_count = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;  
    mem_pool->pmem_map = (memory_block*)((char*)pBuf + mem_pool_struct_size);  
    size_t max_heap_size = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;  
    memory_chunk* heap_arr = (memory_chunk*)((char*)pBuf + mem_pool_struct_size + sizeof(memory_block) * mem_pool->mem_map_unit_count);    
      
    mem_pool->memory = (char*)pBuf + mem_pool_struct_size+ sizeof(memory_block) * mem_pool->mem_map_unit_count + sizeof(memory_chunk) * max_heap_size;  
    mem_pool->size = sBufSize - mem_pool_struct_size - sizeof(memory_block) * mem_pool->mem_map_unit_count - sizeof(memory_chunk) * max_heap_size;  
    size_t align = check_align_addr(mem_pool->memory);  
    mem_pool->size -= align;  
    mem_pool->size = check_align_block(mem_pool->size);  
    mem_pool->mem_block_count = mem_pool->size / MINUNITSIZE;  
    init_max_heap(mem_pool->mem_block_count, heap_arr, &(mem_pool->heap));  
    memory_chunk chunk;  
    chunk.chunk_size = mem_pool->mem_block_count;  
    memory_chunk* pos = insert_heap(chunk, &(mem_pool->heap));  
      
    // 初始化 pmem_map  
    mem_pool->pmem_map[0].count = mem_pool->mem_block_count;  
    mem_pool->pmem_map[0].pmem_chunk = pos;  
    mem_pool->pmem_map[mem_pool->mem_block_count-1].start = 0;  
      
    pos->pfree_mem_addr = mem_pool->pmem_map;  
    mem_pool->mem_used_size = 0;  
    return mem_pool;  
}  
/************************************************************************/  
/* 暂时没用 
/************************************************************************/  
void ReleaseMemoryPool(PMEMORYPOOL* ppMem)   
{  
}  
/************************************************************************/  
/* 从内存池中分配指定大小的内存  
* pMem: 内存池 指针 
* sMemorySize: 要分配的内存大小 
* 成功时返回分配的内存起始地址，失败返回NULL 
/************************************************************************/  
void* GetMemory(size_t sMemorySize, PMEMORYPOOL pMem)  
{  
    sMemorySize = check_align_size(sMemorySize);  
    size_t index = 0;  
    memory_chunk* max_chunk;  
    bool ret = get_max(max_chunk, &(pMem->heap));  
    if (ret == false || max_chunk->chunk_size * MINUNITSIZE < sMemorySize)  
    {  
        return NULL;  
    }  
    pMem->mem_used_size += sMemorySize;  
    if (max_chunk->chunk_size * MINUNITSIZE == sMemorySize)  
    {  
        // 当要分配的内存大小与当前chunk中的内存大小相同时，从pfree_mem_chunk链表中删除此chunk  
        size_t current_index = (max_chunk->pfree_mem_addr - pMem->pmem_map);  
        remove_max(&(pMem->heap));  
          
        return index2addr(pMem, current_index);  
    }  
    else  
    {  
        // 当要分配的内存小于当前chunk中的内存时，更改pfree_mem_chunk中相应chunk的pfree_mem_addr  
          
        // 复制当前mem_map_unit  
        memory_block copy;  
        copy.count = max_chunk->pfree_mem_addr->count;  
        copy.pmem_chunk = max_chunk;  
        // 记录该block的起始和结束索引  
        memory_block* current_block = max_chunk->pfree_mem_addr;  
        current_block->count = sMemorySize / MINUNITSIZE;  
        size_t current_index = (current_block - pMem->pmem_map);  
        pMem->pmem_map[current_index+current_block->count-1].start = current_index;  
        current_block->pmem_chunk = NULL; // NULL表示当前内存块已被分配  
        // 当前block被一分为二，更新第二个block中的内容  
        memory_chunk* pos = decrease_element_value(max_chunk, &(pMem->heap), current_block->count);  
        pMem->pmem_map[current_index+current_block->count].count = copy.count - current_block->count;  
        pMem->pmem_map[current_index+current_block->count].pmem_chunk = pos;  
        // 更新原来的pfree_mem_addr  
        pos->pfree_mem_addr = &(pMem->pmem_map[current_index+current_block->count]);  
      
        size_t end_index = current_index + copy.count - 1;  
        pMem->pmem_map[end_index].start = current_index + current_block->count;  
        return index2addr(pMem, current_index);  
    }     
}  
/************************************************************************/  
/* 从内存池中释放申请到的内存 
* pMem：内存池指针 
* ptrMemoryBlock：申请到的内存起始地址 
/************************************************************************/  
void FreeMemory(void *ptrMemoryBlock, PMEMORYPOOL pMem)   
{  
    size_t current_index = addr2index(pMem, ptrMemoryBlock);  
    size_t size = pMem->pmem_map[current_index].count * MINUNITSIZE;  
    // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并  
    memory_block* pre_block = NULL;  
    memory_block* next_block = NULL;  
    memory_block* current_block = &(pMem->pmem_map[current_index]);  
    // 第一个  
    if (current_index == 0)  
    {  
        if (current_block->count < pMem->mem_block_count)  
        {  
            next_block = &(pMem->pmem_map[current_index+current_block->count]);  
            // 如果后一个内存块是空闲的，合并  
            if (next_block->pmem_chunk != NULL)  
            {  
                memory_chunk* pos = increase_element_value(next_block->pmem_chunk, &(pMem->heap), current_block->count);  
                pos->pfree_mem_addr = current_block;  
                pMem->pmem_map[current_index+current_block->count+next_block->count-1].start = current_index;  
                current_block->count += next_block->count;  
                current_block->pmem_chunk = pos;  
                next_block->pmem_chunk = NULL;  
            }  
            // 如果后一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {  
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = insert_heap(new_chunk, &(pMem->heap));  
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  
        {  
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = insert_heap(new_chunk, &(pMem->heap));  
            current_block->pmem_chunk = pos;  
        }         
    }  
      
    // 最后一个  
    else if (current_index == pMem->mem_block_count-1)  
    {  
        if (current_block->count < pMem->mem_block_count)  
        {  
            pre_block = &(pMem->pmem_map[current_index-1]);  
            size_t index = pre_block->count;  
            pre_block = &(pMem->pmem_map[index]);  
              
            // 如果前一个内存块是空闲的，合并  
            if (pre_block->pmem_chunk != NULL)  
            {  
                memory_chunk* pos = increase_element_value(pre_block->pmem_chunk, &(pMem->heap), current_block->count);  
                pre_block->pmem_chunk = pos;  
                pMem->pmem_map[current_index+current_block->count-1].start = current_index - pre_block->count;  
                pre_block->count += current_block->count;  
                current_block->pmem_chunk = NULL;  
            }  
            // 如果前一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk  
            else  
            {  
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = insert_heap(new_chunk, &(pMem->heap));  
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  
        {  
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = insert_heap(new_chunk, &(pMem->heap));  
            current_block->pmem_chunk = pos;  
        }  
    }  
    else  
    {         
        next_block = &(pMem->pmem_map[current_index+current_block->count]);  
        pre_block = &(pMem->pmem_map[current_index-1]);  
        size_t index = pre_block->start;  
        pre_block = &(pMem->pmem_map[index]);  
        bool is_back_merge = false;  
        if (next_block->pmem_chunk == NULL && pre_block->pmem_chunk == NULL)  
        {  
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = insert_heap(new_chunk, &(pMem->heap));  
            current_block->pmem_chunk = pos;  
        }  
        // 后一个内存块  
        if (next_block->pmem_chunk != NULL)  
        {  
            memory_chunk* pos = increase_element_value(next_block->pmem_chunk, &(pMem->heap), current_block->count);  
            pos->pfree_mem_addr = current_block;  
            pMem->pmem_map[current_index+current_block->count+next_block->count-1].start = current_index;  
            current_block->count += next_block->count;  
            current_block->pmem_chunk = pos;  
            next_block->pmem_chunk = NULL;  
            is_back_merge = true;  
        }  
        // 前一个内存块  
        if (pre_block->pmem_chunk != NULL)  
        {  
            pMem->pmem_map[current_index+current_block->count-1].start = current_index - pre_block->count;  
            pre_block->count += current_block->count;  
            memory_chunk* pos = increase_element_value(pre_block->pmem_chunk, &(pMem->heap), current_block->count);  
            pre_block->pmem_chunk = pos;  
            pos->pfree_mem_addr = pre_block;  
            if (is_back_merge)  
            {  
                remove_element(current_block->pmem_chunk, &(pMem->heap));  
            }  
            current_block->pmem_chunk = NULL;              
        }     
    }  
    pMem->mem_used_size -= size;  
}  
 
	MemoryPoolTest.cpp
[cpp] view plain copy
// memory pool test.cpp : Defines the entry point for the console application.  
//  
#include <tchar.h>  
#include "MemoryPool.h"  
#include <iostream>  
#include <windows.h>  
#include <vector>  
#include <time.h>  
#include <math.h>  
#include <fstream>  
using namespace std;  
int break_time = 0;  
size_t used_size = 0;  
void output_heap(max_heap* heap, ofstream& out)  
{  
    for (int k = 0; k < heap->currentSize; k++)  
    {  
        if (k != 0 && k % 10 == 0)  
        {  
            out << endl;  
        }  
        out << heap->heap[k].chunk_size << " ";  
    }  
    out << endl;  
}  
void check_mem_pool(int& max_chunk_size, int& free_chunk_count, int& min_chunk_size, int& total_free_mem, MEMORYPOOL* mem_pool)  
{  
    total_free_mem = 0;  
    max_chunk_size = 0;  
    min_chunk_size = 1024*1024*1024;  
    free_chunk_count = mem_pool->heap.currentSize;  
      
    size_t size;  
    for (int k = 0; k < free_chunk_count; k++)  
    {  
        size = mem_pool->heap.heap[k].chunk_size * MINUNITSIZE;  
        total_free_mem += size;  
        if (size > max_chunk_size)  
        {  
            max_chunk_size = size;  
        }  
        if (size < min_chunk_size)  
        {  
            min_chunk_size = size;  
        }  
    }  
}  
// 申请后紧接着释放  
double test_mem_pool_perf_1(PMEMORYPOOL mem_pool, int iter, int* sizes)  
{  
    cout << "*********************test_mem_pool_perf_1*********************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    for (int i = 0; i < iter; i++)  
    {  
        void *p = GetMemory(sizes[i], mem_pool);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << sizes[i] << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
        FreeMemory(p,  mem_pool);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_1: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "*********************test_mem_pool_perf_1*********************" <<  endl << endl << endl;  
    return t;  
}  
double test_std_perf_1(int iter, int* sizes)  
{  
    cout << "*********************test_std_perf_1*********************" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    for (int i = 0; i < iter; i++)  
    {  
        void *p = malloc(sizes[i]);  
        if (p == NULL)  
        {  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << sizes[i] << " Byte" << endl;  
            break;  
        }  
        free(p);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf_1: iter = " << iter << endl;  
    cout << "time: " << t << endl;  
    cout << "*********************test_std_perf_1*********************" <<  endl << endl << endl;  
    return t;  
}  
double test_mem_pool_perf_rand(PMEMORYPOOL mem_pool, int iter, int* sizes, int* instruction)  
{  
    cout << "-----------------------test_mem_pool_perf_rand----------------------- "<< endl;  
    void** p = new void*[iter];  
    if (p == NULL)  
    {  
        cout << "new failed" << endl;  
        return -1;  
    }  
    cout << "test start" << endl;  
    LARGE_INTEGER litmp, gftime;   
    LONGLONG QPart1, QPart2, start, finish;  
    double t, GetMemory_time, FreeMemory_time;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    int index = 0;  
    int size;  
    int free_tmp = 0;  
    double seach_time;  
    for (int i = 0; i < iter; i++)  
    {  
        size = sizes[i];  
        p[index++] = GetMemory(size, mem_pool);  
        if (p[index-1] == NULL)  
        {  
            break_time++;  
            cout << "break @ iterator = " << i << " / " << iter << ",    need memory " << size << " Byte" << endl;  
            cout << "total memory is: " << mem_pool->size << " Byte" << endl;  
            cout << "memory used is: " << mem_pool->mem_used_size << " Byte" << endl;  
            cout << "memory left is: " << mem_pool->size -  mem_pool->mem_used_size  << endl << endl;  
            int max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem;  
            check_mem_pool(max_chunk_size, free_chunk_count, min_chunk_size, total_free_mem,  mem_pool);  
            cout << "check memory pool result:" << endl;  
            cout << "free_chunk_count:    " << free_chunk_count << endl  
                << "total_free_mem:   " << total_free_mem << endl  
                << "max_chunk_size:   " << max_chunk_size << endl  
                << "min_chunk_size:   " << min_chunk_size << endl;  
            break;  
        }  
        used_size += size;  
        if (instruction[i] == 1)  
        {             
            FreeMemory(p[--index],  mem_pool);  
            used_size -= size;  
        }     
    }  
      
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_mem_pool_perf_rand: iter = " << iter << endl;  
    cout << "time: " << t << endl << endl;  
    delete []p;  
    return t;  
}  
double test_std_perf(int iter, int* sizes, int* instruction)  
{  
    cout << "test_std_perf" << endl;  
    void** p =new void*[iter];  
    if (p == NULL)  
    {  
        cout << "new failed" << endl;  
        return -1;  
    }  
      
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
//  cout << "test start" << endl;  
    int index = 0;  
    int size;  
    for (int i = 0; i < iter; i++)  
    {  
        size = sizes[i];  
        p[index++] = malloc(size);  
        if (p[index-1] == NULL)  
        {  
            cout << i << endl;  
            break;  
        }  
        if (instruction[i] == 1)  
        {  
            free(p[--index]);  
        }         
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf: iter = " << iter << endl;  
    cout << "time: " << t << endl << endl;  
    for (int k = 0; k < index; k++)  
    {  
        free(p[k]);  
    }  
    return t;  
}  
double test_std_perf_fix_size(int iter, int size)  
{  
    cout << "test_std_perf_fix_size" << endl;  
    LARGE_INTEGER litmp;   
    LONGLONG QPart1, QPart2;  
    double t;  
    double dfMinus, dfFreq;   
    QueryPerformanceFrequency(&litmp);  
    dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率  
    QueryPerformanceCounter(&litmp);  
    QPart1 = litmp.QuadPart;// 获得初始值  
    int index = 0;  
      
    for (int i = 0; i < iter; i++)  
    {  
        void *p = malloc(size);  
        if (p == NULL)  
        {  
            cout << i << endl;  
            break;  
        }  
        free(p);  
    }  
    QueryPerformanceCounter(&litmp);  
    QPart2 = litmp.QuadPart;//获得中止值  
    dfMinus = (double)(QPart2-QPart1);  
    t = dfMinus / dfFreq;// 获得对应的时间值，单位为秒     
    cout << "test_std_perf: iter = " << iter << endl;  
    cout << "time: " << t << endl << endl;  
    return t;  
}  
void rand_test()  
{  
    ofstream out("rand_test.txt");  
    used_size = 0;  
    int iter = 2000;  
    size_t sBufSize = 500* 1024*1024;  
    void*pBuf = malloc(sBufSize);  
    if (pBuf == NULL)  
    {  
        cout << "malloc failed" << endl;  
        return;  
    }  
    PMEMORYPOOL mem_pool = CreateMemoryPool(pBuf, sBufSize);  
    int* instruction = new int[iter];  
    int* sizes = new int[iter];   
    if (instruction == NULL || sizes == NULL)  
    {  
        cout << "new failed" << endl;  
        return;  
    }  
    srand(time(NULL));   
    ofstream out_rand("rand");  
    ofstream out_size("size");  
    cout << "generate rand number" << endl;  
    int k_count = 0;  
    for (int i = 0; i < iter; i++)  
    {  
        instruction[i] = rand() % 2;  
        sizes[i] = (rand() % 16 + 1) * 64;  
    }  
    int test_count = 200;  
    double t1, t2;  
    double* ratio = new double[test_count];  
    int count = 0;  
    for (int k = 0; k < test_count; k++)  
    {  
        if (break_time != 0)  
        {  
            cout << "break @ " << k << " / " << test_count << endl;  
            break;  
        }  
        count++;  
        cout << "******************************************test " << k+1 << " *************************************************" << endl;  
        t1 = test_std_perf(iter, sizes, instruction);  
        t2 = test_mem_pool_perf_rand(mem_pool, iter,  sizes, instruction);  
        cout << "total memory: " << mem_pool->size << ", memory used: " << mem_pool->mem_used_size   
            << ", memory left: " << mem_pool->size - mem_pool->mem_used_size  << endl;  
        ratio[k] = t1 / t2;  
    }  
    if(break_time == 0)  
        break_time = test_count;  
    break_time = count - 1;  
    cout << "*************************** ratio (system time / mem_pool time) ***************************" << endl;  
    for (int k = 0; k < break_time; k++)  
    {  
        out << ratio[k] << ",";  
        if (k % 10 == 0 && k != 0)  
        {  
            cout << endl;  
        }  
        cout << ratio[k] << " ";  
    }  
    cout << endl;  
    delete []ratio;  
    free(pBuf);  
    delete []instruction;  
    delete []sizes;  
}  
// 申请紧接着释放  
void rand_test_2()  
{  
    size_t sBufSize = 500* 1024*1024;  
    void*pBuf = malloc(sBufSize);  
    if (pBuf == NULL)  
    {  
        cout << "malloc failed" << endl;  
        return;  
    }  
    PMEMORYPOOL mem_pool = CreateMemoryPool(pBuf, sBufSize);  
    int iter = 1000;  
    int test_count = 1024;  
    int* sizes = new int[iter];   
    double* ratio = new double[test_count];  
    double* std_perf = new double[test_count];  
    double* mem_pool_perf = new double[test_count];  
    if (sizes == NULL || ratio == NULL)  
    {  
        cout << "new memory failed" << endl;  
        return;  
    }  
    srand(time(NULL));   
    cout << "generate rand number" << endl;  
    ofstream out("rand_test_2.txt");  
    ofstream out_std("std_perf_2.txt");  
    ofstream out_mem_pool("mem_pool_perf_2.txt");  
    for (int k = 0; k < test_count; k++)  
    {  
        for (int i = 0; i < iter; i++)  
        {  
            sizes[i] = (rand() % 16 + 1) * 64 + 1024 * k;  
        }  
        cout << "******************************************test " << k+1 << " *************************************************" << endl;  
        double mem_pool_t = test_mem_pool_perf_1(mem_pool, iter, sizes);  
        double std_t = test_std_perf_1(iter, sizes);  
        std_perf[k] = std_t;  
        mem_pool_perf[k] = mem_pool_t;  
        ratio[k] = std_t / mem_pool_t;  
    }  
    cout << "*************************** ratio (system time / mem_pool time) ***************************" << endl;  
    for (int k = 0; k < test_count; k++)  
    {  
        out_std << std_perf[k] << ",";  
        out_mem_pool << mem_pool_perf[k] << ",";  
        out << ratio[k] << ",";  
        if (k % 10 == 0 && k != 0)  
        {  
            cout << endl;  
        }  
        cout << ratio[k] << " ";  
    }  
    cout << endl;  
    delete []sizes;  
    delete []ratio;  
    delete []mem_pool_perf;  
    delete []std_perf;  
    free(pBuf);  
}  
int _tmain(int argc, _TCHAR* argv[])  
{  
//  rand_test();  
    rand_test_2();  
      
    return 0;  
} 
