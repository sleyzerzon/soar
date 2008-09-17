//virtual void CommandOutput::OnKernelEvent( int eventID, AgentSML* pAgentSML, void* pCallData )
//{
//	// Registered for this event in source command
//	if ( eventID == smlEVENT_BEFORE_PRODUCTION_REMOVED )
//	{
//		// Only called when source command is active
//		++m_NumProductionsExcised;
//
//		if (m_SourceVerbose) {
//			production* p = (production*) pCallData ;
//			assert(p) ;
//			assert(p->name->sc.name) ;
//
//			std::string name( p->name->sc.name );
//
//			m_ExcisedDuringSource.push_back( name );
//		}
//	}
