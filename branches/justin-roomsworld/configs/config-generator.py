#!/usr/bin/env python3

from argparse import ArgumentParser
from itertools import product
from os import environ as env, makedirs as mkdir
from os.path import exists
from random import choice, randint, randrange, seed, uniform
from shutil import rmtree
import re
import sys

heights = ["short", "medium", "tall"]
colors = ["red", "orange", "yellow", "green", "blue", "indigo", "violet"]
shapes = ["sphere", "tetrahedron", "cube", "octahedron", "dodecahedron", "icosahedron", "tesseract"]
rooms = [10 * n for n in list(range(6))]
rooms.append(35)
min_x = 1
max_x = 30
min_y = 1
max_y = 40

def generate_map(randseed, num_blocks, num_tasks):
	seed(randseed)
	templates = set()
	blocks = []
	tasks = []
	for i in range(num_blocks):
		height = choice(heights)
		color = choice(colors)
		shape = choice(shapes)
		templates.add((height, color, shape))
		blocks.append((height, color, shape, uniform(min_x, max_x), uniform(min_y, max_y)))
	for i in range(num_tasks):
		tasks.append((randrange(num_blocks), choice(rooms)))
	# generate object templates
	config = []
	for template in templates:
		config.append(re.sub("\n\t*", "\n", """
				metadata.objects.{height}-{color}-{shape}.size = [ .25, .25];
				metadata.objects.{height}-{color}-{shape}.height = "{height}";
				metadata.objects.{height}-{color}-{shape}.color = "{color}";
				metadata.objects.{height}-{color}-{shape}.shape = "{shape}";
				metadata.objects.{height}-{color}-{shape}.type = "block";
			""".format(height=template[0], color=template[1], shape=template[2])).strip())
	# generate placed objects
	config.append("metadata.objects.placed = [")
	for block in blocks:
		config.append("\t{height}-{color}-{shape}, {x}, -{y},".format(height=block[0], color=block[1], shape=block[2], x=block[3], y=block[4]))
	config.append("];")
	config = "\n".join(config)
	# generate smem 
	smem = []
	smem.append("smem --add {")
	for index, task in enumerate(tasks):
		smem.append(re.sub("\n\t*", "\n", """
			(<task{index}> ^type task
			               ^object <obj{index}>
			               ^destination <dest{index}>
			               ^id {index})
			(<obj{index}> ^height {height}
			              ^color {color}
			              ^shape {shape})
			(<dest{index}> ^id {room})
			""".format(index=index, height=blocks[task[0]][0], color=blocks[task[0]][1], shape=blocks[task[0]][2], room=task[1])).strip())
		smem.append("")
	smem.append("}")
	smem = "\n".join(smem)
	return (config, smem)

def main(experiment):
	params = {}
	params["experiment-name"] = (experiment,)
	params["trial-num"] = range(0, 1)
	params["max-patrol-circuits"] = (3,)
	params["rand-seed"] = (0.5690544779, 0.7655848284, 0.7369625796, 0.8272997919, 0.7505874376, 0.2878377921, 0.1086963577, 0.2739799798, 0.5553159005, 0.8903765146)

	params["num-blocks"] = (11,)
	params["num-tasks"] = (11,)

	if experiment == "rehearsal-entry":
		params["method-ecological-entry"] = ("true",)
		params["method-ecological-rehearsal"] = ("true",)
		params["method-ecological-rehearsal-amount"] = range(10, 61, 10)
		params["decay-rate"] = [float(n) / 100 for n in range(35, 46, 2)]
		params["linear-velocity"] = [float(n) / 10 for n in range(3, 12, 2)]
		params["filename-template"] = (r"{experiment-name}_{decay-rate:.2f}_{linear-velocity}_{method-ecological-rehearsal-amount}_{rand-seed}",)
	elif experiment == "rehearsal-timing":
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-timing-interval"] = (400,)
		params["method-ecological-rehearsal"] = ("true",)
		params["method-ecological-rehearsal-amount"] = range(10, 61, 10)
		params["decay-rate"] = [float(n) / 100 for n in range(35, 46, 2)]
		params["linear-velocity"] = [float(n) / 10 for n in range(3, 12, 2)]
		params["filename-template"] = (r"{experiment-name}_{decay-rate:.2f}_{linear-velocity}_{method-ecological-rehearsal-amount}_{rand-seed}",)

	elif experiment == "retrieval-entry":
		params["method-ecological-entry"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["decay-rate"] = [float(n) / 100 for n in range(65, 76, 2)]
		params["linear-velocity"] = [float(n) / 10 for n in range(3, 12, 2)]
		params["filename-template"] = (r"{experiment-name}_{decay-rate:.2f}_{linear-velocity}_{rand-seed}",)
	elif experiment == "retrieval-timing":
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["method-ecological-timing-interval"] = (400,)
		params["decay-rate"] = [float(n) / 100 for n in range(65, 76, 2)]
		params["linear-velocity"] = [float(n) / 10 for n in range(3, 12, 2)]
		params["filename-template"] = (r"{experiment-name}_{decay-rate:.2f}_{linear-velocity}_{rand-seed}",)

	else:
		print("unknown experiment name: {}".format(experiment))
		exit(1)

	if "filename-template" not in params:
		print("experiment does not have a filename-template")
		exit(1)

	expdir = "experiments/{}".format(experiment)
	if exists(expdir):
		rmtree(expdir)
	mkdir(expdir)

	template = open("template.txt", "r").read()
	run_files = []
	keys = sorted(params.keys())
	cache = {}
	for values in product(*(params[k] for k in keys)):
		raw_params = dict(zip(keys, values))
		param_map = dict(zip([(k if "." in k else "seek.properties.{}".format(k)) for k in keys], values))

		filename = raw_params["filename-template"].format(**raw_params)
		for attribute in ("experiment-name", "filename-template"):
			param_map["seek.properties.{}".format(attribute)] = '"{}"'.format(raw_params[attribute])
		param_map["seek.properties.log-file"] = '"../../exp_results/{}.txt"'.format(filename)

		task_key = (raw_params["rand-seed"], raw_params["num-blocks"], raw_params["num-tasks"])
		if task_key not in cache:
			objects, smem = generate_map(task_key[0], task_key[1], task_key[2])
			cache[task_key] = (objects, smem)
		else:
			objects, smem = cache[task_key]
		param_map["seek.properties.tasks-file"] = '"../../tasks/{}.soar"'.format("_".join(str(k) for k in task_key))

		run_file = "{}/{}.run".format(expdir, filename)
		with open(run_file, "w") as fd:
			for k, v in sorted(param_map.items()):
				fd.write("{} = {};\n".format(k, v))
			fd.write(objects)
			fd.write("\n")
			fd.write(template)
		run_files.append(run_file)
	for key, value in cache.items():
		task_file = "../tasks/{}.soar".format("_".join(str(k) for k in key))
		if not exists(task_file):
			with open(task_file, "w") as fd:
				fd.write(value[1])
	with open("{}/multiple.runs".format(expdir), "w") as fd:
		fd.write("multiple-runs {\n")
		fd.write("\tconfigs = [\n")
		fd.write("\n".join('\t\t"../../configs/{}",'.format(run_file) for run_file in sorted(run_files)) + "\n")
		fd.write("\t];\n")
		fd.write("\tcycles = -1;\n")
		fd.write("\tseconds = -1;\n")
		fd.write("}\n")

if __name__ == "__main__":
	arg_parser = ArgumentParser(usage="%(prog)s <experiment>", description="experiment configuration generator for RoomsWorld")
	arg_parser.add_argument("experiments",  metavar="EXPERIMENTS", nargs="*", help="the experiment to generate configurations for")
	args = arg_parser.parse_args()

	for exp in args.experiments:
		main(exp)
