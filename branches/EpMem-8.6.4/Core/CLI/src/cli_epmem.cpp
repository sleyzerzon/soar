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

#include "gSKI_Agent.h"
#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"
#include "gsysparam.h"

void epmem_print_memory_by_id(agent *thisAgent, int id);


using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEpmem(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
    Options optionsData[] = {
        {'n', "on",         0},
        {'o', "off",        0},
        {'z', "pause",      0},
        {'p', "print",      1},
        {'m', "match",      1},
        {'d', "diff",       2},
        {'c', "cue",        2},
        {'l', "load",       0},
        {'s', "save",       0},
        {'a', "autosave",   0},
        {0, 0, 0}
    };

    long arg1 = -1;
    long arg2 = -1;
    int optCount = 0;
    std::string tmpStr;
    std::vector<std::string>::iterator iter;
    char convbuf[64];
    char buf[1024];
    std::string msg;
    
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
                    msg = "Epmem ID expected.  Can not be a negative number.";
                }
                else
                {
                    DoEpmem(pAgent, EPMEM_PRINT_MEM, (long)arg1, 0);
                }
                break;
            case 'm':
                strcpy(convbuf, m_OptionArgument.c_str());
                if ( (convbuf[0] == 's') || (convbuf[0] == 'S') )
                {
                    arg1 = atoi(((char *)convbuf) + 1);
                }
                else
                {
                    arg1 = atoi(convbuf);
                }
                
                if (arg1 < 0)
                {
                    msg = "Invalid state ID.";
                }
                
                DoEpmem(pAgent, EPMEM_PRINT_MATCH_DIAGNOSTIC, arg1, 0);
                break;
            case 'd':
				if (argv.size() != 4)
                {
                    sprintf(buf,
                            "Two epmem IDs expected.  I received %d.",
                            argv.size() - 2);
                    msg = buf;
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
                    msg = "Epmem ID expected.  Can not be a negative number.";
                    break;
                }
                
                DoEpmem(pAgent, EPMEM_COMPARE_MEMS, (long)arg1, (long)arg2);
                break;
            case 'c':
				if (argv.size() != 4)
                {
                    msg = "Wrong number of arguments. Expected a state followed by a memory id.";
                    break;
                }
                iter = argv.begin();
                tmpStr = *(++iter);
                tmpStr = *(++iter);
                strcpy(convbuf, tmpStr.c_str());
                if ( (convbuf[0] == 's') || (convbuf[0] == 'S') )
                {
                    arg1 = atoi(((char *)convbuf) + 1);
                }
                else
                {
                    arg1 = atoi(convbuf);
                }
                if (arg1 < 0)
                {
                    msg = "Invalid state ID.";
                    break;
                }
                tmpStr = *(++iter);
                arg2 = atoi(tmpStr.c_str());

                if (arg2 < 0)
                {
                    msg = "Epmem ID expected for second number.  Can not be a negative number.";
                    break;
                }
                
                DoEpmem(pAgent, EPMEM_CUE_COMPARE_MEM, arg1, arg2);
                break;
            case 'l':
                //Use empty string to specify the default filename
                convbuf[0] = '\0';
                if (argv.size() >= 3)
                {
                    //Retrieve filename
                    iter = argv.begin();
                    tmpStr = *(++iter);
                    tmpStr = *(++iter);
                    strcpy(convbuf, tmpStr.c_str());
                }//else                
                DoEpmem(pAgent, EPMEM_LOAD, (long)convbuf, 0);
                break;
            case 's':
                //Use empty string to specify the default filename
                convbuf[0] = '\0';
				if (argv.size() >= 3)
                {
                    //Retrieve filename
                    iter = argv.begin();
                    tmpStr = *(++iter);
                    tmpStr = *(++iter);
                    strcpy(convbuf, tmpStr.c_str());
                }//else                
                DoEpmem(pAgent, EPMEM_SAVE, (long)convbuf, 0);
                break;
            case 'a':
                //Set default arguments
                arg2 = 500;     // default
                convbuf[0] = '\0';  // default

                //Get user specified arguments
				if (argv.size() >= 3)
                {
                    //Retrieve autosave filename
                    iter = argv.begin();
                    tmpStr = *(++iter);
                    tmpStr = *(++iter);
                    strcpy(convbuf, tmpStr.c_str());

                    //Retrieve autosave frequency
                    arg2 = 500; // default
                    if (argv.size() >= 4)
                    {
                        tmpStr = *(++iter);
                        arg2 = atoi(tmpStr.c_str());
                    }
                    
                    if (arg2 < 0)
                    {
                        msg = "Invalid autosave frequency specified.  Resetting to 500.\n";
                        arg2 = 500;
                    }

                }//else

                //Generate result message
                if (arg2 == 0)
                {
                    msg = "Disabling episodic memory autosave feature.";
                }
                else
                {
                    if (strlen(convbuf) == 0)
                    {
                        strcpy(convbuf, "default file");
                    }
                    sprintf(buf, "Autosaving to %s every %d cycles.",
                            convbuf, arg2);
                    msg = buf;
                }//else
                    
                DoEpmem(pAgent, EPMEM_AUTOSAVE, (long)convbuf, arg2);
                
                break;
            default:
                return SetError(CLIError::kGetOptError);
        }//switch
        
        if ((m_RawOutput) && (msg.length() > 0))
        {
            m_Result << msg.c_str();
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, msg.c_str());
        }
        
    }//for

	if (optCount == 0)
    {
        DoEpmem(pAgent, EPMEM_REPORT_SETTINGS, 0, 0);
        msg = "\n\n---";
        msg += "\nAvailable Commands:\n";
        msg += "\n\tepmem                         Display the current epmem settings and active ";
        msg += "\n\t                              retrievals.";
        msg += "\n\tepmem -n <id>                 Turn the episodic memory system on.";
        msg += "\n\tepmem -o <id>                 Turn the episodic memory system off. ";
        msg += "\n\t                              (All existing episoidic memories will be lost!)";
        msg += "\n\tepmem -z <id>                 Suspend the episodic memory system.";
        msg += "\n\tepmem -p <id>                 Print the contents of the memory with the ";
        msg += "\n\t                              given id.";
        msg += "\n\tepmem -m <state>              Display a match diagnostic for the retrieval on ";
        msg += "\n\t                              the given state.";
        msg += "\n\tepmem -d <id1> <id2>          Compare two memories with the given ids.";
        msg += "\n\tepmem -c <state> <id>         Match the memory with the given id to the memory ";
        msg += "\n\t                              cue on the given state.";
        msg += "\n\tepmem -s <filename>           Save episodic memories to file.";
        msg += "\n\tepmem -l <filename>           Load episodic memories from file.";
        msg += "\n\tepmem -a [<filename> [freq]]  Autosave episodic memories to [filename] every ";
        msg += "\n\t                              [freq] cycles.  Set frequnecy to zero to disable.";
    }
    
    return true;
}

bool CommandLineInterface::DoEpmem(gSKI::Agent* pAgent,
                                   enum eEpmemOptions setting,
                                   long arg1,
                                   long arg2)
{

    long sp_val = 0;
    char *str;
    if (!RequireAgent(pAgent)) return false;
    std::string msg;
    char buf[1024];

	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
    
    switch( setting )
    {
        case EPMEM_ON:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SYSPARAM);
            if (sp_val)
            {
                msg = "The episodic memory system is already ACTIVE.";
            }                
            else
            {
                pKernelHack->EpmemEnable(pAgent);
                msg = "The episodic memory system is now ACTIVE.";
            }
            break;
          
        case EPMEM_OFF:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SYSPARAM);
            if (sp_val)
            {
                pKernelHack->EpmemDisable(pAgent);
                msg = "The episodic memory system has been DISABLED.";
            }
            else
            {
                msg = "The episodic memory system is already DISABLED.";
            }
            break;
            
        case EPMEM_PAUSE:
            sp_val = pKernelHack->GetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM);
            if (sp_val)
            {
                pKernelHack->SetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM, 0);
                msg = "The episodic memory system has been UNPAUSED.";
            }
            else
            {
                pKernelHack->SetSysparam(pAgent, EPMEM_SUSPENDED_SYSPARAM, 1);
                msg = "The episodic memory system has been PAUSED.";
            }
            break;
            
        case EPMEM_REPORT_SETTINGS:
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemPrintStatus(pAgent);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
        
        case EPMEM_PRINT_MEM:
            sprintf(buf, "Printing epmem #%d\n", arg1);
            msg = buf;
            
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
            sprintf(buf,
                    "Displaying the differences between memories %d and %d:\n",
                    arg1, arg2);
            msg = buf;
            
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemCompareMemories(pAgent, arg1, arg2);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;

        case EPMEM_CUE_COMPARE_MEM:
            sprintf(buf, "Comparing the cue on state s%d to memory #%d:\n",
                    arg1, arg2);
            msg = buf;
                
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemCompareCueToMemory(pAgent, arg1, arg2);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
            
        case EPMEM_LOAD:
            str = (char *)arg1;
            if ((str == NULL) || (strlen(str) == 0))
            {
                strcpy(str, "default file");
            }
            sprintf(buf, "Loaded episodic memories from %s\n");
            msg = buf;
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemLoadMemories(pAgent, str);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
          
        case EPMEM_SAVE:
            str = (char *)arg1;
            if ((str == NULL) || (strlen(str) == 0))
            {
                strcpy(str, "default file");
            }
            sprintf(buf, "Saved episodic memories to %s\n", str);
            msg = buf;
            
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemSaveMemories(pAgent, str);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
            
        case EPMEM_AUTOSAVE:
            str = (char *)arg1;
            this->AddListenerAndDisableCallbacks(pAgent);
            pKernelHack->EpmemAutoSaveMemories(pAgent, str, arg2);
            this->RemoveListenerAndEnableCallbacks(pAgent);
            break;
    }//switch

    if ((m_RawOutput) && (msg.length() > 0))
    {
        m_Result << msg.c_str();
    }
    else
    {
        AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, msg.c_str());
    }
        
        
    
    return true;
}

