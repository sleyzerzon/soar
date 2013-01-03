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
	params = {}
	params["seek.properties.trial-num"] = range(0, 2)

	if exists(experiment):
		rmtree(experiment)
	mkdir(experiment)

	if experiment == "doors-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-doors"] = ["true",]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(8, 9)]
	elif experiment == "entry-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-entry"] = ["true",]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(8, 9)]
	elif experiment == "timing-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-timing"] = ["true",]
		params["seek.properties.method-ecological-timing-interval"] = [100 * pow(2, n) for n in range(0, 10)]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(8, 9)]

	template = open("template.txt", "r").read()
	run_files = []
	for param_map in soar_exp.param_permutations(params):
		decay_rate = param_map["seek.properties.decay-rate"]
		trial = param_map["seek.properties.trial-num"]
		param_map["seek.properties.experiment-name"] = '"{}"'.format(experiment)
		param_map["seek.properties.log-file"] = '"../../exp_results/{}_{}_{}.txt"'.format(experiment, decay_rate, trial)
		run_file = "{}/{}_{}_{}.run".format(experiment, experiment, decay_rate, trial)
		with open(run_file, "w") as fd:
			for k, v in sorted(param_map.items()):
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
