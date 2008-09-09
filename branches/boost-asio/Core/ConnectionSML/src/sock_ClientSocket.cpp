#include <portability.h>

/////////////////////////////////////////////////////////////////
// ClientSocket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Creates a socket by connecting to a server at a known
// IP address and port number.
// 
/////////////////////////////////////////////////////////////////
#include "sml_Utils.h"
#include "sock_ClientSocket.h"
#include "sock_OSspecific.h"

#include <sstream>
#include <assert.h>

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClientSocket::ClientSocket()
{

}

ClientSocket::~ClientSocket()
{

}

/////////////////////////////////////////////////////////////////////
// Function name  : ConvertAddress
// 
// Return type    : static in_addr* 	
// Argument       : char* pNetAddress	
// 
// Description	  : Converts from ip address or hostname to
//					socket address data.
//
//					NOTE: Uses static structures so only valid
//					until next call.  (This is gethostbyname's approach
//					so we have just extended it).
//
/////////////////////////////////////////////////////////////////////
static in_addr* ConvertAddress(char const* pNetAddress)
{
	static in_addr address ;

	assert(pNetAddress) ;

	// Try it as aaa.bbb.ccc.ddd first
	address.s_addr = inet_addr(pNetAddress) ;

	// Check if this method worked
	if (address.s_addr != INADDR_NONE)
		return &address ;

	// Then try it as a hostname
	hostent* pHost = gethostbyname(pNetAddress) ;

	// Check if this method worked
	if (pHost != NULL)
	{
		return (in_addr*) *pHost->h_addr_list ;
	}

	return NULL ;
}

//const char* kLocalHost = "127.0.0.1" ; // Special IP address meaning "this machine"
//
///////////////////////////////////////////////////////////////////////
//// Function name  : ClientSocket::ConnectToServer
//// 
//// Return type    : bool 	
//// Argument       : char* pNetAddress	// Can be NULL -- in which case connect to "this machine"
//// Argument       : int port	
//// 
//// Description	  : Connect to a server
////
///////////////////////////////////////////////////////////////////////
//bool ClientSocket::ConnectToServer(char const* pNetAddress, unsigned short port)
//{
//	using namespace boost::asio;
//
//	CTDEBUG_ENTER_METHOD("ClientSocket::ConnectToServer");
//
//	std::string address;
//	ip::address a;
//	if(pNetAddress) a = ip::address::from_string(pNetAddress);
//	else a = ip::address::from_string(kLocalHost);
//	//m_pSocket = new ip::tcp::socket(ioservice);
//	m_pSocket->connect( ip::tcp::endpoint(a, port) );
//
//	return true ;
//}
