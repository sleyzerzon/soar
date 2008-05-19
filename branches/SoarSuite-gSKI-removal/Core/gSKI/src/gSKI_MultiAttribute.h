/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#ifndef GSKI_MULTIATTRIBUTE_H
#define GSKI_MULTIATTRIBUTE_H

#include <string>

#include "IgSKI_MultiAttribute.h"
#include "gSKI_ReleaseImpl.h"

typedef struct agent_struct agent;

namespace gSKI
{

   class MultiAttribute : public RefCountedReleaseImpl<IMultiAttribute>
   {
   public:
      /** 
         Constructor

         @param pAgent Owning agent
         @param attribute Attribute name
         @param priority Matching priority
      */
      MultiAttribute(agent* pAgent, const char* attribute, int priority);

      virtual const char* GetAttributeName(Error *pErr = 0) const; 
      virtual int GetMatchingPriority(Error *pErr = 0) const;

   private:
      agent* m_pAgent;           /// Owning agent
      std::string m_attribute;   /// Name of attribute
      int m_priority;            /// Matching priority

   protected:
      /** Private destructor so no one tries to use delete directly
	      2/23/05: changed to protected to eliminate gcc warning */
      virtual ~MultiAttribute() {}
   }; // class MultiAttribute

} // namespace gSKI

#endif // GSKI_MULTIATTRIBUTE_H
