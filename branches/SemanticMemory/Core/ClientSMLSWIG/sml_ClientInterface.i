/* File : sml_ClientInterface.i */

//
// SWIG support for std::string
//
%include std_string.i

//
// These functions need to be renamed because they only differ by a const type, which isn't enough to distinguish them
//
%rename(SetTagNameConst) sml::ElementXML::SetTagName(char const* tagName);
%rename(AddAttributeConst) sml::ElementXML::AddAttribute(char const* attributeName, char* attributeValue);
%rename(AddAttributeConstConst) sml::ElementXML::AddAttribute(char const* attributeName, char const* attributeValue);
%rename(SetCharacterDataConst) sml::ElementXML::SetCharacterData(char const* characterData);
%rename(SetBinaryCharacterDataConst) sml::ElementXML::SetBinaryCharacterData(char const* characterData, int length);

//
// These static functions create a new Kernel object that should be destroyed later
//
%newobject sml::Kernel::CreateKernelInCurrentThread;
%newobject sml::Kernel::CreateKernelInNewThread;
%newobject sml::Kernel::CreateRemoteConnection;

//
// These static functions generate a new char* object that should be destroyed later
// We also need to tell SWIG how to delete the object (hence the typemaps)
//
%newobject sml::ClientXML::GenerateXMLString(bool) const ;
%newobject sml::ElementXML::GenerateXMLString(bool) const ;
%newobject sml::AnalyzeXML::GenerateXMLString(bool) const ;
%newobject sml::ClientAnalyzedXML::GenerateXMLString(bool) const ;

%typemap(newfree) char* sml::ClientXML::GenerateXMLString {
    sml::ClientXML::DeleteString($1);
}
%typemap(newfree) char* GenerateXMLString {
    sml::ElementXML::DeleteString($1);
}
%typemap(newfree) char* sml::AnalyzeXML::GenerateXMLString {
    sml::AnalyzeXML::DeleteString($1);
}
%typemap(newfree) char* sml::ClientAnalyzedXML::GenerateXMLString {
    sml::ClientAnalyzedXML::DeleteString($1);
}

//
// This parsing method returns a new ElementXML object that should be destroyed later
//
%newobject sml::ElementXML::ParseXMLFromString;

//
// Don't wrap the code for registering callbacks because we need to provide some custom code to make it work
//
%ignore sml::Agent::RegisterForProductionEvent(smlProductionEventId, ProductionEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForRunEvent(smlRunEventId, RunEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForPrintEvent(smlPrintEventId, PrintEventHandler, void*, bool ignoreOwnEchos = true,  bool addToBack = true);
%ignore sml::Agent::RegisterForXMLEvent(smlXMLEventId, XMLEventHandler, void*, bool addToBack = true);
%ignore sml::Agent::RegisterForOutputNotification(OutputNotificationHandler, void*, bool addToBack = true);
%ignore sml::Agent::AddOutputHandler(char const*, OutputEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForSystemEvent(smlSystemEventId, SystemEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForAgentEvent(smlAgentEventId, AgentEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForUpdateEvent(smlUpdateEventId, UpdateEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForStringEvent(smlStringEventId, StringEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::AddRhsFunction(char const*, RhsEventHandler, void*, bool addToBack = true);
%ignore sml::Kernel::RegisterForClientMessageEvent(char const*, RhsEventHandler, void*, bool addToBack = true);

//
// Tell SWIG to include these files in the generated wrapper code
//
%{
#include "sml_Names.h"
#include "sml_ElementXML.h"
#include "sml_AnalyzeXML.h"
#include "sml_ClientErrors.h"
#include "sml_ClientEvents.h"
#include "sml_ClientWMElement.h"
#include "sml_ClientIntElement.h"
#include "sml_ClientFloatElement.h"
#include "sml_ClientStringElement.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientKernel.h"
#include "sml_ClientAgent.h"
#include "sml_ClientXML.h"
#include "sml_ClientTraceXML.h"
#include "sml_ClientAnalyzedXML.h"
%}

//
// Tell SWIG to wrap these files
//
%include "sml_Names.h"
%include "sml_ElementXML.h"
%include "sml_AnalyzeXML.h"
%include "sml_ClientErrors.h"
%include "sml_ClientEvents.h"
%include "sml_ClientWMElement.h"
%include "sml_ClientIntElement.h"
%include "sml_ClientFloatElement.h"
%include "sml_ClientStringElement.h"
%include "sml_ClientIdentifier.h"
%include "sml_ClientKernel.h"
%include "sml_ClientAgent.h"
%include "sml_ClientXML.h"
%include "sml_ClientTraceXML.h"
%include "sml_ClientAnalyzedXML.h"