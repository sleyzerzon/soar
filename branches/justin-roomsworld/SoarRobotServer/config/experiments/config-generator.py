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
	template = open("template.txt", "r").read()

	if experiment == "doors-method-only":
		if exists(experiment):
			rmtree(experiment)
		mkdir(experiment)
		run_files = []
		experiment_params = {}
		experiment_params["seek.properties.method-ecological-doors"] = "yes"
		sweep_params = {}
		sweep_params["seek.properties.trial"] = range(0, 2)
		sweep_params["seek.properties.decay-rate"] = map((lambda n: float(n) / 10), range(5, 9))
		for params in soar_exp.param_permutations(sweep_params):
			all_params = {}
			all_params.update(global_params)
			all_params.update(experiment_params)
			all_params.update(params)
			decay_rate = all_params["seek.properties.decay-rate"]
			trial = all_params["seek.properties.trial"]
			all_params["seek.properties.log-file"] = "{}_{}_{}.txt".format(experiment, decay_rate, trial)
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
			fd.write("\n".join('\t\t"{}",'.format(run_file) for run_file in run_files) + "\n")
			fd.write("\t];\n")
			fd.write("\tcycles = -1;\n")
			fd.write("\tseconds = -1;\n")
			fd.write("}\n")

if __name__ == "__main__":
	arg_parser = ArgumentParser(usage="%(prog)s <experiment>", description="experiment configuration generator for RoomsWorld")
	arg_parser.add_argument("experiment",  metavar="EXPERIMENT", help="the experiment to generate configurations for")
	args = arg_parser.parse_args()

	main(args.experiment)
