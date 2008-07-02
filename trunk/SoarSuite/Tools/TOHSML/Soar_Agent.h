#ifndef SOAR_AGENT_H
#define SOAR_AGENT_H

#include "sml_Client.h"
#include <string>

/** This class can be used transparently as a sml::Agent object,
  * with the added benefit that it will self destruct when it 
  * goes out of scope.
  */
class Soar_Agent {
  Soar_Agent(const Soar_Agent &);
  Soar_Agent & operator=(const Soar_Agent &);

public:
  inline Soar_Agent(sml::Kernel &kernel,
                   const std::string &name = "TOH",
                   const std::string &productions = "../../Environments/JavaTOH/towers-of-hanoi-SML.soar");
  inline ~Soar_Agent();
  
  const sml::Agent & operator*() const          {return *m_agent_ptr;}
  sml::Agent & operator*()                      {return *m_agent_ptr;}
  const sml::Agent * const & operator->() const {return  m_agent_ptr;}
  sml::Agent * operator->()                     {return  m_agent_ptr;}

  operator const sml::Agent & () const          {return *m_agent_ptr;}
  operator sml::Agent & ()                      {return *m_agent_ptr;}
  operator const sml::Agent * const & () const  {return  m_agent_ptr;}
  operator sml::Agent * const & ()              {return  m_agent_ptr;}

  const sml::Kernel * const & get_kernel() const {return m_kernel_ptr;}
  sml::Kernel * const & get_kernel()             {return m_kernel_ptr;}
  
  inline void LoadProductions(const std::string &productions);

private:
  sml::Agent * const m_agent_ptr;

  sml::Kernel * const m_kernel_ptr;
};

/// For inlines
#include "TOH_Game.h"

Soar_Agent::Soar_Agent(sml::Kernel &kernel, const std::string &name, const std::string &productions)
  // Create an arbitrarily named Soar agent
  : m_agent_ptr(kernel.CreateAgent(name.c_str())),
  m_kernel_ptr(&kernel)
{
  // Check that nothing went wrong
  // NOTE: No agent gets created if there�s a problem, so we have to check for
  // errors through the kernel object.
  if(!m_agent_ptr || kernel.HadError()) {
    std::cerr << kernel.GetLastErrorDescription() << std::endl;
    abort();
  }

  LoadProductions(productions);
}

Soar_Agent::~Soar_Agent() {
  m_kernel_ptr->DestroyAgent(m_agent_ptr);
}

void Soar_Agent::LoadProductions(const std::string &productions) {
  // Load the TOH productions
  if(!m_agent_ptr->LoadProductions(productions.c_str()) ||
    m_agent_ptr->HadError()) {
    std::cerr << m_agent_ptr->GetLastErrorDescription() << std::endl;
    abort();
  }
}

#endif
