#ifndef ASSISTICON_HPP
#define ASSISTICON_HPP

#include <gui_generated/containers/AssistIconBase.hpp>

class AssistIcon : public AssistIconBase
{
public:
    AssistIcon();
    virtual ~AssistIcon() {}

    virtual void initialize();
    void setNumber(int no){
	    Unicode::snprintf(assistTextBuffer, ASSISTTEXT_SIZE, "%d", no);
    }
protected:
};

#endif // ASSISTICON_HPP
