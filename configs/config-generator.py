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

def generate_map(randseed, num_tasks, num_blocks):
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
	params["seek.properties.trial-num"] = range(0, 1)
	params["seek.properties.max-patrol-circuits"] = (3,)
	params["seek.properties.rand-seed"] = (8675309,)
	params["seek.properties.num-blocks"] = (11,)
	params["seek.properties.num-tasks"] = (11,)

	expdir = "experiments/{}".format(experiment)
	if exists(expdir):
		rmtree(expdir)
	mkdir(expdir)

	if experiment == "doors-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-doors"] = ["true",]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(5, 10)]
	elif experiment == "entry-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-entry"] = ["true",]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(5, 10)]
	elif experiment == "timing-method-only":
		params["seek.properties.experiment-name"] = [experiment,]
		params["seek.properties.method-ecological-timing"] = ["true",]
		params["seek.properties.method-ecological-timing-interval"] = [100 * pow(2, n) for n in range(2, 9, 2)]
		params["seek.properties.decay-rate"] = [float(n) / 10 for n in range(5, 10)]

	template = open("template.txt", "r").read()
	run_files = []
	keys = sorted(params.keys())
	cache = {}
	for values in product(*(params[key] for key in keys)):
		param_map = dict(zip(keys, values))
		decay_rate = param_map["seek.properties.decay-rate"]
		trial = param_map["seek.properties.trial-num"]
		interval = param_map.get("seek.properties.method-ecological-timing-interval", -1)
		param_map["seek.properties.experiment-name"] = '"{}"'.format(experiment)

		rand_seed = param_map["seek.properties.rand-seed"]
		num_blocks = param_map["seek.properties.num-blocks"]
		num_tasks = param_map["seek.properties.num-tasks"]
		key = (rand_seed, num_blocks, num_tasks)
		param_map["seek.properties.tasks-file"] = '"../../tasks/{}.soar"'.format("_".join(str(k) for k in key))
		if key not in cache:
			objects, smem = generate_map(rand_seed, num_tasks, num_blocks)
			cache[key] = (objects, smem)
		else:
			objects, smem = cache[key]

		filename = "{}_{}_{}_{}".format(experiment, decay_rate, interval, trial)
		param_map["seek.properties.log-file"] = '"../../exp_results/{}.txt"'.format(filename)
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
	arg_parser.add_argument("experiment",  metavar="EXPERIMENT", help="the experiment to generate configurations for")
	args = arg_parser.parse_args()

	main(args.experiment)
