//===-- Queue.cpp -----------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/Process.h"
#include "lldb/Target/Queue.h"
#include "lldb/Target/QueueList.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/SystemRuntime.h"

using namespace lldb;
using namespace lldb_private;

Queue::Queue (ProcessSP process_sp, lldb::queue_id_t queue_id, const char *queue_name) :
    m_process_wp (),
    m_queue_id (queue_id),
    m_queue_name (),
    m_running_work_items_count(0),
    m_pending_work_items_count(0),
    m_pending_items(),
    m_dispatch_queue_t_addr(LLDB_INVALID_ADDRESS)
{
    if (queue_name)
        m_queue_name = queue_name;

    m_process_wp = process_sp;
}

Queue::~Queue ()
{
}

queue_id_t
Queue::GetID ()
{
    return m_queue_id;
}

const char *
Queue::GetName ()
{
    const char *result = NULL;
    if (m_queue_name.size() > 0)
        result = m_queue_name.c_str();
    return result;
}

uint32_t
Queue::GetIndexID ()
{
    return m_queue_id;
}

std::vector<lldb::ThreadSP>
Queue::GetThreads ()
{
    std::vector<ThreadSP> result;
    ProcessSP process_sp = m_process_wp.lock();
    if (process_sp.get ())
    {
        for (ThreadSP thread_sp : process_sp->Threads())
        {
            if (thread_sp->GetQueueID() == m_queue_id)
            {
                result.push_back (thread_sp);
            }
        }
    }
    return result;
}

void
Queue::SetNumRunningWorkItems (uint32_t count)
{
    m_running_work_items_count = count;
}

uint32_t
Queue::GetNumRunningWorkItems () const
{
    return m_running_work_items_count;
}


void
Queue::SetNumPendingWorkItems (uint32_t count)
{
    m_pending_work_items_count = count;
}

uint32_t
Queue::GetNumPendingWorkItems () const
{
    return m_pending_work_items_count;
}

void
Queue::SetLibdispatchQueueAddress (addr_t dispatch_queue_t_addr)
{
    m_dispatch_queue_t_addr = dispatch_queue_t_addr;
}

addr_t
Queue::GetLibdispatchQueueAddress () const
{
    return m_dispatch_queue_t_addr;
}


const std::vector<lldb::QueueItemSP> &
Queue::GetPendingItems ()
{
    if (m_pending_items.size() == 0)
    {
        ProcessSP process_sp = m_process_wp.lock();
        if (process_sp && process_sp->GetSystemRuntime())
        {
            process_sp->GetSystemRuntime()->PopulatePendingItemsForQueue (this);
        }
    }
    return m_pending_items;
}
