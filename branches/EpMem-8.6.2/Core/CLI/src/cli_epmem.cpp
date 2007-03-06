/////////////////////////////////////////////////////////////////
// matches command file.
//
// Author: Andrew Nuxoll
// Date  : Novemeber 2006
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <assert.h>

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

void epmem_print_memory_by_id(agent *thisAgent, int id);


using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEpmem(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
    Options optionsData[] = {
        {'n', "on",         0},
        {'o', "off",        0},
        {'z', "pause",      0},
        {'p', "print",      1},
        {'m', "match",      1},
        {'d', "diff",       2},
        {'c', "cue",        2},
        {0, 0, 0}
    };

    long arg1 = -1;
    long arg2 = -1;
    int optCount = 0;
    std::string tmpStr;
    std::vector<std::string>::iterator iter;
    char buf[64];
    
    for (;;) {
        if (!ProcessOptions(argv, optionsData)) return false;
        if (m_Option == -1) break;

        optCount++;
        switch (m_Option) {
            case 'n':
                DoEpmem(pAgent, EPMEM_ON, 0, 0);
                break;
            case 'o':
                DoEpmem(pAgent, EPMEM_OFF, 0, 0);
                break;
            case 'z':
                DoEpmem(pAgent, EPMEM_PAUSE, 0, 0);
                break;
            case 'p':
                if (!IsInteger(m_OptionArgument))
                {
                    SetError(CLIError::kIntegerExpected);
                    return false;
                }
                arg1 = atoi(m_OptionArgument.c_str());
                if (arg1 < 0)
                {
                    m_Result << "Epmem ID expected.  Can not be a negative number.";
                }
                else
                {
                    DoEpmem(pAgent, EPMEM_PRINT_MEM, (long)arg1, 0);
                }
                break;
            case 'm':
                strcpy(buf, m_OptionArgument.c_str());
                if ( (buf[0] == 's') || (buf[0] == 'S') )
                {
                    arg1 = atoi(((char *)buf) + 1);
                }
                else
                {
                    arg1 = atoi(buf);
                }
                
                if (arg1 < 0)
                {
                    m_Result << "Invalid state ID.";
                }
                
                DoEpmem(pAgent, EPMEM_PRINT_MATCH_DIAGNOSTIC, arg1, 0);
                break;
            case 'd':
				if (argv.size() != 4)
                {
                    m_Result << "Two epmem IDs expected.  I received ";
                    m_Result << argv.size() - 2 << ".";
                    break;
                }
                iter = argv.begin();
                tmpStr = *(++iter);
                tmpStr = *(++iter);
                arg1 = atoi(tmpStr.c_str());
                tmpStr = *(++iter);
                arg2 = atoi(tmpStr.c_str());

                if ((arg1 < 0) || (arg2 < 0))
                {
                    m_Result << "Epmem ID expected.  Can not be a negative number.";
                    break;
                }
                
                DoEpmem(pAgent, EPMEM_COMPARE_MEMS, (long)arg1, (long)arg2);
                break;
            case 'c':
				if (argv.size() != 4)
                {
                    m_Result << "Wrong number of arguments. Expected a state followed by a memory id.";
                    break;
                }
                iter = argv.begin();
                tmpStr = *(++iter);
                tmpStr = *(++iter);
                strcpy(buf, tmpStr.c_str());
                if ( (buf[0] == 's') || (buf[0] == 'S') )
                {
                    arg1 = atoi(((char *)buf) + 1);
                }
                else
                {
                    arg1 = atoi(buf);
                }
                if (arg1 < 0)
                {
                    m_Result << "Invalid state ID.";
                    break;
                }
                tmpStr = *(++iter);
                arg2 = atoi(tmpStr.c_str());

                if (arg2 < 0)
                {
                    m_Result << "Epmem ID expected for second number.  Can not be a negative number.";
                    break;
                }
                
                DoEpmem(pAgent, EPMEM_CUE_COMPARE_MEM, arg1, arg2);
                break;
            default:
                return SetError(CLIError::kGetOptError);
        }
    }

	if (optCount == 0)
    {
        DoEpmem(pAgent, EPMEM_REPORT_SETTINGS, 0, 0);
        m_Result << "\n\n---";
        m_Result << "\nepmem command syntax: epmem [-cdmp [arg1] [arg2]]\n";
        m_Result << "\nExample Commands:\n";
        m_Result << "\n\tepmem                    Display the current epmem settings and active retrievals.";
        m_Result << "\n\tepmem -n <id>            Turn the episodic memory system on.";
        m_Result << "\n\tepmem -o <id>            Turn the episodic memory system off. (All existing episoidic memories will be lost!)";
        m_Result << "\n\tepmem -z <id>            Suspend the episodic memory system.";
        m_Result << "\n\tepmem -p <id>            Print the contents of the memory with the given id.";
        m_Result << "\n\tepmem -m <state>         Display a match diagnostic for the retrieval on the given state.";
        m_Result << "\n\tepmem -d <id1> <id2>     Compare two memories with the given ids.";
        m_Result << "\n\tepmem -c <state> <id>    Match the memory with the given id to the memory cue on the given state.";
    }
    
    return true;
}

bool CommandLineInterface::DoEpmem(gSKI::IAgent* pAgent,
                                   enum eEpmemOptions setting,
                                   long arg1,
                                   long arg2)
{

    long sp_val = 0;
    if (!RequireAgent(pAgent)) return false;

	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
    
    switch( setting )
    {
        case EPMEM_ON:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SYSPARAM);
            if (sp_val)
            {
                m_Result << "The episodic memory system is already ACTIVE.";
            }                
            else
            {
                pKernelHack->EpmemEnable(pAgent);
                m_Result << "The episodic memory system is now ACTIVE.";
            }
            break;
          
        case EPMEM_OFF:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SYSPARAM);
            if (sp_val)
            {
                pKernelHack->EpmemDisable(pAgent);
                m_Result << "The episodic memory system has been DISABLED.";
            }
            else
            {
                m_Result << "The episodic memory system is already DISABLED.";
            }
            break;
            
        case EPMEM_PAUSE:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM);
            if (sp_val)
            {
                pKernelHack->SetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM, 0);
                m_Result << "The episodic memory system has been UNPAUSED.";
            }
            else
            {
                pKernelHack->SetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM, 1);
                m_Result << "The episodic memory system has been PAUSED.";
            }
            break;
            
        case EPMEM_REPORT_SETTINGS:
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemPrintStatus(pAgent);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
        
        case EPMEM_PRINT_MEM:
            m_Result << "Printing epmem #" << arg1 << "\n";
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemPrintMemory(pAgent, arg1);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
            
        case EPMEM_PRINT_MATCH_DIAGNOSTIC:
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemPrintMatchDiagnostic(pAgent, arg1);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
            
        case EPMEM_COMPARE_MEMS:
            m_Result << "Displaying the differences between memories ";
            m_Result << arg1 << " and " << arg2 << ":\n";
                
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemCompareMemories(pAgent, arg1, arg2);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;

        case EPMEM_CUE_COMPARE_MEM:
            m_Result << "Comparing the cue on state s" << arg1;
            m_Result << " to memory #" << arg2 << ":\n";
                
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemCompareCueToMemory(pAgent, arg1, arg2);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
            
    }//switch

    return true;
}


