#ifndef _SIGNATURE_HPP_
#define _SIGNATURE_HPP_

#include <string>

class Signature {
protected:
    std::string str;
    /**
     * The field "active_tasks" is only used when the 
     * input automata models the scheduling of a set of 
     * real-time tasks.
     */
    unsigned active_tasks;
public:
    Signature () {}
    Signature (const std::string &s);
    const std::string& get_str() const;
    bool operator == (const Signature &sig) const;
    bool operator < (const Signature &sig) const;
    
    // Only used for real-time scheduling model 
    const unsigned& get_active_tasks() const;
    bool includes(const Signature &sig) const;
    
};

#endif
