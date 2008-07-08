#ifndef SOAR_PLAYER_CLIENT_INL
#define SOAR_PLAYER_CLIENT_INL

#include "SoarPlayerClient.h"

bool SoarPlayerClient::update_and_check_running()
{
    if ( m_run_thread )
    {
        if ( m_run_thread->IsStopped() == false )
        {
            return true;
        }
        else
        {
            m_run_thread = 0;
        }
    }
    return false;
}

std::string SoarPlayerClient::command_run()
{
    if ( update_and_check_running() )
    {
        return std::string( "already running" );
    }
    
    m_stop_issued = false;
    m_run_thread = new RunThread( m_kernel );
    m_run_thread->Start();
    
    return std::string();
}

std::string SoarPlayerClient::command_stop()
{
    if ( !update_and_check_running() )
    {
        return std::string( "not running" );
    }
    
    m_stop_issued = true;
    
    return std::string();
}

std::string SoarPlayerClient::command_step()
{
    if ( update_and_check_running() )
    {
        return std::string( "already running" );
    }
    
    m_kernel->RunAllAgents( 1 );

    return std::string();
}

std::string SoarPlayerClient::command_debug()
{
    return std::string( "not implemented yet" );
}

std::string SoarPlayerClient::command_reset()
{
    if ( update_and_check_running() )
    {
        return std::string( "running, stop first" );
    }
    
    m_agent->InitSoar();

    return std::string();
}

void SoarPlayerClient::update()
{
    double turnrate, speed;

    // read from the proxies
    m_robot.Read();

    // print out sonars for fun
    std::cout << m_sp << std::endl;

    // do simple collision avoidance
    if((m_sp[0] + m_sp[1]) < (m_sp[6] + m_sp[7]))
        turnrate = PlayerCc::dtor(-20); // turn 20 degrees per second
    else
        turnrate = PlayerCc::dtor(20);

    if(m_sp[3] < 0.500)
        speed = 0;
    else
        speed = 0.100;

    // command the motors
    m_pp.SetSpeed(speed, turnrate);

    // BUGBUG
    // The debugger hangs everything unless this line is here... 
    // That should not happen.
    m_kernel->CheckForIncomingEvents();
    
    if ( m_stop_issued ) 
    {
        m_kernel->StopAllAgents();
    }
}

#endif // SOAR_PLAYER_CLIENT_INL
