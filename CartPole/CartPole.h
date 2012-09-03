/* CartPole.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * The actual Towers of Hanoi game object that runs the show.
 */

#ifndef CARTPOLE_H
#define CARTPOLE_H

#include "Soar_Kernel.h"
#include "Soar_Agent.h"
#include <vector>
#include <string>

class Stats_Tracker;

#define CARTPOLE_AGENT_PRODUCTIONS "test_agents/cartpole-random-SML.soar"

class CartPole {
  /// Disabled (No Implementation)
  CartPole(const CartPole &);
  CartPole & operator=(const CartPole &);

  friend inline void toh_update_event_handler(sml::smlUpdateEventId, void *user_data_ptr, sml::Kernel* kernel_ptr, sml::smlRunFlags);

public:
  /* CartPole will create the default 'Soar_Kernel()' if a kernel is not provided.
   * CartPole will take care of the deletion of the given kernel if one is provided.
   */
  inline CartPole(const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS,
                  const bool &remote = false,
                  sml::Kernel * const kernel = 0);
  inline ~CartPole();

  static inline void run_trials(const int &num_trials,
                                const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS);
  static inline void remote_trials(const int &num_trials,
                                   const std::string &ip_address,
                                   const int &port,
                                   const std::string &agent_productions = CARTPOLE_AGENT_PRODUCTIONS);

  inline void run();
  inline void step();

  bool is_finished() const;
  bool is_success() const;

  void set_sp(const int &episode, const float &x_div, const float &x_dot_div, const float &theta_div, const float &theta_dot_div);
  void do_sp(const int &episode);
    
  void reinit(const bool &init_soar, const int &after_episode);
  bool SpawnDebugger();
  void srand(const int &seed);
  void ExecuteCommandLine(const std::string &command);

private:
  void update();

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;
  bool m_terminal;

  sml::StringElement * m_state;
  sml::IntElement * m_step;
  sml::FloatElement * m_reward;
  sml::FloatElement * m_x;
  sml::FloatElement * m_x_dot;
  sml::FloatElement * m_theta;
  sml::FloatElement * m_theta_dot;

  /// Observed
  float m_min_x;
  float m_min_x_dot;
  float m_min_theta;
  float m_min_theta_dot;
  float m_max_x;
  float m_max_x_dot;
  float m_max_theta;
  float m_max_theta_dot;
  
  int m_sp_episode;
  float m_sp_x_div;
  float m_sp_x_dot_div;
  float m_sp_theta_div;
  float m_sp_theta_dot_div;
};

#endif
