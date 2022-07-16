#include "ns3/log.h"
#include "content-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ContentManager");

ContentManager::ContentManager ()
{
    m_size = 0;
    m_array = new PatrolContent[100];
    m_count = 0;
}

ContentManager::~ContentManager ()
{
    delete [] m_array;
}

void
ContentManager::Insert(PatrolContent content)
{
    if (m_size == 100 || Exist(content.cellNumber))
    {
        return;
    }
    
    m_array[m_size] = content;
    m_size++;
}

PatrolContent*
ContentManager::GetContentArray() const
{
    return m_array;
}

uint32_t
ContentManager::GetContentArraySize() const
{
    return m_size;
}

int 
ContentManager::GetCount()
{
    return m_count;
}

void
ContentManager::Merge(PatrolContent *anotherArray, uint32_t anotherArraySize)
// assume anotherArray has unique elements
{   
    for (uint32_t i=0; i<anotherArraySize; ++i)
    {
        if ( !Exist(anotherArray[i]) ) 
        {
            Insert(anotherArray[i]);
        }
    }
}

bool
ContentManager::Exist(PatrolContent content)
{
    for (uint32_t i=0; i<m_size; ++i)
    {
        if ( m_array[i] == content )
        {
            m_count++;
            return true;
        }
    }
    return false;
}

void
ContentManager::PrintContent()
{
    for(uint32_t i = 0; i < m_size; i++)
    {
        cout << "IP : " << m_array[i].producter << " and cell : " << m_array[i].cellNumber << endl;
    }
}

bool
ContentManager::Exist(uint32_t content)
{
    for (uint32_t i=0; i<m_size; ++i)
    {
        if ( m_array[i].cellNumber == content )
        {
            return true;
        }
    }
    return false;
}

Ipv4Address 
ContentManager::SearchProvider(uint32_t content)
{
    for (uint32_t i=0; i<m_size; i++)
    {
        if ( m_array[i].cellNumber == content )
        {
            //cout << "SearchProvider " << m_array[i].producter;
            return m_array[i].producter;
        }
    }

    return Ipv4Address("0.0.0.0");
}

}
