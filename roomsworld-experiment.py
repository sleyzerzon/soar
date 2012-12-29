#!/usr/bin/env python2.7

from os import environ as env
import re
import sys

import soar_exp

def create_configuration():


def main():
	params = {}
	params["home"] = (env["HOME"],)

	commands = (
			"source {home}/research/code/wsd/agent/{representation}.soar",
			)
	# run Soar
	kernel = soar_exp.create_kernel()
	for param_map in soar_exp.param_permutations(params):
		agent = soar_exp.create_agent(kernel, "agent-" + "-".join(str(val) for val in param_map.values()))
		wsd = WSD(kernel, agent, param_map)
		soar_exp.register_destruction_callback(kernel, agent, soar_exp.print_report_row, soar_exp.report_data_wrapper(param_map, wsd, reporters))
		#soar_exp.register_print_callback(kernel, agent, soar_exp.callback_print_message, None)
		soar_exp.run_parameterized_commands(agent, param_map, commands)
		agent.ExecuteCommandLine("run")
		#soar_exp.cli(agent)
		kernel.DestroyAgent(agent)
		del agent
	kernel.Shutdown()
	del kernel

if __name__ == "__main__":
	if len(sys.argv) > 1:
		from time import sleep
		sleep(int(sys.argv[1]))
	main()
