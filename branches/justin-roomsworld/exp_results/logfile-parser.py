#!/usr/bin/env python

import re
import sys

if __name__ == "__main__":
	with open(sys.argv[1]) as fd:
		echo, commands = fd.read().partition("END OUTPUT")[0].partition("BEGIN COMMANDS")[0:3:2]
		"""
		for line in echo.splitlines():
			if re.match("[^>]*:", line):
				print(line)
				"""
		sstats, stimer, estats, etimer, stats = commands.split("\n\n", 5)[1:]

		# smem stats
		extract = ("Memory Usage", "Memory Highwater", "Retrieves", "Queries", "Stores", "Activation Updates", "Mirrors",)
		for line in sstats.splitlines():
			if line.split(":")[0].strip() in extract:
				print("smem {}".format(line))

		# smem timer
		for line in stimer.splitlines():
			if not re.match("[a-z]*>", line):
				print("smem_timer {}".format(line))

		# epmem stats
		extract = ("Time", "Memory Usage", "Memory Highwater", "Retrievals", "Queries", "Nexts", "Prevs",)
		for line in estats.splitlines():
			if line.split(":")[0].strip() in extract:
				print("epmem {}".format(line))

		# epmem timer
		extract = ("_total", "epmem_ncb_retrieval", "epmem_next", "epmem_prev", "epmem_query", "epmem_storage",)
		for line in etimer.splitlines():
			if line.split(":")[0].strip() in extract:
				print("epmem_timer {}".format(line))

		# stats
