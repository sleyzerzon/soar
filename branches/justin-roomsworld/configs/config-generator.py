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
	#params["rand-seed"] = (0.3607740397412259, 0.04379098940730264, 0.2554303853654126, 0.32718370539428654, 0.28781945867564607, 0.21508445060171022, 0.43672484330869266, 0.6871929206606627, 0.78931705115309, 0.7819027528531993)
	params["rand-seed"] = (0.3607740397412259, 0.04379098940730264,)
	params["num-blocks"] = (11,)
	params["num-tasks"] = (11,)

	if experiment == "ecological-task-limits":
		params["max-patrol-circuits"] = (0,)
		params["rand-seed"] = (0.07006882604508546, 0.22537786669043358, 0.25692781709096746, 0.6188586738375645, 0.0980532770823247, 0.41678387137142947, 0.3626522445712418, 0.42746224288858425, 0.7035482882075103, 0.0975384305874587, 0.09125975176581147, 0.23283024445841816, 0.03314801138516621, 0.879153504421756, 0.2940917155846817, 0.9801576212841999, 0.27854285620257746, 0.47307825494084377, 0.6350326544201157, 0.24095028815174035, 0.5226511061336242, 0.002732001296375519, 0.9293725910410491, 0.42862538136223727, 0.29305343002307405, 0.5410154533210964, 0.2524275931799884, 0.04649699806105134, 0.5638734797158196, 0.0021050287919592847, 0.6863811332091414, 0.7528050348554711, 0.19671461099430065, 0.8513909265861525, 0.16088577626839595, 0.7581054233389606, 0.5031286165782922, 0.616890860680216, 0.4729560934336069, 0.3522389482493895, 0.9145493209404224, 0.8853839035786348, 0.801964988509721, 0.763505771434606, 0.9969450254406105, 0.7170739681924918, 0.6277800750914091, 0.4311853354736467, 0.4161531617146135, 0.09186238423666604, 0.21070416259385483, 0.27896706388812886, 0.4698477448639269, 0.029694772307776507, 0.9257233556578373, 0.3492932851101018, 0.759203218114568, 0.6077441588232426, 0.26033741989294756, 0.22369424707123498, 0.7972801646551011, 0.9297271581090589, 0.5908470722623552, 0.5488501561484037, 0.4194963650031087, 0.4084509336508072, 0.4966338885887345, 0.9777593034281377, 0.11394984677983389, 0.10268029360468178, 0.7390309027453477, 0.5320765306636919, 0.10554319645690591, 0.3376006473373482, 0.002300487043233712, 0.9981584322236615, 0.8399677981675663, 0.2646055254797227, 0.10762275563180346, 0.8935510198677603, 0.7452228639901098, 0.12214101562598068, 0.5226206270726433, 0.12596835960367958, 0.5828627834714297, 0.8109872383114022, 0.7897486670785232, 0.012545051667930407, 0.8283591801906518, 0.8760719216593436, 0.8837332333952379, 0.09026078972816953, 0.45530159708069773, 0.13005247384993512, 0.03918840660779155, 0.9814324135130286, 0.10485346785013472, 0.785900949779522, 0.41378604336682456, 0.8737126492525892)
		params["num-blocks"] = (1,)
		params["num-tasks"] = range(10, 51, 10)
		params["decay-rate"] = [float(n) / 10 for n in range(7, 10)]
		params["filename-template"] = (r"{experiment-name}_{decay-rate}_{num-tasks:03}_{rand-seed}_{trial-num}",)

	elif experiment == "optimal-rehearsals-decay":
		params["linear-velocity"] = (0.7,)
		params["decay-rate"] = [float(n) / 100 for n in range(30, 51, 2)]
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-timing-interval"] = (400,)
		params["method-ecological-rehearsal"] = ("true",)
		params["method-ecological-rehearsal-amount"] = [pow(2, n) for n in range(2, 7)]
		params["filename-template"] = (r"{experiment-name}_{method-ecological-rehearsal-amount}_{decay-rate}_{rand-seed}_{trial-num}",)
	elif experiment == "optimal-rehearsals-velocity":
		params["linear-velocity"] = [float(n) / 10 for n in range(3, 12, 2)]
		params["decay-rate"] = (0.4,)
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-timing-interval"] = (400,)
		params["method-ecological-rehearsal"] = ("true",)
		params["method-ecological-rehearsal-amount"] = [pow(2, n) for n in range(1, 11, 2)]
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{decay-rate}_{linear-velocity}_{trial-num}",)

	elif experiment == "doors-retrieval":
		params["method-ecological-doors"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["decay-rate"] = [float(n) / 10 for n in range(5, 10)]
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{decay-rate}_{trial-num}",)
	elif experiment == "entry-retrieval":
		params["method-ecological-entry"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["decay-rate"] = [float(n) / 10 for n in range(5, 10)]
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{decay-rate}_{trial-num}",)
	elif experiment == "timing-retrieval":
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["method-ecological-timing-interval"] = [100 * pow(2, n) for n in range(2, 9, 2)]
		params["decay-rate"] = [float(n) / 10 for n in range(5, 10)]
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{decay-rate}_{method-ecological-timing-interval}_{trial-num}",)

	elif experiment == "doors-robustness":
		params["method-ecological-doors"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["decay-rate"] = (0.7,)
		params["linear-velocity"] = (0.3, 1.1)
		params["rand-seed"] = (0.2554303853654126, 0.32718370539428654, 0.28781945867564607, 0.21508445060171022, 0.43672484330869266)
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{linear-velocity}_{trial-num}",)
	elif experiment == "entry-robustness":
		params["method-ecological-entry"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["decay-rate"] = (0.7,)
		params["linear-velocity"] = (0.3, 1.1)
		params["rand-seed"] = (0.2554303853654126, 0.32718370539428654, 0.28781945867564607, 0.21508445060171022, 0.43672484330869266)
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{linear-velocity}_{trial-num}",)
	elif experiment == "timing-robustness":
		params["method-ecological-timing"] = ("true",)
		params["method-ecological-retrieval"] = ("true",)
		params["method-ecological-timing-interval"] = [100 * pow(2, n) for n in range(2, 9, 2)] # FIXME pick based on results
		params["decay-rate"] = (0.7,)
		params["linear-velocity"] = (0.3, 1.1)
		params["rand-seed"] = (0.2554303853654126, 0.32718370539428654, 0.28781945867564607, 0.21508445060171022, 0.43672484330869266)
		params["filename-template"] = (r"{experiment-name}_{rand-seed}_{linear-velocity}_{method-ecological-timing-interval}_{trial-num}",)

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
