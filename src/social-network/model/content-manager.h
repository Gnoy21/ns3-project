
#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include "ns3/ipv4-address.h"
#include "patrol-content.h"

//#define MAX_SIZE 1000 =>cause a compiler error

using namespace std;

namespace ns3
{

class ContentManager
{
public:    
    ContentManager ();
    virtual ~ContentManager ();
    
    void Insert(PatrolContent entry);
    PatrolContent *GetContentArray() const;
    uint32_t GetContentArraySize() const;
    int GetCount();

    Ipv4Address SearchProvider(uint32_t content);

    void PrintContent();
    
    //check if content exists in this table
    bool Exist(PatrolContent content);
    bool Exist(uint32_t content);
    
    //only merge element that does not yet exist in m_array
    void Merge(PatrolContent*anotherArray, uint32_t anotherArraySize); //merge another_array into m_array
    
private:
    PatrolContent* m_array; //contain only unique entry
    uint32_t m_size;
    int m_count;

};

}	// namespace ns3

#endif /* CONTENT_MANAGER_H */
