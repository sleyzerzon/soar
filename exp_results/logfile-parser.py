#!/usr/bin/env python

import re
import sys

if __name__ == "__main__":
	for f in sys.argv[1:]:
		with open(f, "r") as fd:
			output = {}
			echo, commands = fd.read().partition("END OUTPUT")[0].partition("END ECHOED")[0:3:2]

			# echo'ed output
			for line in echo.splitlines():
				if re.match("[^>]*:", line):
					output.update(("echoed {}".format(k), v) for k, v in [line.split(":", 1),])

			sstats, stimer, epmem, estats, etimer, wma, stats, statsmax, firing = commands.split("seek>")[1:-2]
			# FIXME would like to include topstate too

			# smem stats
			extract = ("Memory Usage", "Memory Highwater", "Retrieves", "Queries", "Stores", "Activation Updates", "Mirrors",)
			for line in sstats.splitlines():
				if ":" in line:
					output.update(("sstats {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# smem timer
			extract = ("_total", "smem_api", "smem_hash", "smem_init", "smem_ncb_retrieval", "smem_query", "smem_storage",)
			for line in stimer.splitlines():
				if ":" in line:
					output.update(("stimer {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# epmem settings
			extract = ("phase", "trigger", "force", "exclusions", "lazy-commit", "path", "balance", "graph-match", "graph-match-ordering",)
			for line in estats.splitlines():
				if ":" in line:
					output.update(("epmem {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# epmem stats
			extract = ("Time", "Memory Usage", "Memory Highwater", "Retrievals", "Queries", "Nexts", "Prevs",)
			for line in estats.splitlines():
				if ":" in line:
					output.update(("estats {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# epmem timer
			extract = ("_total", "epmem_ncb_retrieval", "epmem_next", "epmem_prev", "epmem_query", "epmem_storage",)
			for line in etimer.splitlines():
				if ":" in line:
					output.update(("etimer {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# wma
			extract = ("decay-rate", "decay-thresh", "forgetting", "forget-wme",)
			for line in wma.splitlines():
				if ":" in line:
					output.update(("wma {}".format(k), v) for k, v in [line.split(":", 1),] if k in extract)

			# stats
			# kernel CPU
			output["stats kernel cpu sec"] = re.sub(".*Kernel CPU Time: *([0-9.]*).*", r"\1", stats, flags=re.DOTALL)
			# total CPU
			output["stats total cpu sec"] = re.sub(".*Total  CPU Time: *([0-9.]*).*", r"\1", stats, flags=re.DOTALL)
			# decisions
			output["stats decision"] = re.sub(".*?([0-9]+) decisions.*", r"\1", stats, flags=re.DOTALL)
			# msec/decisions
			output["stats decision msec"] = re.sub(".*?([0-9.]+) msec/decision.*", r"\1", stats, flags=re.DOTALL)
			# elaboration cycles
			output["stats elab cycles"] = re.sub(".*?([0-9]+) elaboration cycles.*", r"\1", stats, flags=re.DOTALL)
			# avg wm size 
			output["stats avg wm size"] = re.sub(".*?([0-9.]+) mean.*", r"\1", stats, flags=re.DOTALL)
			# max wm size 
			output["stats max wm size"] = re.sub(".*?([0-9.]+) maximum.*", r"\1", stats, flags=re.DOTALL)

			# statsmax
			# max decision time
			output["max decision sec"] = re.sub(".*  Time \(sec\) *([0-9.]*).*", r"\1", statsmax, flags=re.DOTALL)
			# max epmem time
			output["max epmem sec"] = re.sub(".*EpMem Time \(sec\) *([0-9.]*).*", r"\1", statsmax, flags=re.DOTALL)
			# max smem time
			output["max smem sec"] = re.sub(".*SMem Time \(sec\) *([0-9.]*).*", r"\1", statsmax, flags=re.DOTALL)
			# max wm changes
			output["max wm changes"] = re.sub(".*WM changes *([0-9.]*).*", r"\1", statsmax, flags=re.DOTALL)

			# firing counts
			for line in firing.splitlines()[2:-1]:
				count, rule = line.strip().split(":")
				output["firing {}".format(rule.strip().replace("-", "").replace("*", " "))] = count.strip()

			print(" ".join(sorted("{}={}".format(re.sub("_+", "_", re.sub("[ -]", "_", k.lower())).strip("_"), v.strip()) for k, v in sorted(output.items()))))

