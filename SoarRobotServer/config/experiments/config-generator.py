#!/usr/bin/env python

from argparse import ArgumentParser
from itertools import product
from os import environ as env, makedirs as mkdir
from os.path import exists
from shutil import rmtree
import re
import sys

import soar_exp

def main(experiment):
	global_params = {}

	if experiment == "doors-method-only":
		if exists(experiment):
			rmtree(experiment)
		mkdir(experiment)
		experiment_params = {}
		experiment_params["seek.properties.experiment-name"] = experiment
		experiment_params["seek.properties.method-ecological-doors"] = "true"
		sweep_params = {}
		sweep_params["seek.properties.trial-num"] = range(0, 2)
		sweep_params["seek.properties.decay-rate"] = map((lambda n: float(n) / 10), range(8, 9))
	create_config_files(global_params, experiment_params, sweep_params)

def create_config_files(global_params, experiment_params, sweep_params):
	template = open("template.txt", "r").read()
	experiment = experiment_params["seek.properties.experiment-name"]
	run_files = []
	for params in soar_exp.param_permutations(sweep_params):
		all_params = {}
		all_params.update(global_params)
		all_params.update(experiment_params)
		all_params.update(params)
		decay_rate = all_params["seek.properties.decay-rate"]
		trial = all_params["seek.properties.trial-num"]
		all_params["seek.properties.experiment-name"] = '"{}"'.format(experiment)
		all_params["seek.properties.log-file"] = '"../../exp_results/{}_{}_{}.txt"'.format(experiment, decay_rate, trial)
		run_file = "{}/{}_{}_{}.run".format(experiment, experiment, decay_rate, trial)
		with open(run_file, "w") as fd:
			for k, v in sorted(all_params.items()):
				fd.write("{} = {};\n".format(k, v))
			fd.write("\n")
			fd.write(template)
		run_files.append(run_file)
	with open("{}/multiple.runs".format(experiment), "w") as fd:
		fd.write("multiple-runs {\n")
		fd.write("\tconfigs = [\n")
		fd.write("\n".join('\t\t"config/experiments/{}",'.format(run_file) for run_file in sorted(run_files)) + "\n")
		fd.write("\t];\n")
		fd.write("\tcycles = -1;\n")
		fd.write("\tseconds = -1;\n")
		fd.write("}\n")

if __name__ == "__main__":
	arg_parser = ArgumentParser(usage="%(prog)s <experiment>", description="experiment configuration generator for RoomsWorld")
	arg_parser.add_argument("experiment",  metavar="EXPERIMENT", help="the experiment to generate configurations for")
	args = arg_parser.parse_args()

	main(args.experiment)
